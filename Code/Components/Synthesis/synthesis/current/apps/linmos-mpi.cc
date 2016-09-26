
///local includes
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

class linmosMPIApp : public askap::Application
{
    public:

        virtual int run(int argc, char* argv[]) {

            try {
                StatReporter stats;
                LOFAR::ParameterSet subset(config().makeSubset("linmos."));
                SynthesisParamsHelper::setUpImageHandler(subset);
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
