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
#include <askap/AskapError.h>
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <Common/ParameterSet.h>
#include <votable/VOTable.h>

// Classes to test
#include "datamodel/ContinuumComponent.h"
#include "service/GlobalSkyModel.h"

using namespace std;
using namespace askap::accessors;
using namespace boost;
using namespace boost::posix_time;

namespace askap {
namespace cp {
namespace sms {

class GlobalSkyModelTest : public CppUnit::TestFixture {

        CPPUNIT_TEST_SUITE(GlobalSkyModelTest);
        CPPUNIT_TEST(testParsetAssumptions);
        CPPUNIT_TEST(testCreateFromParsetFile);
        CPPUNIT_TEST(testNside);
        CPPUNIT_TEST(testHealpixOrder);
        CPPUNIT_TEST(testGetMissingComponentById);
        CPPUNIT_TEST(testIngestVOTableToEmptyDatabase);
        CPPUNIT_TEST(testIngestVOTableFailsForBadCatalog);
        CPPUNIT_TEST(testMetadata);
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
        }

        void tearDown() {
            parset.clear();
        }

        void initEmptyDatabase() {
            gsm.reset(GlobalSkyModel::create(parset));
            gsm->createSchema();
        }

        void testParsetAssumptions() {
            CPPUNIT_ASSERT_EQUAL(
                string("sqlite"),
                parset.get("database.backend").get());
            CPPUNIT_ASSERT_EQUAL(
                string("./tests/service/gsm_unit_tests.dbtmp"),
                parset.get("sqlite.name").get());
        }

        void testCreateFromParsetFile() {
            initEmptyDatabase();
            CPPUNIT_ASSERT(gsm.get());
        }

        void testNside() {
            initEmptyDatabase();
            CPPUNIT_ASSERT_EQUAL(2l << 14, gsm->getHealpixNside());
        }

        void testHealpixOrder() {
            initEmptyDatabase();
            CPPUNIT_ASSERT_EQUAL(14l, gsm->getHealpixOrder());
        }

        void testGetMissingComponentById() {
            initEmptyDatabase();
            shared_ptr<datamodel::ContinuumComponent> component(
                gsm->getComponentByID(9));
            CPPUNIT_ASSERT(!component.get());
        }

        void testIngestVOTableToEmptyDatabase() {
            parset.replace("sqlite.name", "./tests/service/ingested.dbtmp");
            initEmptyDatabase();
            // perform the ingest
            vector<datamodel::id_type> ids = gsm->ingestVOTable(small_components, "");
            CPPUNIT_ASSERT_EQUAL(size_t(10), ids.size());

            // test that some expected components can be found
            shared_ptr<datamodel::ContinuumComponent> component(
                gsm->getComponentByID(ids[0]));
            CPPUNIT_ASSERT(component.get());
            CPPUNIT_ASSERT_EQUAL(
                string("SB1958_image.i.LMC.cont.sb1958.taylor.0.restored_1a"),
                component->component_id);
        }

        void testMetadata() {
            parset.replace("sqlite.name", "./tests/service/metadata.dbtmp");
            initEmptyDatabase();

            int64_t expected_sb_id = 71414;
            ptime expected_obs_date = second_clock::universal_time();

            // perform the ingest
            vector<datamodel::id_type> ids = gsm->ingestVOTable(
                    small_components,
                    "",
                    expected_sb_id,
                    expected_obs_date);

            // Test that every component has the schedblock ID set
            for (vector<datamodel::id_type>::const_iterator it = ids.begin();
                it != ids.end();
                it++) {
                shared_ptr<datamodel::ContinuumComponent> component(gsm->getComponentByID(*it));

                CPPUNIT_ASSERT_EQUAL(expected_sb_id, component->sb_id);

                // CPPUNIT_ASSERT_EQUAL chokes on the ptime values even though
                // they define operator==.
                CPPUNIT_ASSERT(expected_obs_date == component->observation_date);
            }
        }

        void testIngestVOTableFailsForBadCatalog() {
            initEmptyDatabase();
            CPPUNIT_ASSERT_THROW(
                gsm->ingestVOTable(invalid_components, ""),
                askap::AskapError);
        }
    private:
        shared_ptr<GlobalSkyModel> gsm;
        LOFAR::ParameterSet parset;
        const string parsetFile;
        const string small_components;
        const string large_components;
        const string invalid_components;
};

}
}
}
