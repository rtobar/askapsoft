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
bool FITSImageRW::create(const std::string &name, const casa::IPosition &shape,\
    const casa::CoordinateSystem &csys,\
    uint memoryInMB, bool preferVelocity,\
	bool opticalVelocity, int BITPIX, float minPix, float maxPix,\
	bool degenerateLast, bool verbose, bool stokesLast,\
	bool preferWavelength, bool airWavelength, bool primHead,\
	bool allowAppend, bool history) {


    ASKAPLOG_INFO_STR(FITSlogger,"Creating R/W FITSImage");
    casa::String error;
    casa::TempImage<casa::Float> image(casa::TiledShape(shape),csys,0);
    //
    // Get coordinates and test that axis removal has been
    // mercifully absent
    //
    casa::CoordinateSystem cSys= image.coordinates();
    if (cSys.nWorldAxes() != cSys.nPixelAxes()) {
        error = "FITS requires that the number of world and pixel axes be"
        " identical.";
        ASKAPTHROW(AskapError,error);

    }
    //
    // Make degenerate axes last if requested
    // and make Stokes the very last if requested
    //

    casa::IPosition newShape = shape;
    const casa::uInt ndim = shape.nelements();
    casa::IPosition cursorOrder(ndim); // to be used later in the actual data copying
    for (casa::uInt i=0; i<ndim; i++) {
        cursorOrder(i) = i;
    }
    casa::Bool needNonOptimalCursor = casa::False; // the default value for the case no axis reordering is necessary
    if(stokesLast || degenerateLast){
        casa::Vector<casa::Int> order(ndim);
        casa::Vector<casa::String> cNames = cSys.worldAxisNames();
        casa::uInt nStokes = 0; // number of stokes axes
        if(stokesLast){
            for (casa::uInt i=0; i<ndim; i++) { // loop over axes
                order(i) = i;
                newShape(i) = shape(i);
            }
            for (casa::uInt i=0; i<ndim; i++) { // loop over axes
                if (cNames(i) == "Stokes") { // swap to back
                    nStokes++;
                    order(ndim-nStokes) = i;
                    newShape(ndim-nStokes) = shape(i);
                    order(i) = ndim-nStokes;
                    newShape(i) = shape(ndim-nStokes);
                }
            }
        }
        if(nStokes>0){ // apply the stokes reordering
            cSys.transpose(order,order);
        }
        if (degenerateLast) {
            // make sure the stokes axes stay where they are now
            for (casa::uInt i=ndim-nStokes; i<ndim; i++) {
                order(i) = i;
            }
            casa::uInt j = 0;
            for (casa::uInt i=0; i<ndim-nStokes; i++) { // loop over axes
                if (shape(i)>1) { // axis is not degenerate
                    order(j) = i; // put it in front, keeping order
                    newShape(j) = shape(i);
                    j++;
                }
            }
            for (casa::uInt i=0; i<ndim-nStokes; i++) { // loop over axes again
                if (shape(i)==1) { // axis is degenerate
                    order(j) = i;
                    newShape(j) = shape(i);
                    j++;
                }
            }
            cSys.transpose(order,order); // apply the degenerate reordering
        }
        for (casa::uInt i=0; i<ndim; i++) {
            cursorOrder(i) = order(i);
            if(order(i)!=(casa::Int)i){
                needNonOptimalCursor=casa::True;
            }
        }

    }
    //
    casa::Bool applyMask = casa::False;
    casa::Array<casa::Bool>* pMask = 0;
    if (image.isMasked()) {
        applyMask = casa::True;
        pMask = new casa::Array<casa::Bool>(casa::IPosition(0,0));
    }
    // //
    // // Find scale factors
    // //
    casa::Record header;
    casa::Double bscale, bzero;
    casa::Bool hasBlanks = casa::True;
    if (BITPIX == -32) {

        bscale = 1.0;
        bzero = 0.0;
        header.define("bitpix", BITPIX);
        header.setComment("bitpix", "Floating point (32 bit)");
        //
        // We don't yet know if the image has blanks or not, so assume it does.
        //
        hasBlanks = casa::True;
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

    if (applyMask && !hasBlanks) applyMask = casa::False;
    //
    casa::Vector<casa::Int> naxis(ndim);
    casa::uInt i;
    for (i=0; i < ndim; i++) {
        naxis(i) = newShape(i);
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


    // //
    casa::ImageInfo ii = image.imageInfo();
    if (!ii.toFITS (error, header)) {
        return false;
    }

    header.define("COMMENT1", ""); // inserts spaces
    // I should FITS-ize the units

    header.define("BUNIT", image.units().getName().chars());
    header.setComment("BUNIT", "Brightness (pixel) unit");
    //
    casa::IPosition shapeCopy = newShape;
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
        casa::IPosition shapeCopy = newShape;
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

    // Add in the fields from miscInfo that we can

    const casa::uInt nmisc = image.miscInfo().nfields();
    for (i=0; i<nmisc; i++) {
        casa::String tmp0 = image.miscInfo().name(i);
        casa::String miscname(tmp0.at(0,8));
        if (tmp0.length() > 8) {
            ASKAPLOG_INFO_STR(FITSlogger, "Truncating miscinfo field " << tmp0 \
            << " to " << miscname);
        }
        //
        if (miscname != "end" && miscname != "END") {
            if (header.isDefined(miscname)) {
                // These warnings just cause confusion.  They are usually
                // from the alt* keywords which FITSSpectralUtil writes.
                // They may also have been preserved in miscInfo when an
                // image came from FITS and hence the conflict.

                /*
                os << LogIO::WARN << "FITS keyword " << miscname
                << " is already defined so dropping it" << LogIO::POST;
                */
            } else {
                casa::DataType misctype = image.miscInfo().dataType(i);
                switch(misctype) {
                    case casa::TpBool:
                    header.define(miscname, image.miscInfo().asBool(i));
                    break;
                    case casa::TpChar:

                    case casa::TpUChar:

                    case casa::TpShort:

                    case casa::TpUShort:

                    case casa::TpInt:

                    case casa::TpUInt:
                    header.define(miscname, image.miscInfo().asInt(i));
                    break;
                    case casa::TpFloat:
                    header.define(miscname, image.miscInfo().asfloat(i));
                    break;
                    case casa::TpDouble:
                    header.define(miscname, image.miscInfo().asdouble(i));
                    break;
                    case casa::TpComplex:
                    header.define(miscname, image.miscInfo().asComplex(i));
                    break;
                    case casa::TpDComplex:
                    header.define(miscname, image.miscInfo().asDComplex(i));
                    break;
                    case casa::TpString:
                    if (miscname.contains("date") && miscname != "date") {
                        // Try to canonicalize dates (i.e. solve Y2K)
                        casa::String outdate;
                        // We only need to convert the date, the timesys we'll just
                        // copy through
                        if (casa::FITSDateUtil::convertDateString(outdate,\
                            image.miscInfo().asString(i))) {
                                // Conversion worked - change the header
                                header.define(miscname, outdate);
                        } else {
                                // conversion failed - just copy the existing date
                                header.define(miscname, image.miscInfo().asString(i));
                        }
                    } else {
                            // Just copy non-date strings through
                            header.define(miscname, image.miscInfo().asString(i));
                    }

                    break;
                    // These should be the cases that we actually see. I don't think
                    // asArray* converts types.

                    case casa::TpArrayBool:
                    header.define(miscname, image.miscInfo().asArrayBool(i));
                    break;
                    case casa::TpArrayChar:
                    case casa::TpArrayUShort:
                    case casa::TpArrayInt:
                    case casa::TpArrayUInt:
                    case casa::TpArrayInt64:
                    header.define(miscname, image.miscInfo().toArrayInt(i));
                    break;
                    case casa::TpArrayFloat:
                    header.define(miscname, image.miscInfo().asArrayfloat(i));
                    break;
                    case casa::TpArrayDouble:
                    header.define(miscname, image.miscInfo().asArraydouble(i));
                    break;
                    case casa::TpArrayString:
                    header.define(miscname, image.miscInfo().asArrayString(i));
                    break;
                    default:
                    {
                        ASKAPLOG_INFO_STR(FITSlogger, "Not writing miscInfo field '"  \
                        << miscname << "' - cannot handle type ");
                    }
                }
            }

        }
        if (header.isDefined(miscname)) {
            header.setComment(miscname, image.miscInfo().comment(i));
        }
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
    casa::FitsKeywordList kw;
    kw = casa::FITSKeywordUtil::makeKeywordList(primHead, casa::True);

    //kw.mk(FITS::EXTEND, True, "Tables may follow");
    // add the general keywords for WCS and so on
    ok = casa::FITSKeywordUtil::addKeywords(kw, header);
    if (! ok) {
        error = "Error creating initial FITS header";
        return false;
    }

    if(history){
        //
        // HISTORY
        //
        const casa::LoggerHolder& logger = image.logger();
        //
        vector<casa::String> historyChunk;
        casa::uInt nstrings;
        casa::Bool aipsppFormat;
        casa::uInt firstLine = 0;
        while (1) {
            firstLine = casa::FITSHistoryUtil::toHISTORY(historyChunk, aipsppFormat,\
                nstrings, firstLine, logger);
                if (nstrings == 0) {
                    break;
                }
                casa::String groupType;
                if (aipsppFormat) groupType = "LOGTABLE";
                casa::FITSHistoryUtil::addHistoryGroup(kw, historyChunk, nstrings, groupType);
        }
    }

    //
    // END
    //
    kw.end();
    casa::PrimaryArray<float>* fits32 = 0;

    casa::FitsOutput *outfile = new casa::FitsOutput(name.c_str(),casa::FITS::Disk);
    if (outfile->err())
        ASKAPLOG_WARN_STR(FITSlogger, "Error creating FITS file for output\n");


    fits32 = new casa::PrimaryArray<float>(kw);

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
    return true;

}
bool FITSImageRW::write(const casa::Array<float> &arr) {
    ASKAPLOG_INFO_STR(FITSlogger,"Writing array to FITS image");
    return false;
}
FITSImageRW::~FITSImageRW()
{
}
