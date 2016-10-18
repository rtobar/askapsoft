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

// ASKAPsoft includes
#include <askap/AskapLogging.h>

// Local package includes
// Just for testing, see if the ODB generated datamodel code compiles...
#include "datamodel/ContinuumComponent-odb.h"

ASKAP_LOGGER(logger, ".SkyModelService");

using namespace askap::cp::sms;
using namespace askap::interfaces::skymodelservice;

/// @brief Constructor.
SkyModelServiceImpl::SkyModelServiceImpl()
{
    // need the parset
    
    // call a database factory method to create the right database based on the
    // parset

    // check for whether the schema already exists.
    // Depending on parset flag, either emit an error for missing schema or 
    // in test mode create a fresh one.
}

/// @brief Destructor.
SkyModelServiceImpl::~SkyModelServiceImpl()
{
    // shutdown the database
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
