///
/// @file : Duchamp driver
///
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
///
/// @copyright (c) 2007 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <casa/Logging/LogIO.h>
#include <askap/Log4cxxLogSink.h>

#include <askapparallel/AskapParallel.h>
#include <analysisutilities/AnalysisUtilities.h>

#include <duchamp/duchamp.hh>

#include <Common/ParameterSet.h>

#include <stdexcept>
#include <iostream>

#include <casa/OS/Timer.h>

using std::cout;
using std::endl;

using namespace askap;
using namespace askap::analysis;

ASKAP_LOGGER(logger, "testMW.log");


// Main function
int main(int argc, const char** argv)
{
    // This class must have scope outside the main try/catch block
    askap::askapparallel::AskapParallel comms(argc, argv);
    try{
      ASKAPLOG_INFO_STR(logger, printWorkerPrefix(comms) << "rank = " << comms.rank());
      ASKAPLOG_INFO_STR(logger, printWorkerPrefix(comms) << "isParallel() = " << comms.isParallel());
      ASKAPLOG_INFO_STR(logger, printWorkerPrefix(comms) << "isMaster() = " << comms.isMaster());
      ASKAPLOG_INFO_STR(logger, printWorkerPrefix(comms) << "isWorker() = " << comms.isWorker());
        ///==============================================================================
    } catch (const askap::AskapError& x) {
        ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
        std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    } catch (const std::exception& x) {
        ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
        std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
        exit(1);
    }

    return 0;
}