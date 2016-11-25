/// @file VoTableComponent.h
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

#ifndef ASKAP_CP_SMS_VOTABLECOMPONENT_H
#define ASKAP_CP_SMS_VOTABLECOMPONENT_H

// System includes
#include <string>

// ASKAPsoft includes
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>


// Local package includes

namespace askap {
namespace cp {
namespace sms {

/// @brief VO table data container for Component data
class VoTableComponent :
    private boost::noncopyable {
    public:

        /// @brief Factory method for constructing the VoTableComponent implementation.
        ///
        /// @param size The number of components for which space should be preallocated.
        /// @return The VoTableComponent instance.
        /// @throw AskapError   If the implementation cannot be constructed.
        static VoTableComponent* create(long size);

        /// @brief Destructor.
        virtual ~VoTableComponent();

    private:
        /// @brief Constructor.
        /// Private. Use the factory method to create.
        ///
        /// @param size The number of components for which space should be preallocated.
        VoTableComponent(long size);
};

}
}
}

#endif
