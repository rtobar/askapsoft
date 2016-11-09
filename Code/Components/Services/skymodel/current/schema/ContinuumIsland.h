/// @file ContinuumIsland.h
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

#ifndef ASKAP_CP_SMS_CONTINUUMISLAND_H
#define ASKAP_CP_SMS_CONTINUUMISLAND_H

// System includes
#include <memory>
#include <string>
#include <vector>

// ASKAPsoft includes
#include <odb/core.hxx>

// Local package includes
#include "ContinuumComponent.h"


namespace askap {
namespace cp {
namespace sms {
namespace datamodel {

// Datamodel versioning
// Disabled for now as I don't need it until we get closer to production.
#pragma db model version(1, 1)

// Map C++ bool to an INT NOT NULL database type
#pragma db value(bool) type("INT")

/// @brief Datamodel class for Continuum Islands
/// Do not edit the version of this file in the `datamodel` directory, as it is
/// a copy of the files in the `schema` directory.

#pragma db object optimistic
struct ContinuumIsland {
    ContinuumIsland() {}

    #include "ContinuumIsland.i"

    // Define a To-Many relationship to ContinuumComponent
    #pragma db value_not_null unordered
    std::vector<ContinuumComponent*> components;
};

};
};
};
};

#endif
