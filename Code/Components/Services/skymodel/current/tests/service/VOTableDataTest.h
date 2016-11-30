/// @file VOTableDataTest.h
///
/// @copyright (c) 2016 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Daniel Collins <daniel.collins@csiro.au>

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// Support classes
#include <string>
#include <boost/filesystem.hpp>
#include <votable/VOTable.h>

// Classes to test
#include "service/HealPixTools.h"
#include "service/VOTableData.h"
#include "service/VOTableComponent.h"

using namespace std;
using namespace askap::accessors;

namespace askap {
namespace cp {
namespace sms {

class VOTableDataTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(VOTableDataTest);
        CPPUNIT_TEST(testFirstComponentValues);
        CPPUNIT_TEST(testHealpixIndexation);
        CPPUNIT_TEST(testLoadCount);
        CPPUNIT_TEST(testLargeLoadCount);
        CPPUNIT_TEST(testAssumptions);
        CPPUNIT_TEST_SUITE_END();

    public:
        VOTableDataTest() :
            small_components("./tests/data/votable_small_components.xml"),
            large_components("./tests/data/votable_large_components.xml")
        {
        }

        void setUp() {
        }

        void tearDown() {
        }

        void testFirstComponentValues() {
            boost::shared_ptr<VOTableData> pData(VOTableData::create(small_components, ""));
            const datamodel::ContinuumComponent& c = pData->getComponents()[0];
            CPPUNIT_ASSERT_DOUBLES_EQUAL(79.176918, c.ra_deg_cont, 0.000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-71.819671, c.dec_deg_cont, 0.000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.01f, c.ra_err, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.01f, c.dec_err, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1400.5f, c.freq, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(326.530f, c.flux_peak, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.283f, c.flux_peak_err, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(378.831f, c.flux_int, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.542f, c.flux_int_err, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(34.53f, c.maj_axis, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(30.62f, c.min_axis, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.03f, c.maj_axis_err, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.01f, c.min_axis_err, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(83.54f, c.pos_ang, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.29f, c.pos_ang_err, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(12.84f, c.maj_axis_deconv, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(10.85f, c.min_axis_deconv, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-15.32f, c.pos_ang_deconv, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(243.077f, c.chi_squared_fit, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1210.092f, c.rms_fit_Gauss, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.24f, c.spectral_index, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.38f, c.spectral_curvature, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.509f, c.rms_image, 0.000001f);
            CPPUNIT_ASSERT_EQUAL(string("SB1958_image.i.LMC.cont.sb1958.taylor.0.restored_1a"), c.component_id);
            CPPUNIT_ASSERT_EQUAL(true, c.has_siblings);
            CPPUNIT_ASSERT_EQUAL(false, c.fit_is_estimate);
        }

        void testHealpixIndexation() {
            const int order = 14;
            HealPixTools hp(order);
            boost::shared_ptr<VOTableData> pData(VOTableData::create(large_components, "", order));
            const VOTableData::ComponentList& components = pData->getComponents();

            for (VOTableData::ComponentList::const_iterator it = components.begin();
                 it != components.end();
                 it++) {
                boost::int64_t expected = hp.calcHealPixIndex(it->ra_deg_cont, it->dec_deg_cont);
                CPPUNIT_ASSERT_EQUAL(expected, it->healpix_index);
            }
        }

        void testLoadCount() {
            boost::shared_ptr<VOTableData> pData(VOTableData::create(small_components, ""));
            CPPUNIT_ASSERT_EQUAL(10l, pData->getCount());
        }

        void testLargeLoadCount() {
            boost::shared_ptr<VOTableData> pData(VOTableData::create(large_components, ""));
            CPPUNIT_ASSERT_EQUAL(134l, pData->getCount());
        }

        void testAssumptions() {
            // Not really a unit test of the VOTableData class, rather a
            // test of my assumptions regarding the test data that will impact
            // other tests.
            CPPUNIT_ASSERT(boost::filesystem::exists(small_components));

            VOTable vt = VOTable::fromXML(small_components);
            CPPUNIT_ASSERT_EQUAL(vt.getResource().size(), 1ul);

            const VOTableTable t = vt.getResource()[0].getTables()[0];
            CPPUNIT_ASSERT_EQUAL(vt.getResource()[0].getTables().size(), 1ul);
            CPPUNIT_ASSERT_EQUAL(t.getFields().size(), 33ul);
            CPPUNIT_ASSERT_EQUAL(t.getRows().size(), 10ul);
        }

    private:
        const string small_components;
        const string large_components;
};

}
}
}
