/// @file FITSImageRW.cc
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
#include <askap_accessors.h>
#include <askap/AskapLogging.h>

#include <casacore/images/Images/FITSImage.h>
#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Utilities/DataType.h>

#include <imageaccess/FITSImageRW.h>

ASKAP_LOGGER(FITSlogger, ".FITSImageRW");

using namespace askap;
using namespace askap::accessors;

FITSImageRW::FITSImageRW(const casa::String& name, casa::uInt whichRep, casa::uInt whichHDU)
: FITSImage(name, whichRep, whichHDU)
{
    ASKAPLOG_INFO_STR(FITSlogger,"Instantiating FITSImageRW");
}
FITSImageRW::FITSImageRW(const casa::String& name, const casa::MaskSpecifier& mask, casa::uInt whichRep, casa::uInt whichHDU)
: FITSImage(name, mask, whichRep, whichHDU)
{
    ASKAPLOG_INFO_STR(FITSlogger,"Instantiating FITSImageRW");
}
FITSImageRW::FITSImageRW(const FITSImageRW& other)
: FITSImage(other)
{
    ASKAPLOG_INFO_STR(FITSlogger,"Entering Copy FITSImageRW Constructor");
}
bool FITSImageRW::create(const casa::String& name) {
    ASKAPLOG_INFO_STR(FITSlogger,"Creating barebones FITS image");
    return false;
}
FITSImageRW::~FITSImageRW()
{
}
