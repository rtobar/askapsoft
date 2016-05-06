/// @file imager.cc
///
/// @copyright (c) 2009,2016 CSIRO
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
/// @author Stephen Ord <stephen.ord@csiro.au>

// Include package level header file
#include <askap_imager.h>

// System includes
#include <string>
#include <fstream>
#include <sstream>

// Boost includes
#include <boost/scoped_ptr.hpp>

// ASKAPsoft includes
#include "askap/Application.h"
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/StatReporter.h>
#include <Common/ParameterSet.h>
#include <parallel/ImagerParallel.h>

// Local Package includes
#include "distributedimager/ContinuumImager.h"
#include "distributedimager/MPIBasicComms.h"

using namespace askap;
using namespace askap::cp;

ASKAP_LOGGER(logger, ".main");

class ImagerApp : public askap::Application
{
    public:
        virtual int run(int argc, char* argv[])
        {
            askap::askapparallel::AskapParallel comms_p(argc, const_cast<const char **>(argv));
            StatReporter stats;

            try {
                // Create a subset

                LOFAR::ParameterSet subset(config().makeSubset("Imager."));

                // Instantiate the comms class

                ASKAPCHECK(comms_p.isParallel(), "This imager can only be run as a parallel MPI job");
                
                // Instantiate the Distributed Imager
                ContinuumImager imager(subset, comms_p);
                
                // runit
                imager.run();
            } catch (const askap::AskapError& e) {
                ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << e.what());
                std::cerr << "Askap error in " << argv[0] << ": " << e.what() << std::endl;
                comms_p.abort();
                return 1;
            } catch (const std::exception& e) {
                ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << e.what());
                std::cerr << "Unexpected exception in " << argv[0] << ": " << e.what()
                    << std::endl;
                comms_p.abort();
                return 1;
            }

            stats.logSummary();

            return 0;
        }
};

int main(int argc, char *argv[])
{
    ImagerApp app;
    return app.main(argc, argv);
}
