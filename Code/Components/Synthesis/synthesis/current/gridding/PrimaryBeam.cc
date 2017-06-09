/// @file PrimaryBeam.tcc
///
/// @abstract
/// Base class for primary beams
/// @ details
/// defines the interface to the Primary Beam structures for the purpose of image
/// based weighting or (via an illumination) the gridding.
///
#include "PrimaryBeam.h"
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".primarybeam.primarybeam");

namespace askap {
    namespace synthesis {

            PrimaryBeam::~PrimaryBeam() {

            }
            PrimaryBeam::ShPtr PrimaryBeam::createPrimaryBeam(const LOFAR::ParameterSet&)
            {
               ASKAPTHROW(AskapError, "createPrimaryBeam is supposed to be defined for every derived gridder, "
                                      "PrimaryBeam::createPrimaryBeam should never be called");
               return PrimaryBeam::ShPtr();
            }
    }
}
