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

        GaussianPB();

        static inline std::string PrimaryBeamName() { return "GaussianPB";}

        virtual ~GaussianPB();

        GaussianPB(const GaussianPB &other);

        static PrimaryBeam::ShPtr createPrimaryBeam(const LOFAR::ParameterSet &parset);



    private:


    };

} // namespace synthesis

} // namespace askap


#endif // #ifndef ASKAP_SYNTHESIS_GAUSSIAN_H
