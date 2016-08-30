"""Test CP Manager service response to SB State Changed events

    Available events are:
    DRAFT, SUBMITTED, SCHEDULED, EXECUTING, PROCESSING, PENDINGARCHIVE,
    COMPLETED, ERRORED, RETIRED

    According to the current design, cp manager should only respond to the
    PROCESSING event.
"""

import os
from datetime import datetime
from time import sleep

from unittest import skip

import Ice, IceStorm
from askap.iceutils import IceSession, get_service_object
from askap.slice import CP, SchedulingBlockService
from askap.interfaces.cp import ICPObsServicePrx
from askap.interfaces.schedblock import ISBStateMonitorPrx, ObsState


class TestSBStateChanged(object):
    def __init__(self):
        self.igsession = None
        self.service = None
        self.topic_manager = None
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

    def test_manager_creates_topic(self):
        self.setup_icestorm(topic=None)
        # wait a little while to give CP Manager time to start up and
        # create the topic
        sleep(5)
        try:
            self.topic_manager.retrieve('sbstatechange')
        except Exception as e:
            assert False, 'Failed with {0}'.format(e)

    def test_sbstate_processing(self):
        self.setup_icestorm()

        assert self.igsession
        assert self.service
        assert self.topic_manager
        assert self.publisher_proxy

        sbid = 1
        timestamp = datetime.now()
        self.publisher_proxy.changed(
            sbid,
            ObsState.PROCESSING,
            str(timestamp))

        # TODO: determine whether the manager process responded
        assert False, 'placeholder until I implement the reverse connection'
