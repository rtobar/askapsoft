/// @file ContinuumWorker.cc
///
/// @copyright (c) 2009 CSIRO
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

// Include own header file first
#include "ContinuumWorker.h"

// System includes
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <fitting/Equation.h>
#include <fitting/INormalEquations.h>
#include <fitting/ImagingNormalEquations.h>
#include <fitting/Params.h>
#include <gridding/IVisGridder.h>
#include <gridding/VisGridderFactory.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <dataaccess/IConstDataSource.h>
#include <dataaccess/TableConstDataSource.h>
#include <dataaccess/IConstDataIterator.h>
#include <dataaccess/IDataConverter.h>
#include <dataaccess/IDataSelector.h>
#include <dataaccess/IDataIterator.h>
#include <dataaccess/SharedIter.h>
#include <utils/PolConverter.h>
#include <Common/ParameterSet.h>
#include <Common/Exceptions.h>
#include <casacore/casa/OS/Timer.h>
#include <parallel/ImagerParallel.h>

// CASA Includes

// Local includes
#include "distributedimager/IBasicComms.h"
#include "distributedimager/SolverCore.h"
#include "distributedimager/Tracing.h"
#include "distributedimager/MSSplitter.h"
#include "messages/ContinuumWorkUnit.h"
#include "messages/ContinuumWorkRequest.h"

using namespace std;
using namespace askap::cp;
using namespace askap;
using namespace askap::scimath;
using namespace askap::synthesis;
using namespace askap::accessors;

ASKAP_LOGGER(logger, ".ContinuumWorker");

ContinuumWorker::ContinuumWorker(LOFAR::ParameterSet& parset,
                                       askapparallel::AskapParallel& comms)
    : itsParset(parset), itsComms(comms)
{
    itsGridder_p = VisGridderFactory::make(itsParset);
    itsSplitter = new MSSplitter::MSSplitter(itsParset);
    
}

ContinuumWorker::~ContinuumWorker()
{
    itsGridder_p.reset();
    delete itsSplitter;
    
}

void ContinuumWorker::run(void)
{
    // Send the initial request for work
    ContinuumWorkRequest wrequest;
    ASKAPLOG_INFO_STR(logger,"Worker is sending request for work");
    wrequest.sendRequest(itsMaster,itsComms);

    while (1) {

        // Get a workunit
        ASKAPLOG_INFO_STR(logger,"Worker is waiting for a work unit");
        ContinuumWorkUnit wu;
        wu.receiveUnitFrom(itsMaster,itsComms);

        if (wu.get_payloadType() == ContinuumWorkUnit::DONE) {
            // Indicates all workunits have been assigned already
            ASKAPLOG_INFO_STR(logger, "Received DONE signal");
            break;
        }

        const string ms = wu.get_dataset();
        ASKAPLOG_INFO_STR(logger, "Received Work Unit for dataset " << ms
                           << ", local channel " << wu.get_localChannel()
                           << ", global channel " << wu.get_globalChannel()
                           << ", frequency " << wu.get_channelFrequency()/1.e6 << "MHz");
        askap::scimath::Params::ShPtr params;
        try {
            params = processWorkUnit(wu);
        } catch (AskapError& e) {
            ASKAPLOG_WARN_STR(logger, "Failure processing channel " << wu.get_globalChannel());
            ASKAPLOG_WARN_STR(logger, "Exception detail: " << e.what());
        }

        // Send the params to the master, which also implicitly requests
        // more work
        ASKAPLOG_INFO_STR(logger, "Sending params back to master for local channel " << wu.get_localChannel()
                           << ", global channel " << wu.get_globalChannel()
                           << ", frequency " << wu.get_channelFrequency()/1.e6 << "MHz");
        wrequest.set_globalChannel(wu.get_globalChannel());
        wrequest.set_params(params);
        wrequest.sendRequest(itsMaster,itsComms);
        wrequest.set_params(askap::scimath::Params::ShPtr()); // Free memory
    }
}

askap::scimath::Params::ShPtr ContinuumWorker::processWorkUnit(const ContinuumWorkUnit& wu)
{
    
    // place measurement set and parset into /dev/shm for use later
    
    const string colName = itsParset.getString("datacolumn", "DATA");
    const string ms = wu.get_dataset();
    char ChannelPar[64];
    
    sprintf(ChannelPar,"[1,%d]",wu.get_localChannel());
    
    LOFAR::ParameterSet unitParset = itsParset;
    unitParset.replace("Imager.Channels",ChannelPar);
    
    // Now we want to create the measurement set and store it
    // the location will default to /dev/shm/...
    // this will permit subsequent operations
    // We are going to have to use a lot of the measurement set classes.
    // most of this is already in the app mssplit - but I cannot find a
    // clean way to do that.
  
    return askap::scimath::Params::ShPtr() ;
}

void ContinuumWorker::storeMs(LOFAR::ParameterSet& unitParset)
{
}
askap::scimath::Params::ShPtr
ContinuumWorker::processSnapshot(LOFAR::ParameterSet& unitParset)
{
}
askap::scimath::Params::ShPtr
ContinuumWorker::processChannel(LOFAR::ParameterSet& unitParset)
{

    
    std::string majorcycle = unitParset.getString("threshold.majorcycle", "-1Jy");
    const double targetPeakResidual = SynthesisParamsHelper::convertQuantity(majorcycle, "Jy");
    const bool writeAtMajorCycle = unitParset.getBool("Images.writeAtMajorCycle", false);
    const int nCycles = unitParset.getInt32("ncycles", 0);
   
    
    ImagerParallel imager(itsComms, unitParset);
   
    
    if (nCycles == 0) {
   
   
        
        imager.receiveModel();

        imager.calcNE();


        
    } else {
       
        imager.receiveModel();
        
        for (int cycle = 0; cycle < nCycles; ++cycle) {
        
            
            if (imager.params()->has("peak_residual")) {
                const double peak_residual = imager.params()->scalarValue("peak_residual");
                ASKAPLOG_INFO_STR(logger, "Reached peak residual of " << peak_residual);
                if (peak_residual < targetPeakResidual) {
                    ASKAPLOG_INFO_STR(logger, "It is below the major cycle threshold of "
                                      << targetPeakResidual << " Jy. Stopping.");
                    break;
                } else {
                    if (targetPeakResidual < 0) {
                        ASKAPLOG_INFO_STR(logger, "Major cycle flux threshold is not used.");
                    } else {
                        ASKAPLOG_INFO_STR(logger, "It is above the major cycle threshold of "
                                          << targetPeakResidual << " Jy. Continuing.");
                    }
                }
            }
            
            ASKAPLOG_INFO_STR(logger, "*** Starting major cycle " << cycle << " ***");
        
            imager.calcNE();
          
            
            if (cycle + 1 >= nCycles) {
                ASKAPLOG_INFO_STR(logger, "Reached " << nCycles <<
                                  " cycle(s), the maximum number of major cycles. Stopping.");
            }
            
            if (writeAtMajorCycle) {
               
                imager.writeModel(std::string(".majorcycle.") + utility::toString(cycle + 1));
            }
            
            imager.receiveModel();
        }
        ASKAPLOG_INFO_STR(logger, "*** Finished major cycles ***");
        ASKAPLOG_INFO_STR(logger, "Will <NOT> CalcNE");
        imager.calcNE();
        
        
    } // end cycling block

    return imager.params();
    
}

void ContinuumWorker::setupImage(const askap::scimath::Params::ShPtr& params,
                                    double channelFrequency)
{
    try {
        const LOFAR::ParameterSet parset = itsParset.makeSubset("Images.");

        const int nfacets = parset.getInt32("nfacets", 1);
        const string name("image.slice");
        const vector<string> direction = parset.getStringVector("direction");
        const vector<string> cellsize = parset.getStringVector("cellsize");
        const vector<int> shape = parset.getInt32Vector("shape");
        //const vector<double> freq = parset.getDoubleVector("frequency");
        const int nchan = 1;

        if (!parset.isDefined("polarisation")) {
            ASKAPLOG_INFO_STR(logger, "Polarisation frame is not defined, "
                              << "only stokes I will be generated");
        }
        const vector<string> stokesVec = parset.getStringVector("polarisation",
                                         vector<string>(1, "I"));

        // there could be many ways to define stokes, e.g. ["XX YY"] or ["XX","YY"] or "XX,YY"
        // to allow some flexibility we have to concatenate all elements first and then
        // allow the parser from PolConverter to take care of extracting the products.
        string stokesStr;
        for (size_t i = 0; i < stokesVec.size(); ++i) {
            stokesStr += stokesVec[i];
        }
        const casa::Vector<casa::Stokes::StokesTypes>
        stokes = scimath::PolConverter::fromString(stokesStr);

        const bool ewProj = parset.getBool("ewprojection", false);
        if (ewProj) {
            ASKAPLOG_INFO_STR(logger, "Image will have SCP/NCP projection");
        } else {
            ASKAPLOG_INFO_STR(logger, "Image will have plain SIN projection");
        }

        ASKAPCHECK(nfacets > 0,
                   "Number of facets is supposed to be a positive number, you gave " << nfacets);
        ASKAPCHECK(shape.size() >= 2,
                   "Image is supposed to be at least two dimensional. " <<
                   "check shape parameter, you gave " << shape);

        if (nfacets == 1) {
            SynthesisParamsHelper::add(*params, name, direction, cellsize, shape, ewProj,
                                       channelFrequency, channelFrequency, nchan, stokes);
            // SynthesisParamsHelper::add(*params, name, direction, cellsize, shape, ewProj,
            //                            freq[0], freq[1], nchan, stokes);
        } else {
            // this is a multi-facet case
            const int facetstep = parset.getInt32("facetstep", casa::min(shape[0], shape[1]));
            ASKAPCHECK(facetstep > 0,
                       "facetstep parameter is supposed to be positive, you have " << facetstep);
            ASKAPLOG_INFO_STR(logger, "Facet centers will be " << facetstep <<
                              " pixels apart, each facet size will be "
                              << shape[0] << " x " << shape[1]);
            // SynthesisParamsHelper::add(*params, name, direction, cellsize, shape, ewProj,
            //                            freq[0], freq[1], nchan, stokes, nfacets, facetstep);
            SynthesisParamsHelper::add(*params, name, direction, cellsize, shape, ewProj,
                                       channelFrequency, channelFrequency,
                                       nchan, stokes, nfacets, facetstep);
        }

    } catch (const LOFAR::APSException &ex) {
        throw AskapError(ex.what());
    }
}
