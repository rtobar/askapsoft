/// @file GlobalSkyModelTest.h
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
#include <Common/ParameterSet.h>
#include <votable/VOTable.h>

// Classes to test
#include "datamodel/ContinuumComponent.h"
#include "service/GlobalSkyModel.h"

using namespace std;
using namespace askap::accessors;

namespace askap {
namespace cp {
namespace sms {

class GlobalSkyModelTest : public CppUnit::TestFixture {

        CPPUNIT_TEST_SUITE(GlobalSkyModelTest);
        CPPUNIT_TEST(testParsetAssumptions);
        CPPUNIT_TEST(testCreateFromParsetFile);
        CPPUNIT_TEST(testNside);
        CPPUNIT_TEST(testHealpixOrder);
        CPPUNIT_TEST(testIngestVOTableToEmptyDatabase);
        CPPUNIT_TEST(testIngestVOTableFailsForBadCatalog);
        CPPUNIT_TEST_SUITE_END();

    public:
        GlobalSkyModelTest() :
            gsm(),
            parset(true),
            parsetFile("./tests/data/sms_parset.cfg"),
            small_components("./tests/data/votable_small_components.xml"),
            large_components("./tests/data/votable_large_components.xml"),
            invalid_components("./tests/data/votable_error_freq_units.xml")
        {
        }

        void setUp() {
            parset.clear();
            parset.adoptFile(parsetFile);
            gsm.reset(GlobalSkyModel::create(parset));
            gsm->createSchema();
        }

        void tearDown() {
            parset.clear();
        }

        void testParsetAssumptions() {
            CPPUNIT_ASSERT_EQUAL(
                string("sqlite"),
                parset.get("database.backend").get());
            CPPUNIT_ASSERT_EQUAL(
                string("./tests/service/gsm_unit_tests.sqlite"),
                parset.get("sqlite.name").get());
        }

        void testCreateFromParsetFile() {
            CPPUNIT_ASSERT(gsm.get());
        }

        void testNside() {
            CPPUNIT_ASSERT_EQUAL(2l << 14, gsm->getHealpixNside());
        }

        void testHealpixOrder() {
            CPPUNIT_ASSERT_EQUAL(14l, gsm->getHealpixOrder());
        }

        void testIngestVOTableToEmptyDatabase() {
            CPPUNIT_ASSERT(gsm->ingestVOTable(small_components, ""));

            boost::shared_ptr<datamodel::ContinuumComponent> component(
                gsm->getComponentByID(1));
            CPPUNIT_ASSERT(component.get());
        }

        void testIngestVOTableFailsForBadCatalog() {
            CPPUNIT_ASSERT(!gsm->ingestVOTable(invalid_components, ""));
        }
    private:
        boost::shared_ptr<GlobalSkyModel> gsm;
        LOFAR::ParameterSet parset;
        const string parsetFile;
        const string small_components;
        const string large_components;
        const string invalid_components;
};

}
}
}
