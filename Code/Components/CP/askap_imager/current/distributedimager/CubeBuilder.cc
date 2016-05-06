/// @file CubeBuilder.cc
///
/// @copyright (c) 2013 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// Include own header file first
#include <distributedimager/CubeBuilder.h>

// Include package level header file
#include <askap_imager.h>

// System includes
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

// ASKAPsoft includes
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <Common/ParameterSet.h>
#include <utils/PolConverter.h>
#include <casacore/casa/Arrays/IPosition.h>
#include <casacore/coordinates/Coordinates/SpectralCoordinate.h>
#include <casacore/coordinates/Coordinates/DirectionCoordinate.h>
#include <casacore/coordinates/Coordinates/StokesCoordinate.h>
#include <casacore/coordinates/Coordinates/CoordinateSystem.h>
#include <casacore/measures/Measures/Stokes.h>
#include <casacore/images/Images/PagedImage.h>
#include <casacore/casa/Quanta/Unit.h>
#include <casacore/casa/Quanta/QC.h>

ASKAP_LOGGER(logger, ".CubeBuilder");

using namespace askap::cp;
using namespace casa;
using namespace std;
using namespace askap::synthesis;

CubeBuilder::CubeBuilder(const LOFAR::ParameterSet& parset,
                         const casa::uInt nchan,
                         const casa::Quantity& f0,
                         const casa::Quantity& inc,
                         const std::string& name)
{
    itsFilename = parset.getString("Images.name");

    ASKAPCHECK(itsFilename.substr(0,5)=="image",
               "Simager.Images.name must start with 'image'");

    // If necessary, replace "image" with _name_ (e.g. "psf", "weights")
    // unless name='restored', in which case we append ".restored"
    if (!name.empty()) {
        if (name == "restored") {
            itsFilename = itsFilename + ".restored";
        } else {
            const string orig = "image";
            const size_t f = itsFilename.find(orig);
            itsFilename.replace(f, orig.length(), name);
        }
    }

    const std::string restFreqString = parset.getString("Images.restFrequency", "-1.");
    if (restFreqString == "HI") {
        itsRestFrequency = casa::QC::HI;
    } else {
        itsRestFrequency = SynthesisParamsHelper::convertQuantity(restFreqString, "Hz");
    }

    // Polarisation
    const std::vector<std::string>
        stokesVec = parset.getStringVector("Images.polarisation", std::vector<std::string>(1,"I"));
    // there could be many ways to define stokes, e.g. ["XX YY"] or ["XX","YY"] or "XX,YY"
    // to allow some flexibility we have to concatenate all elements first and then 
    // allow the parser from PolConverter to take care of extracting the products.                                            
    std::string stokesStr;
    for (size_t i=0; i<stokesVec.size(); ++i) {
        stokesStr += stokesVec[i];
    }
    itsStokes = scimath::PolConverter::fromString(stokesStr);
    const casa::uInt npol=itsStokes.size();
    
    // Get the image shape
    const vector<casa::uInt> imageShapeVector = parset.getUintVector("Images.shape");
    const casa::uInt nx = imageShapeVector[0];
    const casa::uInt ny = imageShapeVector[1];
    const casa::IPosition cubeShape(4, nx, ny, npol, nchan);

    // Use a tile shape appropriate for plane-by-plane access
    casa::IPosition tileShape(cubeShape.nelements(), 1);
    tileShape(0) = 256;
    tileShape(1) = 256;

    const casa::CoordinateSystem csys = createCoordinateSystem(parset, nx, ny, f0, inc);

    ASKAPLOG_DEBUG_STR(logger, "Creating Cube " << itsFilename <<
                       " with shape [xsize:" << nx << " ysize:" << ny <<
                       " npol:" << npol << " nchan:" << nchan <<
                       "], f0: " << f0.getValue("MHz") << " MHz, finc: " <<
                       inc.getValue("kHz") << " kHz");
    itsCube.reset(new casa::PagedImage<float>(casa::TiledShape(cubeShape, tileShape),
                  csys, itsFilename));

    // default flux units are Jy/pixel. If we set the restoring beam
    // later on, can set to Jy/beam
    setUnits("Jy/pixel");
}

CubeBuilder::~CubeBuilder()
{
}

void CubeBuilder::writeSlice(const casa::Array<float>& arr, const casa::uInt chan)
{
    casa::IPosition where(4, 0, 0, 0, chan);
    itsCube->putSlice(arr, where);
}

casa::CoordinateSystem
CubeBuilder::createCoordinateSystem(const LOFAR::ParameterSet& parset,
                                    const casa::uInt nx,
                                    const casa::uInt ny,
                                    const casa::Quantity& f0,
                                    const casa::Quantity& inc)
{
    CoordinateSystem coordsys;
    const vector<string> dirVector = parset.getStringVector("Images.direction");
    const vector<string> cellSizeVector = parset.getStringVector("Images.cellsize");

    // Direction Coordinate
    {
        Matrix<Double> xform(2, 2);
        xform = 0.0;
        xform.diagonal() = 1.0;
        const Quantum<Double> ra = asQuantity(dirVector.at(0), "deg");
        const Quantum<Double> dec = asQuantity(dirVector.at(1), "deg");
        ASKAPLOG_DEBUG_STR(logger, "Direction: " << ra.getValue() << " degrees, "
                           << dec.getValue() << " degrees");

        const Quantum<Double> xcellsize = asQuantity(cellSizeVector.at(0), "arcsec") * -1.0;
        const Quantum<Double> ycellsize = asQuantity(cellSizeVector.at(1), "arcsec");
        ASKAPLOG_DEBUG_STR(logger, "Cellsize: " << xcellsize.getValue()
                           << " arcsec, " << ycellsize.getValue() << " arcsec");

        casa::MDirection::Types type;
        casa::MDirection::getType(type, dirVector.at(2));
        const DirectionCoordinate radec(type, Projection(Projection::SIN),
                                        ra, dec, xcellsize, ycellsize,
                                        xform, nx / 2, ny / 2);

        coordsys.addCoordinate(radec);
    }

    // Stokes Coordinate
    {

        // To make a StokesCoordinate, need to convert the StokesTypes
        // into integers explicitly
        casa::Vector<casa::Int> stokes(itsStokes.size());
        for(unsigned int i=0;i<stokes.size();i++){
            stokes[i] = itsStokes[i];
        }
        const StokesCoordinate stokescoord(stokes);
        coordsys.addCoordinate(stokescoord);
        
    }
    // Spectral Coordinate
    {
        const Double refPix = 0.0;  // is the reference pixel
        SpectralCoordinate sc(MFrequency::TOPO, f0, inc, refPix);

        // add rest frequency, but only if requested, and only for
        // image.blah, residual.blah, image.blah.restored
        if (itsRestFrequency.getValue("Hz") > 0.) {
            if ((itsFilename.find("image.") != string::npos) ||
                    (itsFilename.find("residual.") != string::npos)) {

                if (!sc.setRestFrequency(itsRestFrequency.getValue("Hz"))) {
                    ASKAPLOG_ERROR_STR(logger, "Could not set the rest frequency to " <<
                                       itsRestFrequency.getValue("Hz") << "Hz");
                }
            }
        }

        coordsys.addCoordinate(sc);
    }

    return coordsys;
}

void CubeBuilder::addBeam(casa::Vector<casa::Quantum<double> > &beam)
{
    casa::ImageInfo ii = itsCube->imageInfo();
    ii.setRestoringBeam(beam);
    itsCube->setImageInfo(ii);
    setUnits("Jy/beam");
}

void CubeBuilder::setUnits(const std::string &units)
{
    itsCube->setUnits(casa::Unit(units));
}

