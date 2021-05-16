#!/usr/bin/env python3
#
#   ucdb_script.py
#
#   Created by Edmund Kapusniak on 03/06/2020.
#   Copyright © 2020 Edmund Kapusniak.
#
#   Licensed under the ISC License. See LICENSE file in the project root for
#   full license information.
#

#
# Run this script to generate script numbers and corresponding script codes
# from the normative ISO 15924 text file.  https://unicode.org/iso15924/
#

import sys
from os import path


def parse_entry( line ):
    split = line.split( '#' )
    line = split[ 0 ].strip( ' \t\n;' )
    if not line:
        return None
    return tuple( map( str.strip, line.split( ';' ) ) )

def parse_file( file ):
    entries = [ entry for entry in ( parse_entry( line ) for line in file ) if entry is not None ]
    return entries


iso15924_path = sys.argv[ 1 ]

with open( iso15924_path, 'r' ) as file:
    script_data = parse_file( file )

script_order = [
    next( x for x in script_data if x[ 0 ] == 'Zyyy' ),
    next( x for x in script_data if x[ 0 ] == 'Zinh' ),
    next( x for x in script_data if x[ 0 ] == 'Zzzz' )
]

script_data = script_order + [ x for x in script_data if x not in script_order ]
script_data = [ ( x[ 0 ], x[ 4 ] ) for x in script_data if x[ 4 ] and x[ 0 ] != 'Geok' ]

if len( sys.argv ) < 3 or sys.argv[ 2 ] != 'enum':

    # Output table

    print()
    print( "extern const uint32_t UCDB_SCRIPT_CODE[] =" )
    print( "{" )
    for code, name in script_data:
        print( f"    SCRIPT_CODE( '{code[ 0 ]}', '{code[ 1 ]}', '{code[ 2 ]}', '{code[ 3 ]}' )," )
    print( "};" )
    print()

else:

    # Output enums

    print()
    print( "enum ucdb_script" )
    print( "{" )
    for code, name in script_data:
        print( f"    UCDB_SCRIPT_{name.upper()}," )
    print( "};" )
    print()
