/// @file FITSImageRW.h
/// @brief Read/Write FITS image class
/// @details This class implements the write methods that are absent
/// from the casacore FITSImage.
///
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
/// @author Stephen Ord <stephen.ord@csiro.au
///
#ifndef ASKAP_ACCESSORS_FITS_IMAGE_RW_H
#define ASKAP_ACCESSORS_FITS_IMAGE_RW_H

#include <casacore/images/Images/FITSImage.h>
#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Utilities/DataType.h>

namespace askap {
namespace accessors {

/// @brief Extend FITSImage class functionality
/// @details It is made clear in the casacore implementation that there are
/// difficulties in writing general FITS access routines for writing.
/// I will implement what ASKAP needs here
/// @ingroup imageaccess

class FITSImageRW: public casa::FITSImage {

    // Construct a FITSImage from the disk FITS file name  and extension and apply mask.

    explicit FITSImageRW(const casa::String& name, casa::uInt whichRep=0, casa::uInt whichHDU=0);

    // Construct a FITSImage from the disk FITS file name and extension and apply mask or not.
    FITSImageRW(const casa::String& name, const casa::MaskSpecifier& mask, casa::uInt whichRep=0, casa::uInt whichHDU=0);

    // Copy constructor (reference semantics)
    FITSImageRW(const FITSImageRW& other);

    // Destructor does nothing
    virtual ~FITSImageRW();

    // create an empty fits image of shape
    bool create(const casa::String& name);



};
}
}
#endif
