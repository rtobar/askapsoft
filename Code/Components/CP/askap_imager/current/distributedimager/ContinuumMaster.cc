/// @file ContinuumMaster.cc
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
#include "ContinuumMaster.h"

// System includes
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askapparallel/AskapParallel.h>

#include <Common/ParameterSet.h>
#include <fitting/Params.h>
#include <fitting/Axes.h>
#include <dataaccess/IConstDataSource.h>
#include <dataaccess/TableConstDataSource.h>
#include <dataaccess/IConstDataIterator.h>
#include <dataaccess/IDataConverter.h>
#include <dataaccess/IDataSelector.h>
#include <dataaccess/IDataIterator.h>
#include <dataaccess/SharedIter.h>
#include <casacore/casa/Quanta.h>
#include <imageaccess/BeamLogger.h>

// Local includes
#include "distributedimager/IBasicComms.h"
#include "messages/ContinuumWorkUnit.h"
#include "messages/ContinuumWorkRequest.h"
#include "Tracing.h"

using namespace std;
using namespace askap::cp;
using namespace askap;

ASKAP_LOGGER(logger, ".ContinuumMaster");

ContinuumMaster::ContinuumMaster(LOFAR::ParameterSet& parset,
                                       askapparallel::AskapParallel& comms)
    : itsParset(parset), itsComms(comms), itsBeamList()
{
}

ContinuumMaster::~ContinuumMaster()
{
}

void ContinuumMaster::run(void)
{
    // Read from the configruation the list of datasets to process
    const vector<string> ms = getDatasets(itsParset);
    if (ms.size() == 0) {
        ASKAPTHROW(std::runtime_error, "No datasets specified in the parameter set file");
    }
   
    // Send work orders to the worker processes, handling out
    // more work to the workers as needed.

    // Global channel is the channel offset across all measurement sets
    // For example, the first MS has 16 channels, then the global channel
    // number for the first (local) channel in the second MS is 16 (zero
    // based indexing).
    unsigned int globalChannel = 0;

    // Tracks all outstanding workunits, that is, those that have not
    // been completed
    unsigned int outstanding = 0;

    // Iterate over all measurement sets
    for (unsigned int n = 0; n < ms.size(); ++n) {
        
        askap::accessors::TableConstDataSource ds(ms[n]);
        askap::accessors::IDataSelectorPtr sel = ds.createSelector();
        askap::accessors::IDataConverterPtr conv = ds.createConverter();
        conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO), "Hz");
        conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
        const askap::accessors::IConstDataSharedIter it = ds.createConstIterator(sel, conv);

        const unsigned int msChannels = it->nChannel();
        ASKAPLOG_INFO_STR(logger, "Creating work orders for measurement set "
                           << ms[n] << " with " << msChannels << " channels");

        // Iterate over all channels in the measurement set
        for (unsigned int localChan = 0; localChan < msChannels; ++localChan) {

            casa::Quantity freq;
            
            freq = casa::Quantity(it->frequency()[localChan],"Hz");
            
            int id; // Id of the process the WorkRequest message is received from

            // Wait for a worker to request some work
            ASKAPLOG_INFO_STR(logger, "Master is waiting for a worker to request some work");
            
            ContinuumWorkRequest wrequest;
            wrequest.receiveRequest(id, itsComms);
            // If the channel number is CHANNEL_UNINITIALISED then this indicates
            // there is no image associated with this message. If the channel
            // number is initialised yet the params pointer is null this indicates
            // that an an attempt was made to process this channel but an exception
            // was thrown.
            if (wrequest.get_globalChannel() != ContinuumWorkRequest::CHANNEL_UNINITIALISED) {
                if (wrequest.get_params().get() != 0) {
                    handleImageParams(wrequest.get_params(), wrequest.get_globalChannel());
                } else {
                    ASKAPLOG_WARN_STR(logger, "Global channel " << wrequest.get_globalChannel()
                                      << " has failed - will be set to zero in the cube.");
                    recordBeamFailure(wrequest.get_globalChannel());
                }
                --outstanding;
            }

            // Send the workunit to the worker
            ASKAPLOG_INFO_STR(logger, "Master is allocating workunit " << ms[n]
                              << ", local channel " <<  localChan << ", global channel "
                              << globalChannel << " to worker " << id);
            ContinuumWorkUnit wu;
            wu.set_payloadType(ContinuumWorkUnit::WORK);
            wu.set_dataset(ms[n]);
            wu.set_globalChannel(globalChannel);
            wu.set_localChannel(localChan);
            wu.set_channelFrequency(freq.getValue("Hz"));
            wu.sendUnit(id,itsComms);
            ++outstanding;

            ++globalChannel;
        }
    }
    ASKAPLOG_INFO_STR(logger, "Master is waiting for outstanding workunits to complete");
    // Wait for all outstanding workunits to complete
    while (outstanding > 0) {
        int id;
        ContinuumWorkRequest wrequest;
        wrequest.receiveRequest(id, itsComms);
        if (wrequest.get_globalChannel() != ContinuumWorkRequest::CHANNEL_UNINITIALISED) {
            if (wrequest.get_params().get() != 0) {
                handleImageParams(wrequest.get_params(), wrequest.get_globalChannel());
            } else {
                ASKAPLOG_WARN_STR(logger, "Global channel " << wrequest.get_globalChannel()
                                  << " has failed - will be set to zero in the cube.");
                recordBeamFailure(wrequest.get_globalChannel());
            }
            --outstanding;
        }
    }

    // Send each worker a response to indicate there are
    // no more work units. This is done separate to the above loop
    // since we need to make sure even workers that never received
    // a workunit are send the "DONE" message.
    for (int id = 1; id < itsComms.nProcs(); ++id) {
        ContinuumWorkUnit wu;
        wu.set_payloadType(ContinuumWorkUnit::DONE);
        wu.sendUnit(id,itsComms);
    }

    logBeamInfo();

}

// Utility function to get dataset names from parset.
std::vector<std::string> ContinuumMaster::getDatasets(const LOFAR::ParameterSet& parset)
{
    if (parset.isDefined("dataset") && parset.isDefined("dataset0")) {
        ASKAPTHROW(std::runtime_error,
                   "Both dataset and dataset0 are specified in the parset");
    }

    // First look for "dataset" and if that does not exist try "dataset0"
    vector<string> ms;
    if (parset.isDefined("dataset")) {
        ms = itsParset.getStringVector("dataset", true);
    } else {
        string key = "dataset0";   // First key to look for
        long idx = 0;
        while (parset.isDefined(key)) {
            const string value = parset.getString(key);
            ms.push_back(value);

            ostringstream ss;
            ss << "dataset" << idx + 1;
            key = ss.str();
            ++idx;
        }
    }

    return ms;
}

void ContinuumMaster::handleImageParams(askap::scimath::Params::ShPtr params,
                                           unsigned int chan)
{
    Tracing::entry(Tracing::WriteImage);
    bool doingPreconditioning=false;
    const vector<string> preconditioners=itsParset.getStringVector("preconditioner.Names",std::vector<std::string>());
    for (vector<string>::const_iterator pc = preconditioners.begin(); pc != preconditioners.end(); ++pc) {
        if( (*pc)=="Wiener" || (*pc)=="NormWiener" || (*pc)=="Robust" || (*pc) == "GaussianTaper") {
            doingPreconditioning=true;
        }
    }

    // Pre-conditions
    ASKAPCHECK(params->has("model.slice"), "Params are missing model parameter");
    ASKAPCHECK(params->has("psf.slice"), "Params are missing psf parameter");
    ASKAPCHECK(params->has("residual.slice"), "Params are missing residual parameter");
    ASKAPCHECK(params->has("weights.slice"), "Params are missing weights parameter");
    if (itsParset.getBool("restore", false) ) {
        ASKAPCHECK(params->has("image.slice"), "Params are missing image parameter");
        if (doingPreconditioning) {
            ASKAPCHECK(params->has("psf.image.slice"), "Params are missing psf.image parameter");
        }
    }

    if (itsParset.getBool("restore", false)) {
        // Record the restoring beam
        const askap::scimath::Axes &axes = params->axes("image.slice");
        recordBeam(axes, chan);
        storeBeam(chan);
    }

    // Write image
    {
        const casa::Array<double> imagePixels(params->value("model.slice"));
        casa::Array<float> floatImagePixels(imagePixels.shape());
        casa::convertArray<float, double>(floatImagePixels, imagePixels);
        itsImageCube->writeSlice(floatImagePixels, chan);
    }

    // Write PSF
    {
        const casa::Array<double> imagePixels(params->value("psf.slice"));
        casa::Array<float> floatImagePixels(imagePixels.shape());
        casa::convertArray<float, double>(floatImagePixels, imagePixels);
        itsPSFCube->writeSlice(floatImagePixels, chan);
    }

    // Write residual
    {
        const casa::Array<double> imagePixels(params->value("residual.slice"));
        casa::Array<float> floatImagePixels(imagePixels.shape());
        casa::convertArray<float, double>(floatImagePixels, imagePixels);
        itsResidualCube->writeSlice(floatImagePixels, chan);
    }

    // Write weights
    {
        const casa::Array<double> imagePixels(params->value("weights.slice"));
        casa::Array<float> floatImagePixels(imagePixels.shape());
        casa::convertArray<float, double>(floatImagePixels, imagePixels);
        itsWeightsCube->writeSlice(floatImagePixels, chan);
    }


    if (itsParset.getBool("restore", false)){

        if(doingPreconditioning) {
            // Write preconditioned PSF image
            {
                const casa::Array<double> imagePixels(params->value("psf.image.slice"));
                casa::Array<float> floatImagePixels(imagePixels.shape());
                casa::convertArray<float, double>(floatImagePixels, imagePixels);
                itsPSFimageCube->writeSlice(floatImagePixels, chan);
            }
        }

        // Write Restored image
        {
            const casa::Array<double> imagePixels(params->value("image.slice"));
            casa::Array<float> floatImagePixels(imagePixels.shape());
            casa::convertArray<float, double>(floatImagePixels, imagePixels);
            itsRestoredCube->writeSlice(floatImagePixels, chan);
        }
    }

    Tracing::exit(Tracing::WriteImage);
}


void ContinuumMaster::recordBeam(const askap::scimath::Axes &axes,
                                    const unsigned int globalChannel)
{

    if (axes.has("MAJMIN")) {
        // this is a restored image with beam parameters set
        ASKAPCHECK(axes.has("PA"), "PA axis should always accompany MAJMIN");
        ASKAPLOG_INFO_STR(logger, "Found beam for image.slice, channel " <<
                          globalChannel << ", with shape " <<
                          axes.start("MAJMIN") * 180. / M_PI * 3600. << "x" <<
                          axes.end("MAJMIN") * 180. / M_PI * 3600. << ", " <<
                          axes.start("PA") * 180. / M_PI);

        casa::Vector<casa::Quantum<double> > beamVec(3, 0.);
        beamVec[0] = casa::Quantum<double>(axes.start("MAJMIN"), "rad");
        beamVec[1] = casa::Quantum<double>(axes.end("MAJMIN"), "rad");
        beamVec[2] = casa::Quantum<double>(axes.start("PA"), "rad");

        itsBeamList[globalChannel] = beamVec;

    }

}

void ContinuumMaster::recordBeamFailure(const unsigned int globalChannel)
{

    casa::Vector<casa::Quantum<double> > beamVec(3, 0.);
    itsBeamList[globalChannel] = beamVec;
    if (globalChannel == itsBeamReferenceChannel) {
        ASKAPLOG_WARN_STR(logger, "Beam reference channel " << itsBeamReferenceChannel
                          << " has failed - output cubes have no restoring beam.");
    }
    
}


void ContinuumMaster::storeBeam(const unsigned int globalChannel)
{
    if (globalChannel == itsBeamReferenceChannel) {
        itsRestoredCube->addBeam(itsBeamList[globalChannel]);
    }
}

void ContinuumMaster::logBeamInfo()
{

    if (itsParset.getBool("restore", false)){
        askap::accessors::BeamLogger beamlog(itsParset.makeSubset("restore."));
        if (beamlog.filename() != "") {
            ASKAPCHECK(itsBeamList.begin()->first == 0, "Beam list doesn't start at channel 0");
            ASKAPCHECK((itsBeamList.size() == (itsBeamList.rbegin()->first + 1)),
                       "Beam list doesn't finish at channel " << itsBeamList.size() - 1);
            std::vector<casa::Vector<casa::Quantum<double> > > beams;
            std::map<unsigned int, casa::Vector<casa::Quantum<double> > >::iterator beam;
            for (beam = itsBeamList.begin(); beam != itsBeamList.end(); beam++) {
                beams.push_back(beam->second);
            }
            beamlog.beamlist() = beams;
            ASKAPLOG_INFO_STR(logger, "Writing list of individual channel beams to beam log "
                              << beamlog.filename());
            beamlog.write();
        }
    }

}
