#include <gridding/PrimaryBeam.h>
#include <gridding/PrimaryBeamFactory.h>
#include <askap/AskapError.h>

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

      CPPUNIT_TEST(testCreateGaussian);
      CPPUNIT_TEST_EXCEPTION(testCreateAbstract,AskapError);
      CPPUNIT_TEST_SUITE_END();

  private:


  public:


      void testCreateGaussian()
      {

         LOFAR::ParameterSet parset;
         parset.add("primarybeam","GaussianPB");
         PrimaryBeam::ShPtr PB = PrimaryBeamFactory::make(parset);
      }
      void testCreateAbstract()
      {
          // PrimaryBeam is still an abstract class
          // calling createPriamryBeam static method should raise an
          // exception
          LOFAR::ParameterSet parset;
          PrimaryBeam::createPrimaryBeam(parset);

      }




  }; // class
  } // synthesis
} //askap
