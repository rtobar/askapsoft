/// @file sms_tools.cc
/// @brief Entry point for Sky Model Service tools and utility functions.
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

// Include package level header file
#include <askap_skymodel.h>

// System includes
#include <string>
#include <fstream>
#include <sstream>

// Boost includes
#include "boost/date_time/posix_time/posix_time.hpp"

// ASKAPsoft includes
#include <askap/Application.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/StatReporter.h>
#include <Common/ParameterSet.h>

// ODB includes
#include <odb/exception.hxx>

// Local Package includes
#include "service/GlobalSkyModel.h"
#include "service/SkyModelService.h"
#include "service/Utility.h"


using namespace boost;
using namespace askap;
using namespace askap::cp::sms;

ASKAP_LOGGER(logger, ".sms_tools");

#define CREATE_SCHEMA "create-schema"
#define INGEST_COMPONENTS "ingest-components"
#define INGEST_POLARISATION "ingest-polarisation"
#define SB_ID "sbid"
#define OBS_DATE "observation-date"
#define RANDOMISE "gen-random-components"

class SmsToolsApp : public askap::Application {
    public:
        virtual int run(int argc, char* argv[])
        {
            StatReporter stats;

            try {
                // Dispatch to the requested utility function
                if (parameterExists(CREATE_SCHEMA)) {
                    return createSchema();
                }
                else if (parameterExists(INGEST_COMPONENTS)) {
                    return ingestVoTable();
                }
                else if (parameterExists(RANDOMISE)) {
                    int64_t count = lexical_cast<int64_t>(parameter(RANDOMISE));
                    return generateRandomComponents(count);
                }
            } catch (const AskapError& e) {
                ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << e.what());
                return 1;
            } catch (const odb::exception& e) {
                ASKAPLOG_FATAL_STR(logger, "Database exception in " << argv[0] << ": " << e.what());
                return 2;
            } catch (const std::exception& e) {
                ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << e.what());
                return 3;
            }

            stats.logSummary();

            return 0;
        }

    private:

        int createSchema() {
            const LOFAR::ParameterSet& parset = config();
            bool dropTables = parset.getBool("database.create_schema.droptables", true);

            boost::shared_ptr<GlobalSkyModel> pGsm(GlobalSkyModel::create(config()));
            return pGsm->createSchema(dropTables) ? 0 : 4;
        }

        int ingestVoTable() {
            ASKAPASSERT(parameterExists(INGEST_COMPONENTS));
            ASKAPASSERT(parameterExists(SB_ID));
            ASKAPASSERT(parameterExists(OBS_DATE));

            string components = parameter(INGEST_COMPONENTS);
            string polarisation = parameterExists(INGEST_POLARISATION) ? parameter(INGEST_POLARISATION) : "";
            int64_t sbid = lexical_cast<int64_t>(parameter(SB_ID));
            posix_time::ptime obsDate = date_time::parse_delimited_time<posix_time::ptime>(parameter(OBS_DATE), 'T');
            boost::shared_ptr<GlobalSkyModel> pGsm(GlobalSkyModel::create(config()));
            pGsm->ingestVOTable(
                components,
                polarisation,
                sbid,
                obsDate);
            return 0;
        }

        int generateRandomComponents(int64_t componentCount) {

            if (componentCount > 0) {

                const int64_t BLOCK_SIZE = 1000;
                int64_t numBlocks = componentCount / BLOCK_SIZE + 1;
                int64_t remainder = componentCount % BLOCK_SIZE;
                boost::shared_ptr<GlobalSkyModel> pGsm(GlobalSkyModel::create(config()));

                for (int64_t n = 0; n < numBlocks; n++) {
                    int64_t componentsInBlock = n == numBlocks - 1 ? remainder : BLOCK_SIZE;
                    ASKAPLOG_DEBUG_STR(
                        logger,
                        "Generating components " <<
                        n * BLOCK_SIZE <<
                        " to " <<
                        n * BLOCK_SIZE + componentsInBlock);

                    GlobalSkyModel::ComponentList components(componentsInBlock);
                    generateRandomComponents(components);
                    pGsm->uploadComponents(components.begin(), components.end());
                }
            }

            return 0;
        }

        void generateRandomComponents(GlobalSkyModel::ComponentList& components) {
            for (GlobalSkyModel::ComponentList::iterator it = components.begin(); 
                 it != components.end();
                 it++) {

                it->
            }
        }
};

int main(int argc, char *argv[])
{
    SmsToolsApp app;
    app.addParameter(CREATE_SCHEMA, "s", "Initialises an empty database", false);
    app.addParameter(INGEST_COMPONENTS, "c", "Ingest/upload a VO Table of components to the global sky model", true);
    app.addParameter(INGEST_POLARISATION, "p", "Optional polarisation data catalog", true);
    app.addParameter(SB_ID, "i", "Scheduling block ID for ingested catalog", true);
    app.addParameter(OBS_DATE, "d", "Observation date for ingested catalog, in form YYYY-MM-DDTHH:MM:SS", true);
    app.addParameter(RANDOMISE, "t", "Populate the database by randomly generating the specified number of components", "0");
    return app.main(argc, argv);
}
