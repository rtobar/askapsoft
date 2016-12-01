# coding: utf-8

import pandas as pd
from string import Template

# This script generates ODB schema code for direct inclusion into the ASKAP Sky Model Service code.

# Define the Outputs
# Each schema file defines a dictionary with the following keys:
# * **input**: the input spreadsheet
# * **output**: the output file
# * **parse_cols**: zero-based column indicies for parsing from the spreadsheet
# * **skiprows**: zero-based row indicies that should be skipped in the spreadsheet

CONTINUUM_COMPONENT_SPEC = './GSM_casda.continuum_component_description.xlsx'

files = [
    {
        'input': CONTINUUM_COMPONENT_SPEC,
        'output': '../schema/ContinuumComponent.i',
        'parse_cols': None,
        'skiprows': [0],
    },
    {
        'input': './GSM_casda_polarisation.xlsx',
        'output': '../schema/Polarisation.i',
        'parse_cols': None,
        'skiprows': [0],
    },
    {
        'input': './GSM_data_source_description.xlsx',
        'output': '../schema/DataSource.i',
        'parse_cols': None,
        'skiprows': [0],
    },
]

view_files = [
    {
        'inputs': [
            CONTINUUM_COMPONENT_SPEC,
            './GSM_casda_polarisation.xlsx',
            ],
        'output': '../schema/ContinuumComponentLsmView.i',
        'parse_cols': None,
        'skiprows': [0],
    },
]

def load(
    filename,
    sheetname='Catalogue description',
    parse_cols=None,
    skiprows=None,
    converters={
            'name': str,
            'description': str,
            'datatype': str,
            'ucd': str,
            'units': str,
            'notes': str,
            'include_in_gsm': bool,
            'index': bool,
            'nullable': bool,
            'lsm_view': bool,
            }):
    """Load the table data from a CASDA definition spreadsheet"""
    data = pd.read_excel(
        filename,
        sheetname=sheetname,
        converters=converters,
        parse_cols=parse_cols,
        skiprows=skiprows)

    # Drop any rows with missing data in the name or datatype columns
    data.dropna(subset=['name', 'datatype'], inplace=True)

    # fill any remaining missing data with empty strings
    data.fillna('', inplace=True)

    return data[data.include_in_gsm == True]

type_map = {
    'BIGINT': 'boost::int64_t',
    'BIGINT UNSIGNED': 'boost::uint64_t',
    'REAL': 'float',
    'DOUBLE': 'double',
    'VARCHAR': 'std::string',
    'TEXT': 'std::string',
    'BOOLEAN': 'bool',
    'INTEGER': 'boost::int32_t',
    'INTEGER UNSIGNED': 'boost::uint32_t',
    'DATETIME': 'boost::posix_time::ptime',
}

class Field(object):
    "Represents a database field"
    def __init__(self, df_row, is_view):
        self.name = df_row.name.strip()
        self.comment = df_row.description.strip()
        self.raw_type = df_row.datatype.strip()
        self.dtype = type_map[self.raw_type]
        self.units = df_row.units.strip()
        self.indexed = df_row.index
        self.nullable = df_row.nullable
        self.is_view = is_view
        self._magic_names = {}

        # not every tablespec contains the ucd column
        try:
            self.ucd = df_row.ucd.strip()
        except:
            self.ucd = ''

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        _str = self._comment()

        if not self.is_view:
            # handle any pragmas for magic field names
            if self.name in self._magic_names:
                for pragma in self._magic_names[self.name]:
                    _str.append(pragma)

            if self.indexed:
                _str.append('#pragma db index')

            if self.nullable:
                _str.append('#pragma db null')
            else:
                _str.append('#pragma db not_null')

        # the field definition
        _str.append('{0} {1};'.format(
            self.dtype,
            self.name))

        # indentation
        # indent = 4
        # _str = [indent * ' ' + line for line in _str]

        _str.append('\n')

        return '\n'.join(_str)

    def _comment(self):
        "Builds the comment text"
        comments = []
        if self.units:
            comments.append('@brief {0} ({1})'.format(self.comment, self.units))
        else:
            comments.append('@brief {0}'.format(self.comment))
        comments.append('UCD: {0}'.format(self.ucd))
        return ['/// ' + c for c in comments]


def get_fields(data_frame, is_view=False):
    "Unpacks a tablespec data frame into a list of field objects"
    fields = []
    for row in data_frame.itertuples(index=False):
        # print(row)
        field = Field(row, is_view)
        fields.append(field)

    return fields


def write_output(data_frame, filename, is_view=False):
    "Writes the data model fields to an include file"
    with open(filename, 'w') as out:
        for field in get_fields(data_frame, is_view):
            out.write(str(field))

#--------------------------------------------------
# VOTable Parser function code generation section
#--------------------------------------------------
HEADER_PREAMBLE = \
'''\
/// ----------------------------------------------------------------------------
/// This file is generated by schema_definitions/generate.py.
/// Do not edit directly or your changes will be lost!
/// ----------------------------------------------------------------------------
///
/// @file VOTableParser.h
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

#ifndef ASKAP_CP_SMS_VOTABLEPARSER_H
#define ASKAP_CP_SMS_VOTABLEPARSER_H

// System includes
#include <string>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>

// ASKAPsoft includes
#include <votable/VOTable.h>

// Local package includes
#include "datamodel/ContinuumComponent.h"

namespace askap {
namespace cp {
namespace sms {

'''

HEADER_POSTAMBLE = '''

}
}
}

#endif
'''

PARSE_COMPONENT_ROW_FIELD_START = '''
void parseComponentRowField(
    size_t row_index,
    const std::string& ucd,
    const std::string& name,
    const std::string& type,
    const std::string& unit,
    const std::string& value,
    std::vector<datamodel::ContinuumComponent>& components,
    std::vector<double>& ra_buffer,
    std::vector<double>& dec_buffer) {

    ASKAPASSERT(row_index >= 0);
    ASKAPASSERT(row_index < components.size());
    ASKAPASSERT(row_index < ra_buffer.size());
    ASKAPASSERT(row_index < dec_buffer.size());
'''

UCD_FIELD_PARSE_PATTERN = '''
    if (boost::iequals(ucd, "$ucd")) {
        ASKAPASSERT($unitCheckExpression);
        ASKAPASSERT(boost::iequals(type, "$votype"));
        components[row_index].$fieldname = boost::lexical_cast<$cpptype>(value);
'''

NO_UCD_FIELD_PARSE_PATTERN = '''
    if (boost::iequals(name, "$fieldname")) {
        // Some fields do not have a unique UCD. They are matched by name.
        ASKAPASSERT($unitCheckExpression);
        ASKAPASSERT(boost::iequals(type, "$votype"));
        components[row_index].$fieldname = boost::lexical_cast<$cpptype>(value);
    }'''

def write_votable_parser():
    "Writes the VOTable to data model class parsing code"
    print('\t../service/VOTableParser.h')
    with open('../service/VOTableParser.h', 'w') as out:
        out.write(HEADER_PREAMBLE)

        # map the types in the spec spreadsheet to the strings expected in the
        # VOTable XML file
        db_to_votable_type_map = {
            'BIGINT': 'int',
            'BIGINT UNSIGNED': 'int',
            'REAL': 'float',
            'DOUBLE': 'double',
            'VARCHAR': 'char',
            'TEXT': 'char',
            'BOOLEAN': 'int',
            'INTEGER': 'int',
            'INTEGER UNSIGNED': 'int',
            'DATETIME': 'char',
        }

        ucd_skip_list = ['', 'meta.code']

        out.write(PARSE_COMPONENT_ROW_FIELD_START)
        fields = get_fields(load(CONTINUUM_COMPONENT_SPEC, skiprows=[0]))
        count = 0
        for field in fields:
            if field.units:
                unitCheck = 'boost::iequals(unit, "{0}")'.format(field.units)
            else:
                unitCheck = 'unit.empty() || unit == "--"'

            if field.ucd not in ucd_skip_list:
                statement = Template(UCD_FIELD_PARSE_PATTERN).substitute(
                        ucd=field.ucd,
                        unitCheckExpression=unitCheck,
                        votype=db_to_votable_type_map[field.raw_type],
                        fieldname=field.name,
                        cpptype=field.dtype)
                if count:
                    out.write('\n    else ')
                out.write(statement)

                # special case for RA and declination
                if field.ucd.casefold() == 'pos.eq.ra;meta.main'.casefold():
                    out.write(
                        Template('        ra_buffer[row_index] = components[row_index].$fieldname;\n').substitute(
                            fieldname=field.name))
                elif field.ucd.casefold() == 'pos.eq.dec;meta.main'.casefold():
                    out.write(
                        Template('        dec_buffer[row_index] = components[row_index].$fieldname;\n').substitute(
                            fieldname=field.name))

                out.write('    }')
                count += 1
            elif field.ucd == 'meta.code':
                statement = Template(NO_UCD_FIELD_PARSE_PATTERN).substitute(
                        unitCheckExpression=unitCheck,
                        votype=db_to_votable_type_map[field.raw_type],
                        fieldname=field.name,
                        cpptype=field.dtype)
                if count:
                    out.write('\n    else ')
                out.write(statement)
                count += 1

        out.write('\n}')

        out.write(HEADER_POSTAMBLE)


if __name__ == '__main__':
    print('Generating tables ...')
    for f in files:
        print('\t' + f['output'])
        data = load(
            f['input'],
            parse_cols=f['parse_cols'],
            skiprows=f['skiprows'])
        write_output(data, f['output'])

    print('Generating views ...')
    for f in view_files:
        print('\t' + f['output'])
        # Load all the input data frames that will be combined into the view
        view_data = [
            load(i, parse_cols=f['parse_cols'], skiprows=f['skiprows'])
            for i in f['inputs']]

        # concatenate the dataframes, and then select just the fields that have
        # the view flag set
        data = pd.concat(view_data).query('lsm_view == True')
        write_output(data, f['output'], is_view=True)

    print('Generating VOTable to datamodel parsing code ...')
    write_votable_parser()

    print('Done')
