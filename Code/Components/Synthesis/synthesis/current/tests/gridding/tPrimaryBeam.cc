
// ASKAPsoft includes
#include <AskapTestRunner.h>

// just to avoid template compilation which will not work without logging


// Test includes

#include <PrimaryBeamTest.h>


int main(int argc, char *argv[])
{
    askapdev::testutils::AskapTestRunner runner(argv[0]);

    runner.addTest( askap::synthesis::PrimaryBeamTest::suite());


    bool wasSucessful = runner.run();

    return wasSucessful ? 0 : 1;
}
