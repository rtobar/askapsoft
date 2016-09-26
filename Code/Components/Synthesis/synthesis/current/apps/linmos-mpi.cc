
///local includes
#include <askap_synthesis.h>

#include <measurementequation/SynthesisParamsHelper.h>
///ASKAP includes
#include <askap/Application.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <askap/StatReporter.h>
///CASA includes
#include <Common/ParameterSet.h>


ASKAP_LOGGER(logger, ".linmos");


using namespace askap;
using namespace askap::synthesis;


// @brief do the merge
/// @param[in] parset subset with parameters
static void mergeMPI(const LOFAR::ParameterSet &parset) {

    ASKAPLOG_INFO_STR(logger, "ASKAP linear (parallel) mosaic task " << ASKAP_PACKAGE_VERSION);
    ASKAPLOG_INFO_STR(logger, "Parset parameters:\n" << parset);

}
class linmosMPIApp : public askap::Application
{
    public:

        virtual int run(int argc, char* argv[]) {

            try {
                StatReporter stats;
                LOFAR::ParameterSet subset(config().makeSubset("linmos."));
                SynthesisParamsHelper::setUpImageHandler(subset);
                mergeMPI(subset);
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
