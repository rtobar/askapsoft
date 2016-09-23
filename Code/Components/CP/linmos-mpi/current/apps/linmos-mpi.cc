
#include<linmosMPI/linmosMPIApp.h>

ASKAP_LOGGER(logger, ".linmos");

using namespace casa;
using namespace askap;


int main(int argc, char *argv[])
{
    LinmosMPIApp app;
    return app.main(argc, argv);
}
