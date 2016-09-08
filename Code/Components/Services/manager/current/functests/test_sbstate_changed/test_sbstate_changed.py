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


class TestReportingService(ICPFuncTestReporter):
    """ Service implementation of the askap.interfaces.cp.ICPFuncTestReporter
    Ice interface.

    Used for feedback from processes under test to the test driver.
    """
    def __init__(self):
        self.notify_history = []

    def sbStateChangedNotification(self, sbid, obsState, current=None):
        # print >> sys.stderr, '\nSB {0} state changed: {1}'.format(sbid, obsState)
        self.notify_history.append((sbid, obsState))


class FuncTestServer(Server):
    """ Ice Server for hosting the TestReportingService. """
    def __init__(self, test_reporting_service, comm):
        self.test_reporting_service = test_reporting_service
        super(FuncTestServer, self).__init__(
            comm,
            configurable=False,
            retries=10,
            monitoring=False)

    def initialize_services(self):
        self.add_service('FuncTestReporter', self.test_reporting_service)

    def wait_async(self):
        def worker():
            # print >> sys.stderr, 'FuncTestServer thread waiting'
            self.wait()

        t = threading.Thread(target=worker)
        t.start()
        return t


class TestSBStateChanged(object):
    def __init__(self):
        self.ice_session = None
        self.cpclient = None
        self.topic_manager = None
        self.publisher_proxy = None
        self.server = None
        self.server_thread = None
        self.test_reporting_service = None

    def setUp(self):
        # Note that the working directory is 'functests', thus paths are
        # relative to that location.
        os.environ["ICE_CONFIG"] = "config-files/ice.cfg"
        os.environ['TEST_DIR'] = 'test_sbstate_changed'
        self.ice_session = IceSession(
            os.path.expandvars("$TEST_DIR/applications.txt"),
            cleanup=True)

        try:
            self.ice_session.start()

            # Create a server to host the functional test reporting service
            self.test_reporting_service = TestReportingService()
            self.server = FuncTestServer(
                self.test_reporting_service,
                self.ice_session.communicator)
            self.server.setup_services()
            # self.server_thread = self.server.wait_async()

            # Get the CP Manager service object
            self.cpclient = get_service_object(
                self.ice_session.communicator,
                "CentralProcessorService@CentralProcessorAdapter",
                ICPObsServicePrx)

        except Exception as ex:
            self.ice_session.communicator.destroy()
            self.ice_session.terminate()
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

    def tearDown(self):
        self.ice_session.communicator.destroy()
        self.ice_session.terminate()
        self.ice_session = None

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

        assert self.publisher_proxy
        assert self.test_reporting_service

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
        assert len(self.test_reporting_service.notify_history) == 1
        actual_sbid, actual_state = self.test_reporting_service.notify_history[0]
        assert actual_sbid == sbid
        assert actual_state == ObsState.PROCESSING
