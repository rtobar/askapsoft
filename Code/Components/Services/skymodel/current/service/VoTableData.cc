/// @file VOTableData.cc
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

// Include own header file first
#include "VOTableData.h"

// Include package level header file
#include "askap_skymodel.h"

// System includes
#include <string>
#include <boost/filesystem.hpp>

// ASKAPsoft includes
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>

// Local package includes
#include "Utility.h"

ASKAP_LOGGER(logger, ".VOTableData");

using namespace std;
using namespace askap::cp::sms;
using namespace askap::accessors;


VOTableData* VOTableData::create(
    string components_file, string polarisation_file)
{
    // open components file
    VOTable components = VOTable::fromXML(components_file);
    ASKAPASSERT(components.getResource().size() == 1ul);
    ASKAPASSERT(components.getResource()[0].getTables().size() == 1ul);
    const VOTableTable components_table = components.getResource()[0].getTables()[0];
    vector<VOTableField> fields = components_table.getFields();
    vector<VOTableRow> rows = components_table.getRows();
    const long num_components = rows.size();

    VOTableData* pData = new VOTableData(num_components);
    ASKAPASSERT(pData);

    // TODO: will it be better to reverse the order of field and row iteration?
    // Typically there will be ~30 fields and ~1000 rows
    for (vector<VOTableRow>::iterator rit = rows.begin(); rit != rows.end(); rit++) {
        for (vector<VOTableField>::iterator fit = fields.begin(); fit != fields.end(); fit++) {
            const string ucd = fit->getUCD();
            const string type = fit->getDatatype();
            const string unit = fit->getUnit();

            //pData->add_row(*rit);
        }
    }

    // open polarisation file if it exists
    if (boost::filesystem::exists(polarisation_file))
    {
        // parse polarisation file
    }

    // Parse fields:
    //  ucd to field/cell index
    // Parse rows:
    //  coerce type
    //  get ucd from cell index
    //  add data
    //      units check


    return pData;
}

VOTableData::VOTableData(long num_components)
{
}

VOTableData::~VOTableData()
{
    ASKAPLOG_DEBUG_STR(logger, "dtor");
}

bool VOTableData::add_row(const VOTableRow& row) {
    return true;
}

