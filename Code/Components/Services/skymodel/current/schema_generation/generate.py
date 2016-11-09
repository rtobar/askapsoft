# coding: utf-8

import pandas as pd

# This notebook generates ODB schema code for direct inclusion into the ASKAP Sky Model Service code.

# Define the Outputs
# Each schema file defines a dictionary with the following keys:
# * **input**: the input spreadsheet
# * **output**: the output file
# * **parse_cols**: zero-based column indicies for parsing from the spreadsheet
# * **skiprows**: zero-based row indicies that should be skipped in the spreadsheet

files = [
    {
        'input': 'GSM_casda.continuum_component_description_v1.8.xlsx',
        'output': '../schema/ContinuumComponent.i',
        'parse_cols': [1,2,3,5,6,9],
        'skiprows': [0,1,2],
    },
    {
        'input': 'GSM_casda_polarisation_v0.6.xlsx',
        'output': '../schema/Polarisation.i',
        'parse_cols': [1,2,3,5,6,9],
        'skiprows': [0,1,2,3],
    },
    {
        'input': 'GSM_casda.continuum_island_description_v0.5.xlsx',
        'output': '../schema/ContinuumIsland.i',
        'parse_cols': [1,2,3,5,6,9],
        'skiprows': [0,1,2,3],
    },
]

def load(
    filename,
    sheetname='Catalogue description',
    parse_cols=None,
    skiprows=None,
    converters={
            'Index': bool,
            'include in gsm': bool,
            }):
    """Load the table data from a CASDA definition spreadsheet"""
    data = pd.read_excel(
        filename,
        sheetname=sheetname,
        converters=converters,
        parse_cols=parse_cols,
        skiprows=skiprows)

    return data[data['include in gsm'] == True]

type_map = {
    'BIGINT': 'long',
    'BIGINT UNSIGNED': 'unsigned long',
    'REAL': 'float',
    'DOUBLE': 'double',
    'VARCHAR': 'std::string',
    'BOOLEAN': 'bool',
    'INTEGER': 'int',
}

class Field(object):
    def __init__(self, name='', comment='', dtype='', units='', indexed=False):
        self.name = name
        self.comment = comment
        self.dtype = dtype
        self.units = units
        self.indexed = indexed

        self._magic_names = {
            'id': ['#pragma db id auto'],
            'version': ['#pragma db version'],
        }

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        s = self._comment()

        # handle any pragmas for magic field names
        if self.name in self._magic_names:
            for pragma in self._magic_names[self.name]:
                s.append(pragma)

        if self.indexed:
            s.append('#pragma db index')

        # the field definition
        s.append('{0} {1};'.format(
            self.dtype,
            self.name))

        # indentation
        indent = 4
        s = [indent * ' ' + line for line in s]

        s.append('\n')

        return '\n'.join(s)

    def _comment(self):
        c = []
        c.append('// @brief {0}'.format(self.comment))
        c.append('// @units {0}'.format(self.units))
        return c

def get_fields(data_frame):
    "Unpacks a tablespec data frame into a list of field objects"
    fields = []
    for r in data_frame.itertuples(index=False):
        field = Field(
            name=r[0],
            comment=r[1],
            dtype=type_map[r[2]],
            units=r[3],
            indexed=r[4])
        fields.append(field)

    return fields

if __name__ == '__main__':
    print('Generating ...')
    for f in files:
        data = load(f['input'], parse_cols=f['parse_cols'], skiprows=f['skiprows'])
        print('\t' + f['output'])
        with open(f['output'], 'w') as out:
            for field in get_fields(data):
                out.write(str(field))
    print('Done')
