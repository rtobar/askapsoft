/// @file ContinuumComponent.h
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

#ifndef ASKAP_CP_SMS_CONTINUUMCOMPONENT_H
#define ASKAP_CP_SMS_CONTINUUMCOMPONENT_H

// System includes
#include <string>

// ASKAPsoft includes
#include <odb/core.hxx>

// Local package includes


namespace askap {
namespace cp {
namespace sms {
namespace datamodel {

// Datamodel versioning
// Disabled for now as I don't need it until we get closer to production.
#pragma db model version(1, 1)

// Map C++ bool to an INT NOT NULL database type
#pragma db value(bool) type("INT")

/// @brief Datamodel class for Continuum Components
/// Do not edit the version of this file in the `datamodel` directory, as it is
/// a copy of the files in the `schema` directory.

#pragma db object optimistic
struct ContinuumComponent {
    ContinuumComponent() {}

    #include "continuum.i"
};

};
};
};
};

#endif
