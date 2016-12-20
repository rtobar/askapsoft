/// @file HealpixTest.cc
///
/// @copyright (c) 2016 CSIRO
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
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/// See the GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Daniel Collins <daniel.collins@csiro.au>

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// Support classes
#include <string>
#include <boost/math/constants/constants.hpp>

// Classes to test
#include "service/HealPixFacade.h"

using std::string;
using std::vector;

namespace askap {
namespace cp {
namespace sms {

class HealpixTest : public CppUnit::TestFixture {

        CPPUNIT_TEST_SUITE(HealpixTest);
        CPPUNIT_TEST(testCalcHealpixIndex);
        CPPUNIT_TEST(testQueryDisk);
        CPPUNIT_TEST(testQueryRect);
        CPPUNIT_TEST(testJ2000ToPointing_valid_values);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }

        void tearDown() {
        }

        void testCalcHealpixIndex() {
            HealPixFacade hp(5);
            HealPixFacade::Index actual = hp.calcHealPixIndex(14.8, 43.1);
            HealPixFacade::Index expected = 2663;
            CPPUNIT_ASSERT_EQUAL(expected, actual);
        }

        void testQueryDisk() {
            HealPixFacade hp(10);
            HealPixFacade::IndexListPtr actual = hp.queryDisk(71.8, -63.1, 1.0/60.0, 8);
            CPPUNIT_ASSERT_EQUAL(size_t(4), actual->size());
            CPPUNIT_ASSERT_EQUAL(33942670l, (*actual)[0]);
            CPPUNIT_ASSERT_EQUAL(33942671l, (*actual)[1]);
            CPPUNIT_ASSERT_EQUAL(33942692l, (*actual)[2]);
            CPPUNIT_ASSERT_EQUAL(33942693l, (*actual)[3]);
        }

        void testQueryRect() {
            HealPixFacade hp(10);
            // create a rect of ~30 sq degrees (5 * 6)
            HealPixFacade::IndexListPtr actual = hp.queryRect(
                70.9, -63.1,  // top-left
                75.9, -69.1,  // bottom-right
                8);           // oversampling factor
                
            CPPUNIT_ASSERT_EQUAL(size_t(4), actual->size());
            //CPPUNIT_ASSERT_EQUAL(33942670l, (*actual)[0]);
            //CPPUNIT_ASSERT_EQUAL(33942671l, (*actual)[1]);
            //CPPUNIT_ASSERT_EQUAL(33942692l, (*actual)[2]);
            //CPPUNIT_ASSERT_EQUAL(33942693l, (*actual)[3]);
        }

        void testQueryRect_range_error_swapped_dec() {
            HealPixFacade hp(10);
            CPPUNIT_ASSERT_THROW(
                hp.queryRect(
                    70.9, -69.1,  // top-left
                    75.9, -63.1,  // bottom-right
                    8),           // oversampling factor
                askap::AskapError);
        }

        void testQueryRect_range_error_swapped_ra() {
            HealPixFacade hp(10);
            CPPUNIT_ASSERT_THROW(
                hp.queryRect(
                    75.9, -63.1,  // top-left
                    70.9, -69.1,  // bottom-right
                    8),           // oversampling factor
                askap::AskapError);
        }

        void testJ2000ToPointing_valid_values() {
            double ra = 10.0;
            double dec = 89.0;
            const double pi_180 = boost::math::double_constants::pi / 180.0;
            pointing expected((90.0 - dec) * pi_180, ra * pi_180);
            pointing actual = HealPixFacade::J2000ToPointing(ra, dec);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(expected.theta, actual.theta, 0.000001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(expected.phi, actual.phi, 0.000001);
        }

    private:

};

}
}
}
