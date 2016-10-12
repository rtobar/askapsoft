""" Functional test stub for the Sky Model Service
"""

import os
import sys
from datetime import datetime
from time import sleep

from unittest import skip

import IceStorm
from askap.iceutils import CPFuncTestBase, get_service_object
from askap.slice import CP, SchedulingBlockService
# from askap.interfaces.cp import ICPObsServicePrx


@skip
class Test(CPFuncTestBase):
    def __init__(self):
        super(Test, self).__init__()
        self.sms_client = None

    def setUp(self):
        # Note that the working directory is 'functests', thus paths are
        # relative to that location.
        os.environ["ICE_CONFIG"] = "config-files/ice.cfg"
        os.environ['TEST_DIR'] = 'test_stub'
        super(Test, self).setUp()

        try:
            self.sms_client = get_service_object(
                self.ice_session.communicator,
                "CentralProcessorService@CentralProcessorAdapter",
                ICPObsServicePrx)
        except Exception as ex:
            self.shutdown()
            raise

    def test_get_service_version(self):
        # fbs = self.feedback_service
        # fbs.clear_history()
        assert self.sms_client.getServiceVersion()
