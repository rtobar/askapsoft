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
#include <casacore/fits/FITS/fitsio.h>
#include <casacore/fits/FITS/FITSDateUtil.h>
#include <casacore/fits/FITS/FITSHistoryUtil.h>
#include <casacore/fits/FITS/FITSReader.h>

#include <casacore/casa/Quanta/MVTime.h>
#include <imageaccess/FITSImageRW.h>

ASKAP_LOGGER(FITSlogger, ".FITSImageRW");

using namespace askap;
using namespace askap::accessors;

casa::FitsKeywordList askap::accessors::theKeywordList;
bool askap::accessors::created;

FITSImageRW::FITSImageRW(const casa::String& name, casa::uInt whichRep, casa::uInt whichHDU)
: FITSImage(name, whichRep, whichHDU)
{
    ASKAPLOG_INFO_STR(FITSlogger,"Instantiating FITSImageRW");
    extern bool created;
    extern casa::FitsKeywordList theKeywordList;
    if (created == true)
        itsPrimaryArray.reset(new casa::PrimaryArray<float>(theKeywordList));





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
bool FITSImageRW::create(const std::string &name, const casa::IPosition &shape,\
    const casa::CoordinateSystem &csys,\
    uint memoryInMB, bool preferVelocity,\
	bool opticalVelocity, int BITPIX, float minPix, float maxPix,\
	bool degenerateLast, bool verbose, bool stokesLast,\
	bool preferWavelength, bool airWavelength, bool primHead,\
	bool allowAppend, bool history) {

    extern casa::FitsKeywordList theKeywordList;
    extern bool created;

    ASKAPLOG_INFO_STR(FITSlogger,"Creating R/W FITSImage");


    casa::String error;
    const casa::uInt ndim = shape.nelements();
    // //
    // // Find scale factors
    // //
    casa::Record header;
    casa::Double bscale, bzero;

    if (BITPIX == -32) {

        bscale = 1.0;
        bzero = 0.0;
        header.define("bitpix", BITPIX);
        header.setComment("bitpix", "Floating point (32 bit)");

    }

    else {
        error =
        "BITPIX must be -32 (floating point)";
        return false;
    }
    //
    // At this point, for 32 floating point, we must apply the given
    // mask.  For 16bit, we may know that there are in fact no blanks
    // in the image, so we can dispense with looking at the mask again.


    //
    casa::Vector<casa::Int> naxis(ndim);
    casa::uInt i;
    for (i=0; i < ndim; i++) {
        naxis(i) = shape(i);
    }
    header.define("naxis", naxis);
    if (allowAppend)
        header.define("extend", casa::True);
    if (!primHead){
        header.define("PCOUNT", 0);
        header.define("GCOUNT", 1);
    }
    header.define("bscale", bscale);
    header.setComment("bscale", "PHYSICAL = PIXEL*BSCALE + BZERO");
    header.define("bzero", bzero);


    header.define("COMMENT1", ""); // inserts spaces
    // I should FITS-ize the units

    header.define("BUNIT", "UNKNOWN");
    header.setComment("BUNIT", "Brightness (pixel) unit");
    //
    casa::IPosition shapeCopy = shape;
    casa::CoordinateSystem cSys = csys;

    casa::Record saveHeader(header);
    casa::Bool ok = cSys.toFITSHeader(header, shapeCopy, casa::True, 'c', casa::True, // use WCS
    preferVelocity, opticalVelocity,
    preferWavelength, airWavelength);
    if (!ok) {
        ASKAPLOG_WARN_STR(FITSlogger, "Could not make a standard FITS header. Setting" \
        <<	" a simple linear coordinate system.") ;

        casa::uInt n = cSys.nWorldAxes();
        casa::Matrix<casa::Double> pc(n,n); pc=0.0; pc.diagonal() = 1.0;
        casa::LinearCoordinate linear(cSys.worldAxisNames(),
        cSys.worldAxisUnits(),
        cSys.referenceValue(),
        cSys.increment(),
        cSys.linearTransform(),
        cSys.referencePixel());
        casa::CoordinateSystem linCS;
        linCS.addCoordinate(linear);

        // Recover old header before it got mangled by toFITSHeader

        header = saveHeader;
        casa::IPosition shapeCopy = shape;
        casa::Bool ok = linCS.toFITSHeader(header, shapeCopy, casa::True, 'c', casa::False); // don't use WCS
        if (!ok) {
            ASKAPLOG_WARN_STR(FITSlogger,"Fallback linear coordinate system fails also.");
            return false;
        }
    }
    // When this if test is True, it means some pixel axes had been removed from
    // the coordinate system and degenerate axes were added.

    if (naxis.nelements() != shapeCopy.nelements()) {
        naxis.resize(shapeCopy.nelements());
        for (casa::uInt j=0; j < shapeCopy.nelements(); j++) {
            naxis(j) = shapeCopy(j);
        }
        header.define("NAXIS", naxis);
    }

    //
    // DATE
    //

    casa::String date, timesys;
    casa::Time nowtime;
    casa::MVTime now(nowtime);
    casa::FITSDateUtil::toFITS(date, timesys, now);
    header.define("date", date);
    header.setComment("date", "Date FITS file was written");
    if (!header.isDefined("timesys") && !header.isDefined("TIMESYS")) {
        header.define("timesys", timesys);
        header.setComment("timesys", "Time system for HDU");
    }
    // //
    // // ORIGIN
    // //

    header.define("ORIGIN", "ASKAPSoft");


    // // Set up the FITS header
    extern casa::FitsKeywordList theKeywordList;

    theKeywordList = casa::FITSKeywordUtil::makeKeywordList(primHead, casa::True);

    //kw.mk(FITS::EXTEND, True, "Tables may follow");
    // add the general keywords for WCS and so on
    ok = casa::FITSKeywordUtil::addKeywords(theKeywordList, header);
    if (! ok) {
        error = "Error creating initial FITS header";
        return false;
    }


    //
    // END
    //
    theKeywordList.end();
    casa::PrimaryArray<float>* fits32 = 0;

    casa::FitsOutput *outfile = new casa::FitsOutput(name.c_str(),casa::FITS::Disk);
    if (outfile->err())
        ASKAPLOG_WARN_STR(FITSlogger, "Error creating FITS file for output\n");


    fits32 = new casa::PrimaryArray<float>(theKeywordList);

    if (fits32==0 || fits32->err()) {
        ASKAPLOG_WARN_STR(FITSlogger, "Error creating Primary HDU from keywords");
        return false;
    }
    if (fits32->write_hdr(*outfile)) {
        ASKAPLOG_INFO_STR(FITSlogger,"Error writing FITS header");
        delete outfile;
        return false;
    }
    ASKAPLOG_INFO_STR(FITSlogger,"Written header");


    std::cout << *fits32 << std::endl;
    delete(fits32);
    delete(outfile);

    created = true;
    return true;

}
bool FITSImageRW::write(const casa::Array<float> &arr) {
    ASKAPLOG_INFO_STR(FITSlogger,"Writing array to FITS image");

    casa::FitsOutput *fout = new casa::FitsOutput(this->name.c_str(),casa::FITS::Disk);
    casa::Bool deleteIt;
    itsPrimaryArray->store(arr.getStorage(deleteIt));
    itsPrimaryArray->write(*fout);
    delete(fout);

    return false;
}
FITSImageRW::~FITSImageRW()
{
}
