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
        'input': '~/Dropbox/GSM_casda.continuum_component_description_v1.8.xlsx',
        'output': '../schema/ContinuumComponent.i',
        'parse_cols': None,
        'skiprows': [0],
    },
    {
        'input': '~/Dropbox/GSM_casda_polarisation_v0.6.xlsx',
        'output': '../schema/Polarisation.i',
        'parse_cols': None,
        'skiprows': [0],
    },
    {
        'input': '~/Dropbox/GSM_data_source_description_v1.0.xlsx',
        'output': '../schema/DataSource.i',
        'parse_cols': None,
        'skiprows': [0],
    },
]

view_files = [
    {
        'inputs': [
            '~/Dropbox/GSM_casda.continuum_component_description_v1.8.xlsx',
            '~/Dropbox/GSM_casda_polarisation_v0.6.xlsx',
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
    'BIGINT': 'long',
    'BIGINT UNSIGNED': 'unsigned long',
    'REAL': 'float',
    'DOUBLE': 'double',
    'VARCHAR': 'std::string',
    'TEXT': 'std::string',
    'BOOLEAN': 'bool',
    'INTEGER': 'int',
    'INTEGER UNSIGNED': 'unsigned int',
    'DATETIME': 'boost::posix_time::ptime',
}

class Field(object):
    def __init__(self, df_row, is_view):
        self.name = df_row.name.strip()
        self.comment = df_row.description.strip()
        self.dtype = type_map[df_row.datatype.strip()]
        self.units = df_row.units.strip()
        self.indexed = df_row.index
        self.nullable = df_row.nullable
        self.is_view = is_view
        self._magic_names = {}

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        s = self._comment()

        if not self.is_view:
            # handle any pragmas for magic field names
            if self.name in self._magic_names:
                for pragma in self._magic_names[self.name]:
                    s.append(pragma)

            if self.indexed:
                s.append('#pragma db index')

            if self.nullable:
                s.append('#pragma db null')
            else:
                s.append('#pragma db not_null')

        # the field definition
        s.append('{0} {1};'.format(
            self.dtype,
            self.name))

        # indentation
        # indent = 4
        # s = [indent * ' ' + line for line in s]

        s.append('\n')

        return '\n'.join(s)

    def _comment(self):
        comments = []
        if self.units:
            comments.append('@brief {0} ({1})'.format(self.comment, self.units))
        else:
            comments.append('@brief {0}'.format(self.comment))
        return ['/// ' + c for c in comments]


def get_fields(data_frame, is_view=False):
    "Unpacks a tablespec data frame into a list of field objects"
    fields = []
    for row in data_frame.itertuples(index=False):
        # print(row)
        field = Field(row, is_view)
        fields.append(field)

    return fields


def write_output(data, filename, is_view=False):
    with open(filename, 'w') as out:
        for field in get_fields(data, is_view):
            out.write(str(field))


if __name__ == '__main__':
    print('Generating tables ...')
    for f in files:
        print('\t' + f['output'])
        data = load(f['input'], parse_cols=f['parse_cols'], skiprows=f['skiprows'])
        write_output(data, f['output'])

    print('Generating views ...')
    for f in view_files:
        print('\t' + f['output'])
        # Load all the input data frames that will be combined into the view
        view_data = [load(i, parse_cols=f['parse_cols'], skiprows=f['skiprows']) for i in f['inputs']]
        # concatenate the dataframes, and then select just the fields that have the view flag set
        data = pd.concat(view_data).query('lsm_view == True')
        write_output(data, f['output'], is_view=True)

    print('Done')
