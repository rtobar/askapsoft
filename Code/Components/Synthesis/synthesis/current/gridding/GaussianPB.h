/// @file GaussianPB.h

/// @brief Standard Gaussian Primary Beam
/// @details
///

#ifndef ASKAP_SYNTHESIS_GAUSSIAN_H
#define ASKAP_SYNTHESIS_GAUSSIAN_H

#include <boost/shared_ptr.hpp>
#include <gridding/IBasicIllumination.h>
#include <gridding/PrimaryBeam.h>
#include <Common/ParameterSet.h>


namespace askap {
namespace synthesis {

    class GaussianPB : public PrimaryBeam

    {

    public:

        typedef boost::shared_ptr<GaussianPB> ShPtr;

        GaussianPB();

        static inline std::string PrimaryBeamName() { return "GaussianPB";}

        virtual ~GaussianPB();

        GaussianPB(const GaussianPB &other);

        static PrimaryBeam::ShPtr createPrimaryBeam(const LOFAR::ParameterSet &parset);

        /// Set some parameters

        void setApertureSize(double apsize) {this->ApertureSize = apsize; };

        void setFWHMScaling(double fwhmScale) {this->FWHMScaling = fwhmScale;};

        void setExpScaling(double expScale) {this->ExpScaling = expScale;};

        /// Get some parameters

        double getFWHM(const double frequency);

        double getExpScaling()
        {return this->ExpScaling;};

        virtual double evaluateAtOffset(double offset, double frequency);

        /// Probably should have a "generate weight" - that calls evaluate for
        /// every pixel ....

    private:

        // Size of the telescope aperture
        double ApertureSize;

        // scaling of FWHM to match simulations
        double FWHMScaling;

        // Further scaling of the Gaussian exponent
        double ExpScaling;

    };

} // namespace synthesis

} // namespace askap


#endif // #ifndef ASKAP_SYNTHESIS_GAUSSIAN_H
