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
#ifndef ASKAP_ANALYSIS_HI_DATA_H_
#define ASKAP_ANALYSIS_HI_DATA_H_

#include <boost/shared_ptr.hpp>
#include <extraction/SourceSpectrumExtractor.h>
#include <extraction/NoiseSpectrumExtractor.h>
#include <extraction/MomentMapExtractor.h>
#include <extraction/CubeletExtractor.h>
#include <Common/ParameterSet.h>

namespace askap {

namespace analysis {

/// @brief Class to hold extracted data used for HI analysis

/// @details This class relates to a specific HI source, and holds
/// extracted source & noise spectra, moment maps, and a cubelet. It
/// provides methods to obtain the extracted arrays for external
/// use. It will provide mechanisms to fit to the moment-0 map and to
/// the integrated spectrum, to support the HI catalogue.
class HIdata {
    public:
        HIdata(const LOFAR::ParameterSet &parset);
        virtual ~HIdata() {};

    /// @brief Set the source to be used.
    void setSource(RadioSource *src) {itsSource=src;};
    
    /// @brief Front-end for the extract functions
    void extract();
    /// @brief Extract the source spectrum using itsSpecExtractor
    void extractSpectrum();
    /// @brief Extract the noise spectrum using itsNoiseExtractor
    void extractNoise();
    /// @brief Extract the moment maps using itsMomentExtractor
    void extractMoments();
    /// @brief Extract the surrounding cubelet using itsCubeletExtractor
    void extractCubelet();

    /// @brief Call the writeImage() function for each extractor
    void write();

    // /// @brief The extracted integrated spectrum
    // casa::Array<float> spectrum();
    // /// @brief The extracted moment-0 map
    // casa::Array<float> moment0();

    // /// @brief Fit a Gaussian to the moment-0 map
    // void fitToMom0(){};
    // //    /// @brief Return the results of the moment-0 Gaussian fit
    

    // /// @brief Fit a "busy-function" to the integrated spectrum
    // void busyFunctionFit(){};

protected:

    /// @brief Parset relating to HI parameters
    LOFAR::ParameterSet            itsParset;
    /// @brief Pointer to defining radio source
    RadioSource                   *itsSource;
    /// @brief Name of the input cube
    std::string                    itsCubeName;
    /// @brief Beam Log recording restoring beam per channel
    std::string                    itsBeamLog;

    /// @brief Extractor to obtain the source spectrum
    boost::shared_ptr<SourceSpectrumExtractor> itsSpecExtractor;
    /// @brief Extractor to obtain the noise spectrum
    boost::shared_ptr<NoiseSpectrumExtractor>  itsNoiseExtractor;
    /// @brief Extractor to obtain the moment maps (contains mom-0,1,2)
    boost::shared_ptr<MomentMapExtractor>      itsMomentExtractor;
    /// @brief Extractor to obtain the cubelets
    boost::shared_ptr<CubeletExtractor>        itsCubeletExtractor;

};

}

}

#endif

