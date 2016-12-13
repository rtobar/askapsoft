/// @file HealPixTools.cc
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
#include "HealPixTools.h"

// Include package level header file
#include "askap_skymodel.h"

// System includes
#include <string>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <Common/ParameterSet.h>
#include <boost/scoped_ptr.hpp>
#include <healpix_tables.h>
#include <pointing.h>

// Local includes
#include "SkyModelServiceImpl.h"
#include "Utility.h"

ASKAP_LOGGER(logger, ".HealPixTools");

using namespace std;
using namespace boost;
using namespace askap::cp::sms;

namespace askap {
namespace cp {
namespace sms {


HealPixTools::HealPixTools(int64_t order)
    :
    itsHealPixBase(2 << order, NEST, SET_NSIDE),
    itsNSide(2 << order)
{
}

int64_t HealPixTools::calcHealPixIndex(double ra, double dec) const
{
    // Note: this initial implementation is not likely to be the most efficient,
    // but it does give me enough to sort out the basics.
    // A more efficient option is likely to be threaded processing of
    // contiguous arrays of ra and dec coordinates, with the T_Healpix_Base
    // object being reused if possible, or thread-local if it is not
    // thread-safe.
    ASKAPASSERT((ra >= 0.0) && (ra < 360.0));
    ASKAPASSERT((dec >= -90.0) && (dec <= 90.0));

    // convert ra/dec to a pointing:
    // theta = (90 - dec)
    // phi = ra
    pointing p(
            utility::degreesToRadians(90.0 - dec),
            utility::degreesToRadians(ra));
    return itsHealPixBase.ang2pix(p);
}

vector<int64_t> HealPixTools::queryDisk(double ra, double dec, double radius, int fact) const
{
    // TODO: implement
    return vector<int64_t>();
}

};
};
};
