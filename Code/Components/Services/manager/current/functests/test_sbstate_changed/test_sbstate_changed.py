"""Test CP Manager service response to SB State Changed events

    Available events are:
    DRAFT, SUBMITTED, SCHEDULED, EXECUTING, PROCESSING, PENDINGARCHIVE,
    COMPLETED, ERRORED, RETIRED

    According to the current design, cp manager should only respond to the
    PROCESSING event.
"""

import os
import sys
import threading
from datetime import datetime
from time import sleep

from unittest import skip

import Ice, IceStorm
from askap.iceutils import IceSession, get_service_object, Server
from askap.slice import CP, SchedulingBlockService
from askap.interfaces.cp import ICPObsServicePrx, ICPFuncTestReporter
from askap.interfaces.schedblock import ISBStateMonitorPrx, ObsState


class FeedbackService(ICPFuncTestReporter):
    """ Service implementation of the askap.interfaces.cp.ICPFuncTestReporter
    Ice interface.

    Used for feedback from processes under test to the test driver.
    """
    def __init__(self):
        self.history = []
        """The method call history. Each entry is a (str, dict) tuple. The str
        gives the method name, and the dict gives the method args as name:value
        pairs.
        """

    def clear_history(self):
        """ Clears the history list. """
        self.history = []

    def sbStateChangedNotification(self, sbid, obsState, current=None):
        # print >> sys.stderr, '\nSB {0} state changed: {1}'.format(sbid, obsState)
        self.history.append(
            ('sbStateChangedNotification', {
                'sbid': sbid,
                'obsState': obsState,
            }))


class FuncTestServer(Server):
    """ Ice Server for hosting the FeedbackService. """
    def __init__(self, feedback_service, comm):
        self.feedback_service = feedback_service
        super(FuncTestServer, self).__init__(
            comm,
            configurable=False,
            retries=10,
            monitoring=False)

    def initialize_services(self):
        self.add_service('FuncTestReporter', self.feedback_service)

    def wait_async(self):
        def worker():
            # print >> sys.stderr, 'FuncTestServer thread waiting'
            self.wait()

        t = threading.Thread(target=worker)
        t.start()
        return t


class CPFuncTestBase(object):
    def __init__(self):
        self.ice_session = None
        self.server = None
        self.server_thread = None
        self.feedback_service = None

    def setUp(self):
        self.ice_session = IceSession(
            os.path.expandvars("$TEST_DIR/applications.txt"),
            cleanup=True)
        try:
            self.ice_session.start()
            self.setup_feedback_service()
        except Exception as ex:
            self.ice_session.communicator.destroy()
            self.ice_session.terminate()
            raise

    def tearDown(self):
        self.shutdown()

    def shutdown(self):
        self.ice_session.communicator.destroy()
        self.ice_session.terminate()
        self.ice_session = None

    def setup_feedback_service(self):
        self.feedback_service = FeedbackService()
        self.server = FuncTestServer(
            self.feedback_service,
            self.ice_session.communicator)
        self.server.setup_services()
        # self.server_thread = self.server.wait_async()


class TestSBStateChanged(CPFuncTestBase):
    def __init__(self):
        super(TestSBStateChanged, self).__init__()
        self.cpclient = None
        self.topic_manager = None
        self.publisher_proxy = None

    def setUp(self):
        # Note that the working directory is 'functests', thus paths are
        # relative to that location.
        os.environ["ICE_CONFIG"] = "config-files/ice.cfg"
        os.environ['TEST_DIR'] = 'test_sbstate_changed'

        super(TestSBStateChanged, self).setUp()

        try:
            self.cpclient = get_service_object(
                self.ice_session.communicator,
                "CentralProcessorService@CentralProcessorAdapter",
                ICPObsServicePrx)
        except Exception as ex:
            self.shutdown()
            raise

    def setup_icestorm(self, topic="sbstatechange"):
        """Create the IceStorm connection
        Modelled on the TOS TypedValuePublisher Python class
        """
        self.topic_manager = get_service_object(
            self.ice_session.communicator,
            'IceStorm/TopicManager@IceStorm.TopicManager',
            IceStorm.TopicManagerPrx)

        if not topic:
            return

        self.topic = topic
        try:
            topic = self.topic_manager.retrieve(topic)
        except IceStorm.NoSuchTopic:
            try:
                topic = self.topic_manager.create(topic)
            except IceStorm.TopicExists:
                return
        publisher = topic.getPublisher().ice_oneway()
        self.publisher_proxy = ISBStateMonitorPrx.uncheckedCast(publisher)

    # @skip('too slow!')
    def test_manager_creates_topic(self):
        self.setup_icestorm(topic=None)
        # wait a little while to give CP Manager time to start up and
        # create the topic
        sleep(5)
        try:
            self.topic_manager.retrieve('sbstatechange')
        except Exception as e:
            assert False, 'Failed with {0}'.format(e)

    # @skip('too slow!')
    def test_sbstate_processing(self):
        self.setup_icestorm()

        fbs = self.feedback_service
        fbs.clear_history()

        sbid = 1
        timestamp = datetime.now()
        self.publisher_proxy.changed(
            sbid,
            ObsState.PROCESSING,
            str(timestamp))

        # We need to allow some time for the round-trip message propagation.
        # it would be better to poll here.
        sleep(4)

        # Exactly 1 notification should have been sent
        assert len(fbs.history) == 1
        name, args = fbs.history[0]
        assert name == 'sbStateChangedNotification'
        assert args['sbid'] == sbid
        assert args['obsState'] == ObsState.PROCESSING
