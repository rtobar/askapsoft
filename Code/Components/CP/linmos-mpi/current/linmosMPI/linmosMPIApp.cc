///local includes
#include<linmosMPIApp.h>
// other 3rd party
#include <Common/ParameterSet.h>

int linmosMPIApp::run(int argc, char* argv[])
{
    StatReporter stats;
    LOFAR::ParameterSet subset(config().makeSubset("linmos."));
    stats.logSummary();
    return 0;
}
