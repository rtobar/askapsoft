"""Test CP Manager service response to SB State Changed events

    Available events are:
    DRAFT, SUBMITTED, SCHEDULED, EXECUTING, PROCESSING, PENDINGARCHIVE,
    COMPLETED, ERRORED, RETIRED

    According to the current design, cp manager should only respond to the
    PROCESSING event.
"""

import os
import sys
from datetime import datetime
from time import sleep

from unittest import skip

import Ice, IceStorm
from askap.iceutils import IceSession, get_service_object, Server
from askap.slice import CP, SchedulingBlockService
from askap.interfaces.cp import ICPObsServicePrx, ICPFuncTestReporter
from askap.interfaces.schedblock import ISBStateMonitorPrx, ObsState


class TestReportingService(ICPFuncTestReporter):
    def __init__(self):
        self.notify_history = []

    def notifySBStateChanged(self, sbid, obsState):
        self.notify_history.append((sbid, obsState))


class FuncTestServer(Server):
    def __init__(self, test_reporting_service, comm, **kwargs):
        self.test_reporting_service = test_reporting_service
        super(FuncTestServer, self).__init__(comm, kwargs)

    def initialize_services(self):
        self.add_service('FuncTestReporter', self.test_reporting_service)

    def get_config(self):
        # I don't need the parameters functionality
        pass


class TestSBStateChanged(object):
    def __init__(self):
        self.igsession = None
        self.cpclient = None
        self.topic_manager = None
        self.publisher_proxy = None
        self.server = None
        self.test_reporting_service = None

    def setUp(self):
        # Note that the working directory is 'functests', thus paths are
        # relative to that location.
        os.environ["ICE_CONFIG"] = "config-files/ice.cfg"
        os.environ['TEST_DIR'] = 'test_sbstate_changed'
        self.igsession = IceSession(
            os.path.expandvars("$TEST_DIR/applications.txt"),
            cleanup=True)

        print >>sys.stderr, sys.argv

        try:
            self.igsession.start()

            # Create a server to host the functional test service
            self.test_reporting_service = TestReportingService()
            self.server = FuncTestServer(
                self.test_reporting_service,
                self.igsession.communicator)

            # Get the CP Manager service object
            self.cpclient = get_service_object(
                self.igsession.communicator,
                "CentralProcessorService@CentralProcessorAdapter",
                ICPObsServicePrx)

        except Exception as ex:
            self.igsession.terminate()
            raise

    def setup_icestorm(self, topic="sbstatechange"):
        """Create the IceStorm connection
        Modelled on the TOS TypedValuePublisher Python class
        """

        self.topic_manager = get_service_object(
            self.igsession.communicator,
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
        self.igsession.terminate()
        self.igsession = None

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

        # Exactly 1 notification should have been sent
        assert len(self.test_reporting_service.notify_history) == 1
        actual_sbid, actual_state = self.test_reporting_service.notify_history[0]
        assert actual_sbid == sbid
        assert actual_state == ObsState.PROCESSING
