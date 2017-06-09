/// @file PrimaryBeam.tcc
///
/// @abstract
/// Base class for primary beams
/// @ details
/// defines the interface to the Primary Beam structures for the purpose of image
/// based weighting or (via an illumination) the gridding.
///
#include "PrimaryBeam.h"
#include "GaussianPB.h"
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".primarybeam.gaussianpb");
namespace askap {
    namespace synthesis {

            GaussianPB::~GaussianPB() {

            }
            PrimaryBeam::ShPtr GaussianPB::createPrimaryBeam(const LOFAR::ParameterSet&)
            {
               ASKAPLOG_DEBUG_STR(logger, "createPrimaryBeam for the Gaussian Primary Beam ");

               return PrimaryBeam::ShPtr();
            }
    }
}
