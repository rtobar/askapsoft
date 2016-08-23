"""Test CP Manager service response to SB State Changed events

    Available events are:
    DRAFT, SUBMITTED, SCHEDULED, EXECUTING, PROCESSING, PENDINGARCHIVE,
    COMPLETED, ERRORED, RETIRED

    According to the current design, cp manager should only respond to the
    PROCESSING event.
"""

import os
from unittest import skip

import Ice, IceStorm
from askap.iceutils import IceSession, get_service_object

# always import from askap.slice before trying to import any interfaces
from askap.slice import CP, SchedulingBlockService
from askap.interfaces.cp import ICPObsServicePrx
from askap.interfaces.schedblock import ISBStateMonitorPrx, ObsState


class TestSBStateChanged(object):
    def __init__(self):
        self.igsession = None
        self.service = None
        self.topic_manager = None
        self.topic = "sbstatechange"
        self.publisher_proxy = None

    def setUp(self):
        # Note that the working directory is 'functests', thus paths are
        # relative to that location.
        os.environ["ICE_CONFIG"] = "config-files/ice.cfg"
        os.environ['TEST_DIR'] = 'test_sbstate_changed'
        self.igsession = IceSession(
            os.path.expandvars("$TEST_DIR/applications.txt"),
            cleanup=True)

        # Get the CP Manager service object
        try:
            self.igsession.start()
            self.service = get_service_object(
                self.igsession.communicator,
                "CentralProcessorService@CentralProcessorAdapter",
                ICPObsServicePrx)
        except Exception, ex:
            self.igsession.terminate()
            raise

        self.setup_icestorm()

    def setup_icestorm(self):
        """Create the IceStorm connection
        Modelled on the TOS TypedValuePublisher Python class
        """

        self.topic_manager = get_service_object(
            self.igsession.communicator,
            'IceStorm/TopicManager@IceStorm.TopicManager',
            IceStorm.TopicManagerPrx)

        try:
            topic = self.topic_manager.retrieve(self.topic)
        except IceStorm.NoSuchTopic:
            try:
                topic = self.topic_manager.create(self.topic)
            except IceStorm.TopicExists:
                return

        publisher = topic.getPublisher().ice_oneway()
        self.publisher_proxy = ISBStateMonitorPrx.uncheckedCast(publisher)

    def tearDown(self):
        self.igsession.terminate()
        self.igsession = None

    def test_sbstate_processing(self):
        assert self.igsession
        assert self.service
        assert self.manager
        assert self.publisher_proxy
        self.publisher_proxy.publish(ObsState.PROCESSING)
        # TODO: determine whether the manager process responded
