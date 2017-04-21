# coding: utf-8
'''
This script generates ODB schema code for direct inclusion into the ASKAP Sky Model Service code.
It also generates the Ice DTO objects from the same definitions.

Requirements:
    * Python 3
    * Pandas >= 0.18 (older versions do not work)
    * xlrd (used by Pandas for reading Excel files)
'''

import pandas as pd
from string import Template

# Define the Outputs
# Each schema file defines a dictionary with the following keys:
# * **input**: the input spreadsheet
# * **output**: the output file
# * **parse_cols**: zero-based column indicies for parsing from the spreadsheet
# * **skiprows**: zero-based row indicies that should be skipped in the spreadsheet

# The mapping from database types to C++ types
TYPE_MAP = {
    'BIGINT': 'boost::int64_t',
    'REAL': 'float',
    'DOUBLE': 'double',
    'VARCHAR': 'std::string',
    'TEXT': 'std::string',
    'BOOLEAN': 'bool',
    'INTEGER': 'boost::int32_t',
    'DATETIME': 'boost::posix_time::ptime',
}

# The mapping from database types to Slice types
SLICE_TYPE_MAP = {
    'BIGINT': 'long',
    'REAL': 'float',
    'DOUBLE': 'double',
    'VARCHAR': 'string',
    'TEXT': 'string',
    'BOOLEAN': 'bool',
    'INTEGER': 'int',
    'DATETIME': 'string',  # will need to convert to a canonical string representation
}

# Indentation constants
I4 = '    '
I8 = '        '

COMMON_FILE_HEADER = '''\
/// ----------------------------------------------------------------------------
/// This file is generated by schema_definitions/generate.py.
/// Do not edit directly or your changes will be lost!
/// ----------------------------------------------------------------------------
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
'''

COMMON_SLICE_HEADER = COMMON_FILE_HEADER + '''
#pragma once

#include <CommonTypes.ice>
'''

SLICE_NAMESPACES = '''
module askap
{
module interfaces
{
module skymodelservice
{
'''

SEARCH_CRITERIA_HEADER = COMMON_SLICE_HEADER + SLICE_NAMESPACES + \
'''\
    /**
     * Allows specification of additional criteria for sky model searches.
     * For fields that are invalid for negative values, the default negative
     * value indicates that a criteria will not be used. For fields that are
     * valid for negative values, an additional boolean flag indicates whether
     * that criteria will be applied to the search or not.
     *
     * All criteria are combined with the AND operator
     **/
    struct SearchCriteria
    {
'''

CONTINUUM_COMPONENT_HEADER = '''

    /**
     * Define a sequence for storing the optional polarisation data inside the
     * component
     **/
    sequence<ContinuumComponentPolarisation> PolarisationOpt;

    /**
     * A continuum component.
     **/
    struct ContinuumComponent
    {
        /// @brief Should only ever contain 0 or 1 elements.
        PolarisationOpt polarisation;

'''

POLARISATION_HEADER = COMMON_SLICE_HEADER + SLICE_NAMESPACES + \
'''
    /**
     * Continuum component polarisation data.
     **/
    struct ContinuumComponentPolarisation
    {
'''

SLICE_FOOTER = '''\
    };

};
};
};
'''

VOTABLE_PARSER_HEADER = COMMON_FILE_HEADER + '''
#pragma once

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
#include "SmsTypes.h"

namespace askap {
namespace cp {
namespace sms {

'''

HEADER_FOOTER = '''

}
}
}
'''

PARSE_POLARISATION_ROW_FIELD_START = '''
void parsePolarisationRowField(
    const std::string& ucd,
    const std::string& name,
    const std::string& type,
    const std::string& unit,
    const std::string& value,
    boost::shared_ptr<datamodel::Polarisation> pPol) {

    ASKAPASSERT(pPol.get());
'''

PARSE_COMPONENT_ROW_FIELD_START = '''
void parseComponentRowField(
    size_t row_index,
    const std::string& ucd,
    const std::string& name,
    const std::string& type,
    const std::string& unit,
    const std::string& value,
    std::vector<datamodel::ContinuumComponent>& components) {

    ASKAPASSERT(row_index >= 0);
    ASKAPASSERT(row_index < components.size());
'''

COMPONENT_UCD_FIELD_PARSE_PATTERN = '''
    if (boost::iequals(ucd, "$ucd")) {
        ASKAPASSERT($unitCheckExpression);
        ASKAPASSERT(boost::iequals(type, "$votype"));
        components[row_index].$fieldname = boost::lexical_cast<$cpptype>(value);
'''

COMPONENT_NO_UCD_FIELD_PARSE_PATTERN = '''
    if (boost::iequals(name, "$fieldname")) {
        // Some fields do not have a unique UCD. They are matched by name.
        ASKAPASSERT($unitCheckExpression);
        ASKAPASSERT(boost::iequals(type, "$votype"));
        components[row_index].$fieldname = boost::lexical_cast<$cpptype>(value);
    }'''

POLARISATION_UCD_FIELD_PARSE_PATTERN = '''
    if (boost::iequals(ucd, "$ucd")) {
        ASKAPASSERT($unitCheckExpression);
        ASKAPASSERT(boost::iequals(type, "$votype"));
        pPol->$fieldname = boost::lexical_cast<$cpptype>(value);
'''

POLARISATION_NO_UCD_FIELD_PARSE_PATTERN = '''
    if (boost::iequals(name, "$fieldname")) {
        // Some fields do not have a unique UCD. They are matched by name.
        ASKAPASSERT($unitCheckExpression);
        ASKAPASSERT(boost::iequals(type, "$votype"));
        pPol->$fieldname = boost::lexical_cast<$cpptype>(value);
    }'''

DATA_MARSHALLER_HEADER = COMMON_FILE_HEADER + '''
#pragma once

// System includes
#include <string>
#include <vector>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

// ASKAPsoft includes
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>

// Ice interfaces
#include <SkyModelService.h>
#include <SkyModelServiceDTO.h>

// Local package includes
#include "datamodel/ContinuumComponent.h"
#include "SmsTypes.h"

namespace askap {
namespace cp {
namespace sms {

// Alias for the Ice type namespace
namespace ice_interfaces = askap::interfaces::skymodelservice;

/// @brief Transfers data from the datamodel::ContinuumComponent class to the
/// Ice DTO class. If present, polarisation data will be added to the optional
/// polarisation member of ContinuumComponent.
///
/// @param[in] components Pointer to a vector of components for transfer.
/// @throw AskapError Thrown if there are errors.
/// @return askap::interfaces::skymodelservice::ComponentSeq
ice_interfaces::ComponentSeq marshallComponentsToDTO(
    boost::shared_ptr< std::vector<datamodel::ContinuumComponent> > components)
{
    // This might be nice if I can define a conversion operator from
    // datamodel::ContinuumComponent to ice_interfaces::ContinuumComponent
    // But unfortunately, both source and destination classes are produced by
    // the corresponding systems (Ice slice compiler, ODB class generation),
    // so I have no easy way to define the conversion. Doing it manually isn't
    // too bad though due to the code generation.
    //return ice_interfaces::ComponentSeq(components->begin(), components->end());

    // preallocate space so I can try OpenMP loop parallelism
    ice_interfaces::ComponentSeq dst(components->size());
    //ice_interfaces::ComponentSeq dst();

    int i = 0;
    #pragma parallel for
    for (std::vector<datamodel::ContinuumComponent>::const_iterator it = components->begin();
        it != components->end();
        it++, i++) {

'''

MARSHALLING_FUNCTION_TAIL = '''

    return dst;
}
'''

POLARISATION_START = '''
        // polarisation may not be present
        if (it->polarisation.get()) {
            const datamodel::Polarisation* src_pol = it->polarisation.get();

            // I need to profile how the performance of stack instance followed
            // by assignment to the vector compares to the resize and assign
            // members in place. On the surface, it is comparing a default ctor, field assignment, and copy ctor
            // to default ctor and field assignment. I hope to avoid the additional
            // copy when pushing the stack instance into the vectory.
            // Of course, the compiler could be doing all sorts of optimisations, so
            // sticking to the clearer code is probably best.
            ice_interfaces::ContinuumComponentPolarisation dst_pol;

            // Or:
            //dst[i].polarisation.resize(1);
            // And then:
            //dst[i].polarisation[0].polPeakFit = src_pol->pol_peak_fit;

'''

POLARISATION_END = '''
            // Assign our stack variable to the vector. Hopefully the additional
            // copy constructor gets optimised away.
            dst[i].polarisation.push_back(dst_pol);
        } // end of polarisation branch
    } // end of component loop
'''

QUERY_BUILDER_HEADER = COMMON_FILE_HEADER + '''
#pragma once

// System includes
#include <string>
#include <boost/shared_ptr.hpp>

// ASKAPsoft includes
#include <askap/AskapError.h>
#include <askap/AskapLogging.h>

// Ice interfaces
#include <SkyModelServiceCriteria.h>

// Local package includes
#include "datamodel/ContinuumComponent.h"

namespace askap {
namespace cp {
namespace sms {

// Alias for the Ice type namespace
namespace ice_interfaces = askap::interfaces::skymodelservice;

/// @brief Builds an ODB ContinuumComponent query from the Ice criteria struct
///
/// @param[in] The Ice SearchCriteria struct
/// @return The ContinuumComponent query
odb::query<datamodel::ContinuumComponent> queryBuilder(const sms_interface::SearchCriteria& criteria)
{
    typedef odb::query<datamodel::ContinuumComponent> query;
    query q;

'''

QUERY_BUILDER_FOOTER = '''
    return q;
}''' + HEADER_FOOTER

#------------------------------------------------------------
# Configuration
#------------------------------------------------------------
CONTINUUM_COMPONENT_SPEC = './GSM_casda.continuum_component_description.xlsx'
POLARISATION_SPEC = './GSM_casda_polarisation.xlsx'

FILES = [
    {
        'input': CONTINUUM_COMPONENT_SPEC,
        'output': '../schema/ContinuumComponent.i',
        'parse_cols': None,
        'skiprows': [0],
    },
    {
        'input': POLARISATION_SPEC,
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

# This is setup to write to the same file, with continuum component appending to
# the polarisation file (thus the difference between headers and footers)
# It also means that order is important, the component definition must be last
SLICE_FILES = [
    {
        'input': POLARISATION_SPEC,
        'output': '../SkyModelServiceDTO.ice',
        'parse_cols': None,
        'skiprows': [0],
        'file_header': POLARISATION_HEADER,
        'file_footer': '    };',
        'append_to_file': False,
    },
    {
        'input': CONTINUUM_COMPONENT_SPEC,
        'output': '../SkyModelServiceDTO.ice',
        'parse_cols': None,
        'skiprows': [0],
        'file_header': CONTINUUM_COMPONENT_HEADER,
        'file_footer': SLICE_FOOTER,
        'append_to_file': True,
    },
]


#------------------------------------------------------------
# Utility functions
#------------------------------------------------------------
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
            'generate_query_criteria': bool,
            'negative_is_invalid': bool,
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


def to_camel_case(snake_str):
    '''Converts a snake string to lower camel case.
    E.G.: my_identifier --> myIdentifier
    '''
    components = snake_str.split('_')
    # We capitalize the first letter of each component except the first one
    # with the 'title' method and join them together.
    return components[0] + "".join(x.title() for x in components[1:])


class Field(object):
    "Represents a database field"
    def __init__(self, df_row, is_view, type_map, indent=0, camel_case=False):
        self.name = df_row.name.strip()
        if camel_case:
            self.name = to_camel_case(self.name)

        self.comment = df_row.description.strip()
        self.raw_type = df_row.datatype.strip()
        self.dtype = type_map[self.raw_type]
        self.units = df_row.units.strip()
        self.indexed = df_row.index
        self.nullable = df_row.nullable
        self.is_view = is_view
        self.lsm_view = df_row.lsm_view
        self._magic_names = {}
        self._indent = indent
        self.generate_criteria = df_row.generate_query_criteria
        self.negative_is_invalid = df_row.negative_is_invalid

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
        _str = [self._indent * ' ' + line for line in _str]

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


def get_fields(data_frame, type_map, is_view, indent=0, camel_case=False):
    "Unpacks a tablespec data frame into a list of field objects"
    fields = []
    for row in data_frame.itertuples(index=False):
        field = Field(row, is_view, type_map, indent=indent, camel_case=camel_case)
        fields.append(field)
    return fields


def write_output(
    data_frame,
    filename,
    type_map,
    is_view=False,
    indent=0,
    camel_case=False,
    append_to_file=False,
    file_header=None,
    file_footer=None):
    "Writes the data model fields to a file"
    mode = 'a' if append_to_file else 'w'
    print("\t" + filename)
    with open(filename, mode) as out:
        if file_header:
            out.write(file_header)
        for field in get_fields(data_frame, type_map, is_view, indent=indent, camel_case=camel_case):
            if not is_view or (is_view and field.lsm_view):
                out.write(str(field))
        if file_footer:
            out.write(file_footer)


def write_field_parsing_code(
    out,
    fields,
    db_to_votable_type_map,
    ucd_skip_list,
    ucd_field_template,
    no_ucd_field_template,
    special_case_ra_dec=False):
    count = 0
    for field in fields:
        if field.units:
            unit_check = 'boost::iequals(unit, "{0}")'.format(field.units)
        else:
            unit_check = 'unit.empty() || unit == "--" || unit == "none"'

        if field.ucd not in ucd_skip_list:
            statement = Template(ucd_field_template).substitute(
                    ucd=field.ucd,
                    unitCheckExpression=unit_check,
                    votype=db_to_votable_type_map[field.raw_type],
                    fieldname=field.name,
                    cpptype=field.dtype)
            if count:
                out.write('\n    else ')
            out.write(statement)
            out.write('    }')
            count += 1
        elif field.ucd == 'meta.code':
            statement = Template(no_ucd_field_template).substitute(
                    unitCheckExpression=unit_check,
                    votype=db_to_votable_type_map[field.raw_type],
                    fieldname=field.name,
                    cpptype=field.dtype)
            if count:
                out.write('\n    else ')
            out.write(statement)
            count += 1


def generate_votable_parser():
    "Writes the VOTable to data model class parsing code"
    print('\t../service/VOTableParser.h')
    with open('../service/VOTableParser.h', 'w') as out:
        out.write(VOTABLE_PARSER_HEADER)

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

        out.write(PARSE_COMPONENT_ROW_FIELD_START)
        write_field_parsing_code(
            out,
            get_fields(load(CONTINUUM_COMPONENT_SPEC, skiprows=[0]), TYPE_MAP, False),
            db_to_votable_type_map,
            ['', 'meta.code'],
            COMPONENT_UCD_FIELD_PARSE_PATTERN,
            COMPONENT_NO_UCD_FIELD_PARSE_PATTERN,
            special_case_ra_dec=True)
        out.write('\n}\n\n')

        # Write the polarisation parser function
        out.write(PARSE_POLARISATION_ROW_FIELD_START)
        write_field_parsing_code(
            out,
            get_fields(load(POLARISATION_SPEC, skiprows=[0]), TYPE_MAP, False),
            db_to_votable_type_map,
            ['', 'meta.code'],
            POLARISATION_UCD_FIELD_PARSE_PATTERN,
            POLARISATION_NO_UCD_FIELD_PARSE_PATTERN,
            special_case_ra_dec=False)
        out.write('\n}')
        out.write(HEADER_FOOTER)


def generate_orm_to_dto_marshaller():
    '''Generates a function for marshalling data from the ORM classes into
    the Ice DTO structures.
    Only database fields that have the lsm_view flag set to true are marshalled.
    '''
    print('\t../service/DataMarshalling.h')
    with open('../service/DataMarshalling.h', 'w') as out:
        out.write(DATA_MARSHALLER_HEADER)

        # Copy the component data
        out.write(I8 + '// Copy the component data\n')
        for f in get_fields(load(CONTINUUM_COMPONENT_SPEC, skiprows=[0]), TYPE_MAP, False):
            # only fields with lsm_view=True are written to the Ice structs
            if f.lsm_view:
                out.write('{0}dst[i].{1} = it->{2};\n'.format(
                    I8,
                    to_camel_case(f.name),
                    f.name))

        out.write(POLARISATION_START)
        # todo: polarisation fields
        for f in get_fields(load(POLARISATION_SPEC, skiprows=[0]), TYPE_MAP, False):
            # only fields with lsm_view=True are written to the Ice structs
            if f.lsm_view:
                out.write('{0}    dst_pol.{1} = src_pol->{2};\n'.format(
                    I8,
                    to_camel_case(f.name),
                    f.name))
        out.write(POLARISATION_END)
        out.write(MARSHALLING_FUNCTION_TAIL)
        out.write(HEADER_FOOTER)


def generate_query_builder():
    '''Generates a function for building an ODB query from the Ice SearchCriteria struct.'''
    def emit_query_term(out, minmax, f):
        ice_criteria_name = to_camel_case(minmax + '_' + f.name)
        op = '>=' if minmax == 'min' else '<='

        if f.negative_is_invalid:
            out.write('{0}if (criteria.{1} >= 0)\n'.format(I4, ice_criteria_name))
        else:
            out.write('{0}if (criteria.{1})\n'.format(
                I4,
                to_camel_case('use_' + minmax + '_' + f.name)))

        out.write('{0}q = q && query::{1} {2} criteria.{3};\n'.format(
            I8,
            f.name,
            op,
            ice_criteria_name))

    output = '../service/QueryBuilder.h'
    print('\t' + output)

    with open(output, 'w') as out:
        out.write(QUERY_BUILDER_HEADER)

        for f in get_fields(load(CONTINUUM_COMPONENT_SPEC, skiprows=[0]), SLICE_TYPE_MAP, False):
            if f.lsm_view and f.generate_criteria:
                emit_query_term(out, 'min', f)
                emit_query_term(out, 'max', f)

        out.write(QUERY_BUILDER_FOOTER)


def generate_search_criteria_structures():
    '''Generates the Ice search criteria structures.'''

    output = '../SkyModelServiceCriteria.ice'
    print('\t' + output)

    with open(output, 'w') as out:
        out.write(SEARCH_CRITERIA_HEADER)

        # generate criteria for the component data
        for f in get_fields(load(CONTINUUM_COMPONENT_SPEC, skiprows=[0]), SLICE_TYPE_MAP, False):
            if f.lsm_view and f.generate_criteria:
                for minmax in ['min_', 'max_']:
                    criteriaName = to_camel_case(minmax + f.name)
                    if f.negative_is_invalid:
                        out.write('{0}{2} {1} = -1;\n'.format(
                            I8,
                            criteriaName,
                            f.dtype))
                    else:
                        out.write('{0}{2} {1} = 0;\n'.format(
                            I8,
                            criteriaName,
                            f.dtype))
                        out.write('{0}bool {1} = false;\n'.format(
                            I8,
                            to_camel_case('use_' + minmax + f.name)))

        out.write(SLICE_FOOTER)


def generate_database_schema():
    for f in FILES:
        data = load(
            f['input'],
            parse_cols=f['parse_cols'],
            skiprows=f['skiprows'])
        write_output(data, f['output'], TYPE_MAP)


def generate_ice_dto():
    for f in SLICE_FILES:
        data = load(
            f['input'],
            parse_cols=f['parse_cols'],
            skiprows=f['skiprows'])

        write_output(
            data,
            f['output'],
            SLICE_TYPE_MAP,
            is_view=True,
            indent=8,
            camel_case=True,
            append_to_file=f['append_to_file'],
            file_header=f['file_header'],
            file_footer=f['file_footer'])


if __name__ == '__main__':
    print('Generating database schema files ...')
    generate_database_schema()

    print('Generating slice data transfer objects (DTO) ...')
    generate_ice_dto()

    print('Generating VOTable to datamodel parsing code ...')
    generate_votable_parser()

    print('Generating data marshalling function ...')
    generate_orm_to_dto_marshaller()

    print('Generating query builder function ...')
    generate_query_builder()

    print('Generating Ice search criteria ...')
    generate_search_criteria_structures()

    print('Done')
    print("* Don't forget to move the generated Ice files to Code/Interfaces/slice/current with 'make generate_ice'")
