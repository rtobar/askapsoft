
///local includes
#include <askap_synthesis.h>
#include <utils/LinmosUtils.h>
#include <measurementequation/SynthesisParamsHelper.h>
///ASKAP includes
#include <askap/Application.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/StatReporter.h>
#include <askapparallel/AskapParallel.h>

///CASA includes
#include <casacore/images/Images/ImageInterface.h>
#include <casacore/images/Images/PagedImage.h>
#include <casacore/images/Images/SubImage.h>

///3rd party
#include <Common/ParameterSet.h>


ASKAP_LOGGER(logger, ".linmos");


using namespace askap;
using namespace askap::synthesis;


// @brief do the merge
/// @param[in] parset subset with parameters
static void mergeMPI(const LOFAR::ParameterSet &parset, askap::askapparallel::AskapParallel &comms) {

    ASKAPLOG_INFO_STR(logger, "ASKAP linear (parallel) mosaic task " << ASKAP_PACKAGE_VERSION);
    ASKAPLOG_INFO_STR(logger, "Parset parameters:\n" << parset);

    imagemath::LinmosAccumulator<float> accumulator;

    // load the parset
    if ( !accumulator.loadParset(parset) ) return;

    // initialise an image accessor
    accessors::IImageAccess& iacc = SynthesisParamsHelper::imageHandler();

    // loop over the mosaics, reading each in an adding to the output pixel arrays
    vector<string> inImgNames, inWgtNames, inSenNames;
    string outImgName, outWgtName, outSenName;
    map<string,string> outWgtNames = accumulator.outWgtNames();
    for(map<string,string>::iterator ii=outWgtNames.begin(); ii!=outWgtNames.end(); ++ii) {

        // get output files for this mosaic
        outImgName = (*ii).first;
        outWgtName = accumulator.outWgtNames()[outImgName];
        ASKAPLOG_INFO_STR(logger, "++++++++++++++++++++++++++++++++++++++++++");
        ASKAPLOG_INFO_STR(logger, "Preparing mosaic " << outImgName);
        if (!accumulator.outWgtDuplicates()[outImgName]) {
            ASKAPLOG_INFO_STR(logger, " - also weights image " << outWgtName);
        }
        accumulator.doSensitivity(false);
        if (accumulator.genSensitivityImage()[outImgName]) {
            outSenName = accumulator.outSenNames()[outImgName];
            accumulator.doSensitivity(true);
            ASKAPLOG_INFO_STR(logger, " - also sensitivity image " << outSenName);
        }

        // get input files for this mosaic
        inImgNames = accumulator.inImgNameVecs()[outImgName];
        ASKAPLOG_INFO_STR(logger, " - input images: "<<inImgNames);
        if (accumulator.weightType() == FROM_WEIGHT_IMAGES) {
            inWgtNames = accumulator.inWgtNameVecs()[outImgName];
            ASKAPLOG_INFO_STR(logger, " - input weights images: " << inWgtNames);
        }
        else if (accumulator.weightType() == FROM_BP_MODEL) {
            accumulator.beamCentres(loadBeamCentres(parset,iacc,inImgNames));
        }
        if (accumulator.doSensitivity()) {
            inSenNames = accumulator.inSenNameVecs()[outImgName];
            ASKAPLOG_INFO_STR(logger, " - input sensitivity images: " << inSenNames);
        }

    }

    // set the output coordinate system and shape, based on the overlap of input images
    vector<IPosition> inShapeVec;
    vector<CoordinateSystem> inCoordSysVec;
    for (vector<string>::iterator it = inImgNames.begin(); it != inImgNames.end(); ++it) {
        casa::PagedImage<casa::Float> img(*it);
        ASKAPCHECK(img.ok(),"Error loading "<< *it);
        ASKAPCHECK(img.shape().nelements()>=3,"Work with at least 3D cubes!");
        const casa::IPosition shape = img.shape();
        ASKAPLOG_INFO_STR(logger," - Shape " << shape);
        casa::IPosition blc(shape.nelements(),0);
        casa::IPosition trc(shape);

        blc[3] = comms.rank();
        trc[3] = 1; // this could be nchan/nWorkers ...

        ASKAPCHECK(blc[3]>=0 && blc[3]<shape[3], "Start channel is outside the number of channels or negative, shape: "<<shape);
        ASKAPCHECK(trc[3]<=shape[3], "Subcube extends beyond the original cube, shape:"<<shape);


        ASKAPLOG_INFO_STR(logger, " - Corners " << "blc  = " << blc << ", trc = " << trc << "\n");


        casa::Slicer slc(blc,trc,casa::Slicer::endIsLength);
        ASKAPLOG_INFO_STR(logger, " - Slicer " << slc);

        casa::SubImage<casa::Float> si = casa::SubImage<casa::Float>(img,slc,casa::AxesSpecifier(casa::True));

        // get the shape of a single channel slice based upon rank
        inShapeVec.push_back(si.shape());
        inCoordSysVec.push_back(si.coordinates());

    }
    accumulator.setOutputParameters(inShapeVec, inCoordSysVec);

    // set up the output pixel arrays
    Array<float> outPix(accumulator.outShape(),0.);
    Array<float> outWgtPix(accumulator.outShape(),0.);
    Array<float> outSenPix;
    if (accumulator.doSensitivity()) {
        outSenPix = Array<float>(accumulator.outShape(),0.);
    }

    // set up an indexing vector for the arrays
    IPosition curpos(outPix.shape());
    ASKAPASSERT(curpos.nelements()>=2);
    for (uInt dim=0; dim<curpos.nelements(); ++dim) {
        curpos[dim] = 0;
    }

    // loop over the input images, reading each in an adding to the output pixel arrays
    for (uInt img = 0; img < inImgNames.size(); ++img ) {

        // short cuts
        string inImgName = inImgNames[img];
        string inWgtName, inSenName;

        ASKAPLOG_INFO_STR(logger, "Processing input image " << inImgName);
        if (accumulator.weightType() == FROM_WEIGHT_IMAGES) {
            inWgtName = inWgtNames[img];
            ASKAPLOG_INFO_STR(logger, " - and input weight image " << inWgtName);
        }
        if (accumulator.doSensitivity()) {
            inSenName = inSenNames[img];
            ASKAPLOG_INFO_STR(logger, " - and input sensitivity image " << inSenName);
        }

        casa::PagedImage<casa::Float> inImg(inImgName);
        const casa::IPosition shape = inImg.shape();
        casa::IPosition blc(shape.nelements(),0);
        casa::IPosition trc(shape);

        blc[3] = comms.rank();
        trc[3] = 1; // this could be nchan/nWorkers ...

        casa::Slicer slc(blc,trc,casa::Slicer::endIsLength);

        Array<float> inPix = inImg.getSlice(slc);

        Array<float> inWgtPix;
        Array<float> inSenPix;

        if (accumulator.weightType() == FROM_WEIGHT_IMAGES) {

            casa::PagedImage<casa::Float> inImg(inWgtName);
            const casa::IPosition shape = inImg.shape();
            casa::IPosition blc(shape.nelements(),0);
            casa::IPosition trc(shape);

            blc[3] = comms.rank();
            trc[3] = 1; // this could be nchan/nWorkers ...

            casa::Slicer slc(blc,trc,casa::Slicer::endIsLength);

            inWgtPix = inImg.getSlice(slc);

            ASKAPASSERT(inPix.shape() == inWgtPix.shape());
        }
        if (accumulator.doSensitivity()) {

            casa::PagedImage<casa::Float> inImg(inSenName);
            const casa::IPosition shape = inImg.shape();
            casa::IPosition blc(shape.nelements(),0);
            casa::IPosition trc(shape);

            blc[3] = comms.rank();
            trc[3] = 1; // this could be nchan/nWorkers ...

            casa::Slicer slc(blc,trc,casa::Slicer::endIsLength);

            inSenPix = inImg.getSlice(slc);

            ASKAPASSERT(inPix.shape() == inSenPix.shape());
        }
    }
    // set up an iterator for all directionCoordinate planes in the input image
    scimath::MultiDimArrayPlaneIter planeIter(accumulator.inShape());

    // test whether to simply add weighted pixels, or whether a regrid is required
    bool regridRequired = !accumulator.coordinatesAreEqual();

    // if regridding is required, set up buffer some images
    if ( regridRequired ) {

        ASKAPLOG_INFO_STR(logger, " - regridding -- input pixel grid is different from the output");
        // currently all output planes have full-size, so only initialise once
        // would be faster if this was reduced to the size of the current input image
        if ( accumulator.outputBufferSetupRequired() ) {
            ASKAPLOG_INFO_STR(logger, " - initialising output buffers and the regridder");
            // set up temp images required for regridding
            //accumulator.initialiseOutputBuffers();
            // set up regridder
            accumulator.initialiseRegridder();
        }

        // set up temp images required for regridding
        // need to do this here if some do and some do not have sensitivity images
        accumulator.initialiseOutputBuffers();

        // set up temp images required for regridding
        // are those of the previous iteration correctly freed?
        accumulator.initialiseInputBuffers();

        } else {
        ASKAPLOG_INFO_STR(logger, " - not regridding -- input pixel grid is the same as the output");
        }
    }
}
class linmosMPIApp : public askap::Application
{
    public:

        virtual int run(int argc, char* argv[]) {

            // This class must have scope outside the main try/catch block
            askap::askapparallel::AskapParallel comms(argc, const_cast<const char**>(argv));

            try {
                StatReporter stats;
                LOFAR::ParameterSet subset(config().makeSubset("linmos."));
                SynthesisParamsHelper::setUpImageHandler(subset);
                mergeMPI(subset, comms);
                stats.logSummary();
                return 0;
            }
            catch (const askap::AskapError& e) {
                ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << e.what());
                std::cerr << "Askap error in " << argv[0] << ": " << e.what() << std::endl;
                return 1;
            } catch (const std::exception& e) {
                ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << e.what());
                std::cerr << "Unexpected exception in " << argv[0] << ": " << e.what()
                    << std::endl;
                return 1;
            }


    };

};

int main(int argc, char *argv[])
{
    linmosMPIApp app;
    return app.main(argc, argv);
}
