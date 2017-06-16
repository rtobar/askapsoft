/// @file PrimaryBeam.tcc
///
/// @abstract
/// Base class for primary beams
/// @ details
/// defines the interface to the Primary Beam structures for the purpose of image
/// based weighting or (via an illumination) the gridding.
///
#include "askap_imagemath.h"


#include "PrimaryBeam.h"
#include "GaussianPB.h"
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>
#include <Common/ParameterSet.h>

ASKAP_LOGGER(logger, ".primarybeam.gaussianpb");
namespace askap {
    namespace imagemath {


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
            PrimaryBeam::ShPtr GaussianPB::createPrimaryBeam(const LOFAR::ParameterSet &parset)
            {
               ASKAPLOG_DEBUG_STR(logger, "createPrimaryBeam for the Gaussian Primary Beam ");

               // this is static so use this to create the instance....

               // just for logging, declare private handle to avoid issues with template
               // log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(askap::generateLoggerName(std::string("createGaussianPB")));
               //

               // These pretty much define the pb as
               // exp(-1(offset*offset)*expscaling/fwhm/fwhm)
               // fwhm is a function of frequency so is only known when that is known


               GaussianPB::ShPtr ptr;

               // We need to pull all the parameters out of the parset - and set
               // all the private variables required to define the beam


               ptr.reset( new GaussianPB());

               ptr->setApertureSize(parset.getDouble("aperture",12));
               ptr->setFWHMScaling(parset.getDouble("fwhmscaling", 1.00));
               ptr->setExpScaling(parset.getDouble("expscaling", 4.*log(2.)));

               ASKAPLOG_DEBUG_STR(logger,"Created Gaussian PB instance");
               return ptr;

            }

            double GaussianPB::evaluateAtOffset(double offset, double frequency) {

                double pb = exp(-offset*offset*getExpScaling()/(getFWHM(frequency)*getFWHM(frequency)));
                return pb;

            }

            double GaussianPB::getFWHM(const double frequency) {
                double sol = 299792458.0;
                double fwhm = sol/frequency/this->ApertureSize;
                return fwhm;
            }
    }
}
