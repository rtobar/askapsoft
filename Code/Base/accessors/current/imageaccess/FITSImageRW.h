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
#include <casacore/fits/FITS/fitsio.h>

#include "boost/scoped_ptr.hpp"

namespace askap {
namespace accessors {

/// @brief Extend FITSImage class functionality
/// @details It is made clear in the casacore implementation that there are
/// difficulties in writing general FITS access routines for writing.
/// I will implement what ASKAP needs here
/// @ingroup imageaccess

extern casa::FitsKeywordList theKeywordList;
extern bool created;

class FITSImageRW: public casa::FITSImage, public casa::ImageFITSConverter {

public:
    // Construct a FITSImage from the disk FITS file name  and extension and apply mask.

    explicit FITSImageRW(const casa::String& name, casa::uInt whichRep=0, casa::uInt whichHDU=0);

    // Construct a FITSImage from the disk FITS file name and extension and apply mask or not.
    FITSImageRW(const casa::String& name, const casa::MaskSpecifier& mask, casa::uInt whichRep=0, casa::uInt whichHDU=0);

    // Copy constructor (reference semantics)
    FITSImageRW(const FITSImageRW& other);

    // Destructor does nothing
    virtual ~FITSImageRW();

    // write into a FITS image
    bool write(const casa::Array<float>& );

    // build a FITS file with a header and coordinate system matching
    // This function heavily adapted from the casacore ImagetoFITS converters
    // made this static so it can be called without an instance.
    // All of the FITSimage parent calss constructors require the existance of
    // the file on disk. Which will not always be the case

    static bool create(const std::string &name, const casa::IPosition &shape,\
        const casa::CoordinateSystem &csys,\
        uint memoryInMB = 64,\
        bool preferVelocity = true,\
        bool opticalVelocity = true,\
        int BITPIX=-32,\
        float minPix = 1.0,\
        float maxPix = -1.0,\
        bool degenerateLast=false,\
        bool verbose=true,\
        bool stokesLast=false,\
        bool preferWavelength=false,\
        bool airWavelength=false,\
        bool primHead=true,\
        bool allowAppend=false,\
        bool history=true);

        /// keyword list from the primary array
        /// the same for all instances filled by create
        casa::FitsKeywordList theKeywordList;

    private:
        /// The name of the output file
        std::string name;

        /// pointer to the primary array
        boost::scoped_ptr<casa::PrimaryArray<float> > itsPrimaryArray;


};
}
}
#endif
