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

import IceStorm
from askap.iceutils import CPFuncTestBase, get_service_object
from askap.slice import CP, SchedulingBlockService
from askap.interfaces.cp import ICPObsServicePrx
from askap.interfaces.schedblock import ISBStateMonitorPrx, ObsState


# @skip
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

        fbs = self.feedback_service
        fbs.clear_history()

        sbid = 2056
        timestamp = datetime.now()
        self.publisher_proxy.changed(
            sbid,
            ObsState.PROCESSING,
            str(timestamp))

        # We need to allow some time for the round-trip message propagation.
        expected_history_length = 1
        for retries in range(5):
            if len(fbs.history) < expected_history_length:
                sleep(1)
            else:
                break

        # Exactly 1 notification should have been sent
        assert len(fbs.history) == expected_history_length
        name, args = fbs.history[0]
        assert name == 'sbStateChangedNotification'
        assert args['sbid'] == sbid
        assert args['obsState'] == ObsState.PROCESSING
