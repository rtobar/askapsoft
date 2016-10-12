/// @file SkyModelService.cc
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
#include "SkyModelService.h"

// Include package level header file
#include "askap_skymodel.h"

// System includes
#include <string>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <Common/ParameterSet.h>
#include <iceutils/CommunicatorConfig.h>
#include <iceutils/CommunicatorFactory.h>
#include <iceutils/ServiceManager.h>

// Local includes
#include "SkyModelServiceImpl.h"

ASKAP_LOGGER(logger, ".SkyModelService");

using namespace askap;
using namespace askap::cp::sms;
using namespace askap::cp::icewrapper;

SkyModelService::SkyModelService(const LOFAR::ParameterSet& parset) :
    itsParset(parset)
{
    ASKAPLOG_INFO_STR(logger, "ASKAP Sky Model Service - " << ASKAP_PACKAGE_VERSION);

    // grab Ice config from parset
    const LOFAR::ParameterSet& iceParset = parset.makeSubset("ice");
    const string locatorHost = iceParset.get("locator_host");
    const string locatorPort = iceParset.get("locator_port");
    const string serviceName = iceParset.get("service_name");
    const string adapterName = iceParset.get("adapter_name");
    ASKAPLOG_DEBUG_STR(logger, "locator host: " << locatorHost);
    ASKAPLOG_DEBUG_STR(logger, "locator port: " << locatorPort);
    ASKAPLOG_DEBUG_STR(logger, "service name: " << serviceName);
    ASKAPLOG_DEBUG_STR(logger, "adapter name: " << adapterName);

    // instantiate communicator
    CommunicatorConfig cc(locatorHost, locatorPort);
    cc.setAdapter(adapterName, "tcp", true);
    CommunicatorFactory commFactory;
    itsComm = commFactory.createCommunicator(cc);

    // instantiate Ice implementation class
    Ice::ObjectPtr smsImpl = new SkyModelServiceImpl();

    // assemble the service manager
    itsServiceManager.reset(new ServiceManager(itsComm, smsImpl, serviceName, adapterName));
}

SkyModelService::~SkyModelService()
{
    // stop the service manager
    if (itsServiceManager.get()) {
        itsServiceManager->stop();
        itsServiceManager.reset();
    }

    // destroy communicator
    itsComm->destroy();
}

void SkyModelService::run(void)
{
    itsServiceManager->start(false);
}
