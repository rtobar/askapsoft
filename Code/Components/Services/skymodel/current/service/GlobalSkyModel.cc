/// @file GlobalSkyModel.cc
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

// Include own header file first
#include "GlobalSkyModel.h"

// Include package level header file
#include "askap_skymodel.h"

// System includes
#include <string>

// ASKAPsoft includes
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>
#include <votable/VOTable.h>

// ODB includes
#include <odb/mysql/database.hxx>
#include <odb/sqlite/database.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/connection.hxx>
#include <odb/transaction.hxx>

// Local package includes
#include "datamodel/ContinuumComponent-odb.h"
#include "Utility.h"

ASKAP_LOGGER(logger, ".GlobalSkyModel");

using namespace odb;
using namespace askap::cp::sms;
using namespace askap::accessors;


GlobalSkyModel* GlobalSkyModel::create(const LOFAR::ParameterSet& parset)
{
    GlobalSkyModel* pImpl = 0;
    const string dbType = parset.get("database.backend");
    ASKAPLOG_DEBUG_STR(logger, "database backend: " << dbType);

    if (dbType.compare("sqlite") == 0) {
        // get parameters
        const LOFAR::ParameterSet& dbParset = parset.makeSubset("sqlite.");
        const string dbName = dbParset.get("name");

        ASKAPLOG_INFO_STR(logger, "Instantiating sqlite backend into " << dbName);

        // TODO Parset flag for db creation control
        // create the database
        ::boost::shared_ptr<odb::database> pDb(
            new sqlite::database(
                dbName,
                SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
        ASKAPCHECK(pDb.get(), "GlobalSkyModel creation failed");

        // create the implementation
        pImpl = new GlobalSkyModel(pDb);
        ASKAPCHECK(pImpl, "GlobalSkyModel construction failed");
    }
    else if (dbType.compare("mysql") == 0) {
        // TODO Implement support for MySQL
        ASKAPTHROW(AskapError, "MySQL support not implemented yet");
    }
    else {
        ASKAPTHROW(AskapError, "Unsupported database backend: " << dbType);
    }

    ASKAPCHECK(pImpl, "GlobalSkyModel creation failed");
    return pImpl;
}

GlobalSkyModel::GlobalSkyModel(boost::shared_ptr<odb::database> database)
    :
    itsDb(database)
{
}

GlobalSkyModel::~GlobalSkyModel()
{
    ASKAPLOG_DEBUG_STR(logger, "dtor");
    // shutdown the database
}

bool GlobalSkyModel::createSchema(bool dropTables)
{
    // SQLite has quirks that must be handled with DB-specific code...
    if (itsDb->id () == odb::id_sqlite) {
        ASKAPLOG_DEBUG_STR(logger, "Creating sqlite db");
        createSchemaSqlite(dropTables);
        return true;
    }
    // TODO: other database backends. Generic code?

    return false;
}

void GlobalSkyModel::createSchemaSqlite(bool dropTables)
{
    // Create the database schema. Due to bugs in SQLite foreign key
    // support for DDL statements, we need to temporarily disable
    // foreign keys.
    connection_ptr c(itsDb->connection());

    c->execute ("PRAGMA foreign_keys=OFF");

    transaction t(c->begin());
    schema_catalog::create_schema(*itsDb, "", dropTables);
    t.commit();

    c->execute("PRAGMA foreign_keys=ON");
}

bool GlobalSkyModel::ingestVOTable(const std::string& filename) {

    return false;
}
