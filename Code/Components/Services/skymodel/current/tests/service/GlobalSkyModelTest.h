/// @file GlobalSkyModelTest.cc
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
#include <boost/filesystem.hpp>
#include <votable/VOTable.h>

// Classes to test
#include "service/GlobalSkyModel.h"

using namespace std;
using namespace askap::accessors;

namespace askap {
namespace cp {
namespace sms {

class GlobalSkyModelTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(GlobalSkyModelTest);
        CPPUNIT_TEST(testSmallVoTableMetadata);
        CPPUNIT_TEST_SUITE_END();

    public:
        GlobalSkyModelTest() :
            small_no_pol("./tests/data/votable_small_no_polarisation.xml"),
            large_no_pol("./tests/data/votable_large_no_polarisation.xml")
        {
        }

        void setUp() {
        }

        void tearDown() {
        }

        void testSmallVoTableMetadata() {
            CPPUNIT_ASSERT(boost::filesystem::exists(small_no_pol));

            VOTable vt = VOTable::fromXML(small_no_pol);
            cout << "\nSmall VOTable Metadata\n" <<
                "Desc: " << vt.getDescription() << endl <<
                vt.getResource().size() << " resources\n" <<
                vt.getInfo().size() << " info entries\n";

            const VOTableResource& r = vt.getResource().front();
            cout <<   "Resource 0:\n" <<
                //"ID: " << r.getID() << endl <<
                "Name: " << r.getName() << endl <<
                "Type: " << r.getType() << endl <<
                "Num Info blocks: " << r.getInfo().size() << endl <<
                //"Num Tables: " << r.getTables().size() << endl <<
                endl;

            const VOTableTable& t = r.getTables().front();
            cout << "Table: " << &t;

            //const VOTableTable& t = r.getTables()[0];
            //cout <<   "Table 0:\n" <<
                //"ID: " << t.getID() << endl <<
                //"Name: " << t.getName() << endl <<
                //"Desc: " << t.getDescription() << endl <<
                //"Num groups: " << t.getGroups().size() << endl <<
                //"Num fields: " << t.getFields().size() << endl <<
                //"Num rows: " << t.getRows().size() << endl;

            //if (r.getInfo().size() > 0) {
                //for (vector<VOTableInfo>::iterator it = r.getInfo().begin(); it != r.getInfo().end(); ++it) {
                    //cout << "\t" << it->getID() << " : " << it->getName() << " : " <<
                        //it->getValue() << " : " << it->getText(); }
            //} else {
                //cout << "No info\n";
            //}

            //if (r.getTables().size() > 0) {
                //for (vector<VOTableTable>::iterator it = r.getTables().begin(); it != r.getTables().end(); ++it) {
                    //cout << "\t" << it->getID() << " : " << it->getName() << " : " <<
                        //it->getValue() << " : " << it->getText(); }
            //} else {
                //cout << "No tables\n";
            //}
        }

    private:

        const string small_no_pol;
        const string large_no_pol;
};

}
}
}
