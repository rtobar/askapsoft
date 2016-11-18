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
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
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

// Classes to test
#include "service/Spherical.h"

using std::string;
using std::vector;

namespace askap {
namespace cp {
namespace sms {

class HealpixTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(HealpixTest);
        CPPUNIT_TEST(testCalcHealpixIndex);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }

        void tearDown() {
        }

        void testCalcHealpixIndex() {
            //double ra = 14.8;
            //double dec = 43.1;
            //long order = 5;

            //Spherical s(order);
            //long actual = s.calcHealPixIndex(ra, dec);
            //long expected = 43;  // TODO determine real expected value

            //CPPUNIT_ASSERT_EQUAL(expected, actual);
        }

    private:

};

}
}
}
