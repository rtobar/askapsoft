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

import nose.tools as nt
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
    SearchCriteria,
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
            radius=1.0,
            criteria=SearchCriteria())

        assert len(components) == 1
        c = components[0]
        assert c.componentId == "SB1958_image.i.LMC.cont.sb1958.taylor.0.restored_1a"
        nt.assert_almost_equal(c.ra, 70.176918)
        nt.assert_almost_equal(c.dec, -61.819671)
        nt.assert_almost_equal(c.raErr, 0.00999999977)
        nt.assert_almost_equal(c.decErr, 0.00999999977)
        nt.assert_almost_equal(c.freq, 1400.5)
        nt.assert_almost_equal(c.fluxPeak, 326.529998779)
        nt.assert_almost_equal(c.fluxPeakErr, 0.282999992371)
        nt.assert_almost_equal(c.fluxInt, 378.830993652)
        nt.assert_almost_equal(c.fluxIntErr, 0.541999995708)
        nt.assert_almost_equal(c.spectralIndex, -1.24000000954)
        nt.assert_almost_equal(c.spectralCurvature, -1.37999999523)

        # this database has polarisation data
        assert len(c.polarisation) == 1
        p = c.polarisation[0]
        nt.assert_almost_equal(p.lambdaRefSq, 0.09)
        nt.assert_almost_equal(p.polPeakFit, 1.55)
        nt.assert_almost_equal(p.polPeakFitDebias, 1.42)
        nt.assert_almost_equal(p.polPeakFitErr, 0.01)
        nt.assert_almost_equal(p.polPeakFitSnr, 150.0)
        nt.assert_almost_equal(p.polPeakFitSnrErr, 1.0)
        nt.assert_almost_equal(p.fdPeakFit, 52.0)
        nt.assert_almost_equal(p.fdPeakFitErr, 5.2)

    def test_simple_rect_search(self):
        roi = Rect(centre=Coordinate(79.375, -71.5), extents=RectExtents(0.75, 1.0))
        results = self.sms_client.rectSearch(roi, SearchCriteria())

        assert len(results) == 4

        actual_ids = [c.componentId for c in results]
        expected_ids = [
            "SB1958_image.i.LMC.cont.sb1958.taylor.0.restored_1b",
            "SB1958_image.i.LMC.cont.sb1958.taylor.0.restored_1c",
            "SB1958_image.i.LMC.cont.sb1958.taylor.0.restored_4a",
            "SB1958_image.i.LMC.cont.sb1958.taylor.0.restored_4c"]

        for expected in expected_ids:
            assert expected in actual_ids
