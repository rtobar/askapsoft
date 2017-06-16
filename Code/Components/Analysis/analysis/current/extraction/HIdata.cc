/// @file
///
/// Class to hold the extracted data for a single HI source
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <extraction/HIdata.h>

#include <askap_analysis.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <boost/shared_ptr.hpp>
#include <extraction/SourceSpectrumExtractor.h>
#include <extraction/NoiseSpectrumExtractor.h>
#include <extraction/MomentMapExtractor.h>
#include <extraction/CubeletExtractor.h>
#include <Common/ParameterSet.h>

ASKAP_LOGGER(logger, ".hidata");

namespace askap {

namespace analysis {

HIdata::HIdata(const LOFAR::ParameterSet &parset):
    itsParset(parset)
{
    itsCubeName = parset.getString("image", "");
    ASKAPCHECK(itsCubeName != "", "No cube name given");

    itsBeamLog = parset.getString("beamLog", "");

    // Define and create (if need be) the directories to hold the extracted data products
    std::stringstream cmd;
    std::string spectraDir = itsParset.getString("HiEmissionCatalogue.spectraDir","Spectra");
    std::string momentDir = itsParset.getString("HiEmissionCatalogue.momentDir","Moments");
    std::string cubeletDir = itsParset.getString("HiEmissionCatalogue.cubeletDir","Cubelets");
    cmd << " mkdir -p " << spectraDir << " " << momentDir << " " << cubeletDir;
    const int status = system(cmd.str().c_str());
    if (status != 0 ) {
        ASKAPTHROW(AskapError, "Error making directories for extracted data products: code = " << status
                   << " - Command = " << cmd);
    }
    
    // Define the parset used to set up the source extractor
    LOFAR::ParameterSet specParset;
    specParset.add(LOFAR::KVpair("spectralCube", itsCubeName));
    specParset.add(LOFAR::KVpair("spectralOutputBase",spectraDir+"/spectrum"));
    specParset.add(LOFAR::KVpair("useDetectedPixels", true));
    specParset.add(LOFAR::KVpair("scaleSpectraByBeam", true));
    specParset.add(LOFAR::KVpair("beamLog", itsBeamLog));
    specParset.add("imagetype",itsParset.getString("imagetype","fits"));
    itsSpecExtractor = boost::shared_ptr<SourceSpectrumExtractor>(new SourceSpectrumExtractor(specParset));

    // Define the parset used to set up the noise extractor
    LOFAR::ParameterSet noiseParset;
    noiseParset.add(LOFAR::KVpair("spectralCube", itsCubeName));
    noiseParset.add(LOFAR::KVpair("spectralOutputBase", spectraDir+"/noiseSpectrum"));
    noiseParset.add(LOFAR::KVpair("noiseArea",
                                  itsParset.getFloat("HiEmissionCatalogue.noiseArea", 50.)));
    noiseParset.add(LOFAR::KVpair("robust",
                                  itsParset.getBool("robust", true)));
    noiseParset.add(LOFAR::KVpair("useDetectedPixels", true));
    noiseParset.add(LOFAR::KVpair("scaleSpectraByBeam", false));
    noiseParset.add("imagetype",itsParset.getString("imagetype","fits"));
    itsNoiseExtractor = boost::shared_ptr<NoiseSpectrumExtractor>(new NoiseSpectrumExtractor(noiseParset));

    // Define the parset used to set up the moment-map extractor
    LOFAR::ParameterSet momentParset;
    momentParset.add(LOFAR::KVpair("spectralCube", itsCubeName));
    momentParset.add(LOFAR::KVpair("momentOutputBase",momentDir+"/mom%m"));
    momentParset.add("moments",itsParset.getString("HiEmissionCatalogue.moments", "[0,1,2]"));
    momentParset.add(LOFAR::KVpair("beamLog", itsBeamLog));
    momentParset.add("imagetype",itsParset.getString("imagetype","fits"));
    itsMomentExtractor = boost::shared_ptr<MomentMapExtractor>(new MomentMapExtractor(momentParset));

    // Define the parset used to set up the moment-map extractor
    LOFAR::ParameterSet cubeletParset;
    cubeletParset.add(LOFAR::KVpair("spectralCube", itsCubeName));
    cubeletParset.add(LOFAR::KVpair("cubeletOutputBase",cubeletDir+"/cubelet"));
    cubeletParset.add(LOFAR::KVpair("beamLog", itsBeamLog));
    cubeletParset.add("imagetype",itsParset.getString("imagetype","fits"));
    itsCubeletExtractor = boost::shared_ptr<CubeletExtractor>(new CubeletExtractor(cubeletParset));


}

void HIdata::findVoxelStats()
{

    itsFluxMax = itsSource->getPeakFlux();
    casa::IPosition start=itsCubeletExtractor->slicer().start().nonDegenerate();
    casa::Array<float> cubelet = itsCubeletExtractor->array().nonDegenerate();
    std::vector<PixelInfo::Voxel> voxelList = itsSource->getPixelSet();
    float min,sumf=0.,sumff=0.;
    std::vector<PixelInfo::Voxel>::iterator vox = voxelList.begin();
    for(;vox < voxelList.end(); vox++){
        //float flux = vox->getF();
        if (itsSource->isInObject(*vox)) {
            casa::IPosition loc(3, vox->getX(), vox->getY(), vox->getZ());
            float flux = cubelet(loc-start);
            if(vox==voxelList.begin()) {
                min = flux;
            } else {
                min = std::min(min,flux);
            }
            sumf += flux;
            sumff += flux*flux;
        }
    }
    float size=float(voxelList.size());
    itsFluxMin = min;
    itsFluxMean = sumf / size;
    itsFluxStddev = sqrt(sumff/size - sumf*sumf/size/size);
    itsFluxRMS = sqrt(sumff/size);

}


void HIdata::extract()
{
                        ASKAPLOG_DEBUG_STR(logger, "Extracting HI object with x=("<<itsSource->getXmin() << ","
                                           << itsSource->getXmax() << ")+" << itsSource->getXOffset()
                                           << ", y=("<<itsSource->getYmin() << "," << itsSource->getYmax()
                                           << ")+" << itsSource->getYOffset());
    extractSpectrum();
    extractNoise();
    extractMoments();
    extractCubelet();
}

void HIdata::extractSpectrum()
{
    itsSpecExtractor->setSource(itsSource);
    itsSpecExtractor->extract();
}

void HIdata::extractNoise()
{
    itsNoiseExtractor->setSource(itsSource);
    itsNoiseExtractor->extract();
}

void HIdata::extractMoments()
{
    itsMomentExtractor->setSource(itsSource);
    itsMomentExtractor->extract();
}

void HIdata::extractCubelet()
{
    itsCubeletExtractor->setSource(itsSource);
    itsCubeletExtractor->extract();
}

void HIdata::write()
{

    itsSpecExtractor->writeImage();
    itsNoiseExtractor->writeImage();
    itsMomentExtractor->writeImage();
    itsCubeletExtractor->writeImage();

}


}

}
