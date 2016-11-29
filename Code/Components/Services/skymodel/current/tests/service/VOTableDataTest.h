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
#include "service/VOTableData.h"
#include "service/VOTableComponent.h"

using namespace std;
using namespace askap::accessors;

namespace askap {
namespace cp {
namespace sms {

class VOTableDataTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(VOTableDataTest);
        //CPPUNIT_TEST(testSmallComponentTableFirstComponentValues);
        CPPUNIT_TEST(testSmallComponentTableLoadCount);
        CPPUNIT_TEST(testLargeComponentTableLoadCount);
        CPPUNIT_TEST(testSmallComponentTableAssumptions);
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

        void testSmallComponentTableFirstComponentValues() {
            boost::shared_ptr<VOTableData> pData(VOTableData::create(small_components, ""));
            const datamodel::ContinuumComponent& c = pData->getComponents()[0];
            CPPUNIT_ASSERT_DOUBLES_EQUAL(79.176918, c.ra_deg_cont, 0.000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-71.819671, c.dec_deg_cont, 0.000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.01f, c.ra_err, 0.000001f);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.01f, c.dec_err, 0.000001f);
        }

        void testSmallComponentTableLoadCount() {
            boost::shared_ptr<VOTableData> pData(VOTableData::create(small_components, ""));
            CPPUNIT_ASSERT_EQUAL(10l, pData->getCount());
        }

        void testLargeComponentTableLoadCount() {
            boost::shared_ptr<VOTableData> pData(VOTableData::create(large_components, ""));
            CPPUNIT_ASSERT_EQUAL(134l, pData->getCount());
        }

        void testSmallComponentTableAssumptions() {
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
