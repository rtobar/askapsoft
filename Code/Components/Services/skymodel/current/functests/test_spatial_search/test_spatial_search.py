""" Sky Model Service spatial search tests

These tests are configured to use the same SQLite database as some of the
C++ GlobalSkyModel class unit tests.

By replicating the same tests against the same database, it helps ensure
consistency between the backend C++ implementation and the Ice interface to the
SMS.
"""

import os
import sys
from datetime import datetime
from time import sleep

from unittest import skip

from askap.iceutils import CPFuncTestBase, get_service_object
from askap.slice import SkyModelService
from askap.interfaces.skymodelservice import (
    ISkyModelServicePrx,
    Coordinate,
    Rect,
    RectExtents,
    ContinuumComponent,
    ContinuumComponentPolarisation,
)


# @skip
class Test(CPFuncTestBase):
    def __init__(self):
        super(Test, self).__init__()
        self.sms_client = None

    def setUp(self):
        # Note that the working directory is 'functests', thus paths are
        # relative to that location.
        os.environ["ICE_CONFIG"] = "config-files/ice.cfg"
        os.environ['TEST_DIR'] = 'test_spatial_search'
        super(Test, self).setUp()

        try:
            self.sms_client = get_service_object(
                self.ice_session.communicator,
                "SkyModelService@SkyModelServiceAdapter",
                ISkyModelServicePrx)
        except Exception as ex:
            self.shutdown()
            raise

    def test_simple_cone_search(self):
        components = self.sms_client.coneSearch(
            centre=Coordinate(70.2, -61.8),
            radius=1.0)
        assert len(components) == 1
        assert components[0].componentId == "SB1958_image.i.LMC.cont.sb1958.taylor.0.restored_1a"

    def test_simple_rect_search(self):
        roi = Rect(centre=Coordinate(79.375, -71.5), extents=RectExtents(0.75, 1.0))
        results = self.sms_client.rectSearch(roi)

        assert len(results) == 4

        actual_ids = [c.componentId for c in results]
        expected_ids = [
            "SB1958_image.i.LMC.cont.sb1958.taylor.0.restored_1b",
            "SB1958_image.i.LMC.cont.sb1958.taylor.0.restored_1c",
            "SB1958_image.i.LMC.cont.sb1958.taylor.0.restored_4a",
            "SB1958_image.i.LMC.cont.sb1958.taylor.0.restored_4c"]

        for expected in expected_ids:
            assert expected in actual_ids
