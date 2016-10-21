/// @file SkyModelServiceImpl.cc
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
#include "SkyModelServiceImpl.h"

// Include package level header file
#include "askap_skymodel.h"

// System includes
#include <string>

// ASKAPsoft includes
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>

// ODB includes
#include <odb/mysql/database.hxx>
#include <odb/sqlite/database.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/connection.hxx>
#include <odb/transaction.hxx>

// Local package includes
#include "datamodel/ContinuumComponent-odb.h"

ASKAP_LOGGER(logger, ".SkyModelService");

using namespace odb;
using namespace askap::cp::sms;
using namespace askap::interfaces::skymodelservice;


SkyModelServiceImpl* SkyModelServiceImpl::create(const LOFAR::ParameterSet& parset)
{
    SkyModelServiceImpl* pImpl = 0;
    const string dbType = parset.get("database.backend");
    //const string tablespace = parset.get("database.tablespace");
    ASKAPLOG_DEBUG_STR(logger, "database backend: " << dbType);
    //ASKAPLOG_DEBUG_STR(logger, "database tablespace: " << tablespace);

    if (dbType.compare("sqlite") == 0) {
        // get parameters
        const LOFAR::ParameterSet& dbParset = parset.makeSubset("sqlite.");
        const string dbName = dbParset.get("name");

        ASKAPLOG_INFO_STR(logger, "Instantiating sqlite backend into " << dbName);

        // create the database
        boost::shared_ptr<odb::database> pDb(
            new sqlite::database(
                dbName,
                SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));

        // create the implementation
        pImpl = new SkyModelServiceImpl(pDb);
        //pImpl = new SkyModelServiceImpl(pDb, tablespace);
        ASKAPCHECK(pImpl, "SkyModelServiceImpl construction failed");
    }
    else if (dbType.compare("mysql") == 0) {
        // TODO Implement support for MySQL
        ASKAPTHROW(AskapError, "MySQL support not implemented yet");
    }
    else {
        ASKAPTHROW(AskapError, "Unsupported database backend: " << dbType);
    }

    ASKAPCHECK(pImpl, "SkyModelServiceImpl creation failed");
    return pImpl;
}

SkyModelServiceImpl::SkyModelServiceImpl(
    boost::shared_ptr<odb::database> database)
    //const std::string& tablespace)
    :
    itsDb(database)
    //itsTablespace(tablespace)
{
}

SkyModelServiceImpl::~SkyModelServiceImpl()
{
    ASKAPLOG_DEBUG_STR(logger, "dtor");
    // shutdown the database
}

bool SkyModelServiceImpl::createSchema(bool dropTables)
{
    if (itsDb->id () == odb::id_sqlite) {
        ASKAPLOG_DEBUG_STR(logger, "Creating sqlite db");
        createSchemaSqlite(dropTables);
        return true;
    }

    return false;
}

void SkyModelServiceImpl::createSchemaSqlite(bool dropTables)
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

std::string SkyModelServiceImpl::getServiceVersion(const Ice::Current&)
{
    ASKAPLOG_DEBUG_STR(logger, "getServiceVersion");
    // TODO: should this return ASKAP_PACKAGE_VERSION? Or a semantic version?
    return "1.0";
}

ComponentIdSeq SkyModelServiceImpl::coneSearch(
    double rightAscension,
    double declination,
    double searchRadius,
    double fluxLimit,
    const Ice::Current&)
{
    ASKAPLOG_DEBUG_STR(logger, "getServiceVersion");
    return ComponentIdSeq();
}

ComponentSeq SkyModelServiceImpl::getComponents(
    const ComponentIdSeq& componentIds,
    const Ice::Current&)
{
    ASKAPLOG_DEBUG_STR(logger, "getComponents");
    return ComponentSeq();
}

ComponentIdSeq SkyModelServiceImpl::addComponents(
    const ComponentSeq& components,
    const Ice::Current&)
{
    ASKAPLOG_DEBUG_STR(logger, "addComponents");
    return ComponentIdSeq();
}

void SkyModelServiceImpl::removeComponents(
    const ComponentIdSeq& componentIds,
    const Ice::Current&)
{
    ASKAPLOG_DEBUG_STR(logger, "removeComponents");
}
