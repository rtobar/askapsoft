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
#include "Ice/Ice.h"

// Local package includes


using namespace askap::cp::sms;
using namespace askap::interfaces::skymodelservice;

/// @brief Constructor.
SkyModelServiceImpl::SkyModelServiceImpl()
{
}

/// @brief Destructor.
SkyModelServiceImpl::~SkyModelServiceImpl()
{
}

std::string SkyModelServiceImpl::getServiceVersion(const Ice::Current&)
{
    return "1.0";
}

ComponentIdSeq SkyModelServiceImpl::coneSearch(
    double rightAscension,
    double declination,
    double searchRadius,
    double fluxLimit,
    const Ice::Current&)
{
    return ComponentIdSeq();
}

ComponentSeq SkyModelServiceImpl::getComponents(
    const ComponentIdSeq& componentIds,
    const Ice::Current&)
{
    return ComponentSeq();
}

ComponentIdSeq SkyModelServiceImpl::addComponents(
    const ComponentSeq& components,
    const Ice::Current&)
{
    return ComponentIdSeq();
}

void SkyModelServiceImpl::removeComponents(
    const ComponentIdSeq& componentIds,
    const Ice::Current&)
{
}
