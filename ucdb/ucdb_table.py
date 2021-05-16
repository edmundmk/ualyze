#!/usr/bin/env python3
#
#   ucdb_table.py
#
#   Created by Edmund Kapusniak on 28/05/2020.
#   Copyright © 2020 Edmund Kapusniak.
#
#   Licensed under the ISC License. See LICENSE file in the project root for
#   full license information.
#

#
# Run this script to generate the property table:
#
#       python3 ucdb_table.py <path-to-unidata-directory>
#
# Uses packTab https://github.com/harfbuzz/packtab/.  Note that as of the
# time of writing, the pip version of packTab is outdated and does not work.
#

import sys
from os import path
import packTab


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

with open( path.join( ucd_path, 'Scripts.txt' ), 'r' ) as file:
    scripts = parse_file( file )

with open( path.join( ucd_path, 'extracted/DerivedBidiClass.txt' ), 'r' ) as file:
    bidi_class = parse_file( file )

with open( path.join( ucd_path, 'LineBreak.txt' ), 'r' ) as file:
    line_break = parse_file( file )

with open( path.join( ucd_path, 'auxiliary/GraphemeBreakProperty.txt' ), 'r' ) as file:
    grapheme_break = parse_file( file )

with open( path.join( ucd_path, 'BidiBrackets.txt' ), 'r' ) as file:
    bidi_brackets = parse_file( file )

with open( path.join( ucd_path, 'emoji/emoji-data.txt' ), 'r' ) as file:
    emoji_data = parse_file( file )

with open( path.join( ucd_path, 'EastAsianWidth.txt' ), 'r' ) as file:
    east_asian_width = parse_file( file )


# Generate lookup tables from data.

def build_map( entries ):
    result = {}
    for c, k, *_ in entries:
        c = c.split( '..' )
        lower = int( c[ 0 ], 16 )
        upper = int( c[ 1 ], 16 ) if len( c ) > 1 else lower
        for c in range( lower, upper + 1 ):
            result[ c ] = k
    return result

zspace = { int( entry[ 0 ], 16 ) for entry in unicode_data if entry[ 2 ] == 'Zs' }
mn_or_mc = { int( entry[ 0 ], 16 ) for entry in unicode_data if entry[ 2 ] in ( 'Mn', 'Mc' ) }
scripts = build_map( scripts )
bidi_class = build_map( bidi_class )
line_break = build_map( line_break )
grapheme_break = build_map( grapheme_break )
paired = { int( entry[ 0 ], 16 ) for entry in bidi_brackets }
extended_pictographic = build_map( ( entry for entry in emoji_data if entry[ 1 ] == 'Extended_Pictographic' ) )
east_asian_width = build_map( east_asian_width )


# LB1: Resolve SA+Mn and SA+Mc to CM.
# LB30: OP/CP with East_Asian_Width in F, W, H -> EAST_ASIAN_OP/EAST_ASIAN_CP

def resolve_lbreak_class( c, lbreak ):
    if lbreak == 'SA' and c in mn_or_mc:
        return 'CM'
    if ( lbreak == 'OP' or lbreak == 'CP' ) and ( east_asian_width.get( c, 'N' ) in [ 'F', 'W', 'H' ] ):
        return 'EAST_ASIAN_' + lbreak
    else:
        return lbreak


# Treat Extended_Pictographic as its own cluster break class.

def resolve_cbreak_class( c, cbreak ):
    return extended_pictographic.get( c, cbreak ) if cbreak == 'XX' else cbreak


# Build unique list of records.

records = []
record_index = {}
data = []

def insert_record( record ):
    if record in record_index:
        return record_index[ record ]
    index = len( records )
    record_index[ record ] = index
    records.append( record )
    return index

insert_record( ( 'Unknown', 'L', 'XX', False, 'XX', False ) )

for c in range( 0x110000 ):
    record = (
        scripts.get( c, 'Unknown' ),
        bidi_class.get( c, 'L' ),
        resolve_lbreak_class( c, line_break.get( c, 'XX' ) ),
        c in zspace,
        resolve_cbreak_class( c, grapheme_break.get( c, 'XX' ) ),
        c in paired
    )
    data.append( insert_record( record ) )


if len( sys.argv ) < 3 or sys.argv[ 2 ] != 'enum':

    # Output table

    print()

    print( "extern const ucdb_entry UCDB_TABLE[] = {" )
    for script, bclass, lbreak, zspace, cbreak, paired in records:
        print( f"    {{ UCDB_SCRIPT_{script.upper()}, UCDB_BIDI_{bclass}, UCDB_LBREAK_{lbreak}, {'true' if zspace else 'false'}, UCDB_CBREAK_{cbreak.upper()}, {'true' if paired else 'false'} }}," )
    print( "};" )
    print()


    # Use packTab to build an index mapping a character to a record.

    solutions = packTab.pack_table( data, 0, None )
    solution = packTab.pick_solution( solutions, compression=1 )
    code = packTab.Code( 'ucdb' )
    expr = solution.genCode( code, 'index' )
    code.print_c()
    print()

else:

    # Build enums

    cbreak_enum = { record[ 4 ] for record in records }

    print()

    bclass_enum = { record[ 1 ].upper() for record in records }
    bclass_enum.remove( "L" )
    bclass_enum.remove( "R" )
    bclass_enum.remove( "AL" )
    bclass_enum = [ "L", "R", "AL" ] + sorted( bclass_enum )

    print( "enum ucdb_bclass" )
    print( "{" )
    for bclass in bclass_enum:
        print( f"    UCDB_BIDI_{bclass}," )
    print( "};" )

    print()

    lbreak_enum = { record[ 2 ].upper() for record in records }
    lbreak_enum.add( "EAST_ASIAN_CP" )
    lbreak_enum.add( "EAST_ASIAN_OP" )

    print( "enum ucdb_lbreak" )
    print( "{" )
    for lbreak in sorted( lbreak_enum ):
        print( f"    UCDB_LBREAK_{lbreak}," )
    print( "};" )

    print()

    cbreak_enum = { record[ 4 ].upper() for record in records }
    print( "enum ucdb_cbreak" )
    print( "{" )
    for cbreak in sorted( cbreak_enum ):
        print( f"    UCDB_CBREAK_{cbreak}," )
    print( "};" )

    print()

