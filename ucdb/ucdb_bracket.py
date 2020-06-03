#!/usr/bin/env python3
#
#   ucdb_bracket.py
#
#   Created by Edmund Kapusniak on 03/06/2020.
#   Copyright © 2020 Edmund Kapusniak.
#
#   Licensed under the MIT License. See LICENSE file in the project root for
#   full license information.
#

#
# Run this script to generate a table of paired brackets and of canoncial
# bracket decompositions from the Unicode Character Database.
#

import sys
from os import path


# Load UCD data.

def parse_entry( line ):
    split = line.split( '#' )
    line = split[ 0 ].strip( ' \t\n;' )
    if not line:
        return None
    return tuple( map( str.strip, line.split( ';' ) ) )

def parse_file( file ):
    entries = [ entry for entry in ( parse_entry( line ) for line in file ) if entry is not None ]
    return entries


ucd_path = sys.argv[ 1 ]

with open( path.join( ucd_path, 'UnicodeData.txt' ), 'r' ) as file:
    unicode_data = parse_file( file )

with open( path.join( ucd_path, 'BidiBrackets.txt' ), 'r' ) as file:
    bidi_brackets = parse_file( file )


# Build bracket pair table.

paired = { int( entry[ 0 ], 16 ) for entry in bidi_brackets }
decomp = { entry[ 0 ] : entry[ 5 ] for entry in unicode_data if int( entry[ 0 ], 16 ) in paired }
decomp = { k : v for k, v in decomp.items() if len( v.split() ) == 1 }

filter_brackets = []
for bracket, bpaired, kind in bidi_brackets:
    if bracket in decomp:
        continue
    bpaired = decomp.get( bpaired, bpaired )
    filter_brackets.append( ( bracket, bpaired, kind ) )

bidi_brackets = filter_brackets


# Output

print()

print( "const ucdb_mapped_entry UCDB_BRACKET_MAPPED[] = {" )
for cp in sorted( decomp.keys() ):
    print( f"    {{ 0x{cp}, 0x{decomp[cp]} }}," )
print( "};" )
print()

print( "const ucdb_paired_entry UCDB_BRACKET_PAIRED[] = {" )
for bracket, bpaired, kind in bidi_brackets:
    print( f"    {{ 0x{bracket}, 0x{bpaired}, {'UCDB_BRACKET_OPEN' if kind == 'o' else 'UCDB_BRACKET_CLOSE'} }}," )
print( "};" )
print()

