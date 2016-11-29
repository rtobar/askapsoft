/// @file VOTableComponent.cc
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
#include "VOTableComponent.h"

// Include package level header file
#include "askap_skymodel.h"

// System includes
#include <string>

// ASKAPsoft includes
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>
#include <votable/VOTable.h>

// Local package includes
#include "Utility.h"

ASKAP_LOGGER(logger, ".VOTableComponent");

using namespace askap::cp::sms;
using namespace askap::accessors;


VOTableComponent* VOTableComponent::create(long size)
{
    return new VOTableComponent(size);
}

VOTableComponent::VOTableComponent(long size)
{
}

VOTableComponent::~VOTableComponent()
{
    ASKAPLOG_DEBUG_STR(logger, "dtor");
}
