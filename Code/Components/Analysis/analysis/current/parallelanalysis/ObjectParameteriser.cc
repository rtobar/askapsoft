/// @file
///
/// Handle the parameterisation of objects that require reading from a file on disk
///
/// @copyright (c) 2014 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <parallelanalysis/ObjectParameteriser.h>

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askapparallel/AskapParallel.h>

#include <parallelanalysis/DuchampParallel.h>

#include <casainterface/CasaInterface.h>

#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
using namespace LOFAR::TYPES;

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".objectparam");

namespace askap {
namespace analysis {

ObjectParameteriser::ObjectParameteriser(askap::askapparallel::AskapParallel& comms):
    DistributedParameteriserBase(comms)
{
}

ObjectParameteriser::~ObjectParameteriser()
{
}


void ObjectParameteriser::parameterise()
{
    if (itsComms->isWorker()) {
        // For each object, get the bounding subsection for that object
        // Define a DuchampParallel and use it to do the parameterisation
        // put parameterised objects into itsOutputList

        if (itsInputList.size() > 0) {

            std::string image = itsReferenceParams.getImageFile();
            std::vector<size_t> dim = analysisutilities::getCASAdimensions(image);
            itsReferenceParset.replace("flagsubsection", "true");

            for (size_t i = 0; i < itsInputList.size(); i++) {

                ASKAPLOG_DEBUG_STR(logger, "Parameterising object #" << i + 1 <<
                                   " out of " << itsInputList.size());

                // get bounding subsection & transform into a Subsection string
                itsInputList[i].setHeader(itsHeader);

                // add the offsets, so that we are in global-pixel-coordinates
                itsInputList[i].addOffsets();
                std::string subsection = itsInputList[i].boundingSubsection(dim, true);

                itsReferenceParset.replace("subsection", subsection);
                // turn off the subimaging, so we read the whole lot.
                itsReferenceParset.replace("nsubx", "1");
                itsReferenceParset.replace("nsuby", "1");
                itsReferenceParset.replace("nsubz", "1");

                // define a duchamp Cube using the filename from the
                // itsReferenceParams

                // set the subsection
                DuchampParallel tempDP(*itsComms, itsReferenceParset);
                // set this to false to stop anything trying to access
                // the recon array
                tempDP.cube().setReconFlag(false);

                // open the image
                tempDP.readData();

                // set the offsets to those from the local subsection
                itsInputList[i].setOffsets(tempDP.cube().pars());
                // remove those offsets, so we are in
                // local-pixel-coordinates (as if we just did the
                // searching)
                itsInputList[i].removeOffsets();
                itsInputList[i].setFlagText("");

                // store the current object to the cube
                tempDP.cube().addObject(itsInputList[i]);

                // parameterise
                tempDP.cube().calcObjectWCSparams();

                sourcefitting::RadioSource src(tempDP.cube().getObject(0));

                if (tempDP.fitParams().doFit()) {

                    src.setFitParams(tempDP.fitParams());
                    src.defineBox(tempDP.cube().pars().section(),
                                  tempDP.cube().header().getWCS()->spec);
                    src.setDetectionThreshold(tempDP.cube(),
                                              tempDP.getFlagVariableThreshold());

                    src.prepareForFit(tempDP.cube(), true);
                    src.setAtEdge(false);

                    tempDP.fitSource(src);

                }

                // put back onto the global grid
                src.addOffsets();

                // set the offsets to those from the base subsection
                src.setOffsets(itsReferenceParams);
                // and remove them, so that we're in subsection coordinates
                src.removeOffsets();

                // get the parameterised object and store to itsOutputList
                itsOutputList.push_back(src);
            }

            ASKAPASSERT(itsOutputList.size() == itsInputList.size());

        }

    }

}

void ObjectParameteriser::gather()
{
    if (itsComms->isParallel()) {

        if (itsTotalListSize > 0) {

            if (itsComms->isMaster()) {
                // for each worker, read completed objects until we get a 'finished' signal

                // now read back the sources from the workers
                LOFAR::BlobString bs;
                for (int n = 0; n < itsComms->nProcs() - 1; n++) {
                    int numSrc;
                    ASKAPLOG_INFO_STR(logger, "Master about to read from worker #" << n + 1);
                    itsComms->receiveBlob(bs, n + 1);
                    LOFAR::BlobIBufString bib(bs);
                    LOFAR::BlobIStream in(bib);
                    int version = in.getStart("OPfinal");
                    ASKAPASSERT(version == 1);
                    in >> numSrc;
                    ASKAPLOG_DEBUG_STR(logger, "Reading " << numSrc <<
                                       " objects from worker #" << n + 1);
                    for (int i = 0; i < numSrc; i++) {
                        sourcefitting::RadioSource src;
                        in >> src;
                        ASKAPLOG_DEBUG_STR(logger, "Read parameterised object " <<
                                           src.getName() << ", ID=" << src.getID());
                        // make sure we have the right WCS etc information
                        src.setHeader(itsHeader);
                        src.setOffsets(itsReferenceParams);
                        itsOutputList.push_back(src);
                    }
                    in.getEnd();
                }

                ASKAPASSERT(itsOutputList.size() == itsInputList.size());

            } else { // WORKER
                // for each object in itsOutputList, send to master
                ASKAPLOG_INFO_STR(logger, "Have parameterised " << itsInputList.size() <<
                                  " edge sources. Returning results to master.");
                LOFAR::BlobString bs;
                bs.resize(0);
                LOFAR::BlobOBufString bob(bs);
                LOFAR::BlobOStream out(bob);
                out.putStart("OPfinal", 1);
                out << int(itsOutputList.size());
                for (size_t i = 0; i < itsOutputList.size(); i++) {
                    out << itsOutputList[i];
                }
                out.putEnd();
                itsComms->sendBlob(bs, 0);
            }

        } else {
            // serial case - need to put output sources into the DP edgelist
            for (size_t i = 0; i < itsOutputList.size(); i++) {
                // make sure we have the right WCS etc information
                itsOutputList[i].setHeader(itsHeader);
            }

        }

    }

}

}


}
