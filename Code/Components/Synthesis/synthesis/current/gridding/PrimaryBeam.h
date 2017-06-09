/// @file PrimaryBeam.h
///
/// @abstract
/// Base class for primary beams
/// @ details
/// defines the interface to the Primary Beam structures for the purpose of image
/// based weighting or (via an illumination) the gridding.
///

#ifndef ASKAP_SYNTHESIS_PRIMARY_BEAM_H

#define ASKAP_SYNTHESIS_PRIMARY_BEAM_H

#include <Common/ParameterSet.h>
#include <boost/shared_ptr.hpp>

namespace askap
{
    namespace synthesis
    {

        class PrimaryBeam {

        public:

            /// Shared pointer definition
            typedef boost::shared_ptr<PrimaryBeam> ShPtr;

            PrimaryBeam();
            virtual ~PrimaryBeam();

            PrimaryBeam(const PrimaryBeam &other);

            /// This has to be static as we need to access it in the register even
            /// if there is not instantiated class.
            static ShPtr createPrimaryBeam(const LOFAR::ParameterSet& parset);

        private:

        }; // class
    } // synthesis
} //askap



#endif
