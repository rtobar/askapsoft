#include <PrimaryBeam.h>
#include <PrimaryBeamFactory.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>
#include <boost/shared_ptr.hpp>



namespace askap
{
  namespace synthesis
  {

    class PrimaryBeamTest : public CppUnit::TestFixture
    {

      CPPUNIT_TEST_SUITE(PrimaryBeamTest);

      CPPUNIT_TEST(testCreate);
      CPPUNIT_TEST_EXCEPTION(testCreateAbstract,AskapError);
      CPPUNIT_TEST_SUITE_END();

  private:


  public:


      void testCreate()
      {

         LOFAR::ParameterSet parset;
         PrimaryBeam::ShPtr PB = PrimaryBeamFactory::make(parset);
      }
      void testCreateAbstract()
      {
          // PrimaryBeam is still an abstract class
          // calling createPriamryBeam static method should raise an
          // exception
          LOFAR::ParameterSet parset;
          PrimaryBeam::ShPtr PB = PrimaryBeam::createPrimaryBeam(parset);

      }




  }; // class
  } // synthesis
} //askap
