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


            GaussianPB::GaussianPB() {
                ASKAPLOG_DEBUG_STR(logger,"GaussianPB default contructor");
            }

            GaussianPB::~GaussianPB() {

            }
            GaussianPB::GaussianPB(const GaussianPB &other) :
            PrimaryBeam(other)
            {
                ASKAPLOG_DEBUG_STR(logger,"GaussianPB copy contructor");
            }
            PrimaryBeam::ShPtr GaussianPB::createPrimaryBeam(const LOFAR::ParameterSet&)
            {
               ASKAPLOG_DEBUG_STR(logger, "createPrimaryBeam for the Gaussian Primary Beam ");

               PrimaryBeam::ShPtr ptr;
               ptr.reset( new GaussianPB());
               return ptr;

            }
    }
}
