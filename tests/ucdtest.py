#!/usr/bin/env python3
#
#   ucdtest.py
#
#   Created by Edmund Kapusniak on 31/05/2020.
#   Copyright © 2020 Edmund Kapusniak.
#
#   Licensed under the MIT License. See LICENSE file in the project root for
#   full license information.
#

#
# Tests ualyze against tests from the Unicode Character Database.
#

import sys
from os import path
import subprocess


def parse_entry( line ):
    split = line.split( '#' )
    line = split[ 0 ].strip( ' \t\n;' )
    if not line:
        return None
    if not line:
        return None
    return tuple( map( str.strip, line.split( ';' ) ) )

def parse_file( file ):
    entries = [ entry for entry in ( parse_entry( line ) for line in file ) if entry is not None ]
    return entries

ualtest = sys.argv[ 1 ]
test_path = sys.argv[ 2 ]
test_name = path.basename( test_path )

with open( test_path, 'r', encoding='utf-8' ) as file:
    test_cases = parse_file( file )


def run_ualtest( text, *args ):

    # Run command.
    command = [ ualtest ]
    command.extend( args )
    result = subprocess.run( command, input = text, stdout = subprocess.PIPE, stderr = subprocess.STDOUT )

    # Print test case and check for failure.
    output = result.stdout.decode( 'utf-8' )
    if result.returncode != 0:
        print( text.decode( 'utf-16-le', errors = 'replace' ) )
        print( "    FAILED:" )
        print( output )
        return None

    # Return output for checking.
    return output


def break_test( test_case, break_kind ):

    break_token = "BREAK_" + break_kind + " "

    offset = 0
    breaks = []
    string = []

    for token in test_case.split():
        if token == '÷':
            breaks.append( offset )
        elif token != '×':
            codepoint = int( token, 16 )
            string.append( chr( codepoint ) )
            offset += 1 if codepoint <= 0xFFFF else 2

    output = run_ualtest( ''.join( string ).encode( 'utf-16-le' ) )
    if output is None:
        return 1

    pstart = 0
    result = []
    for line in output.splitlines():
        if line.startswith( "PARAGRAPH " ):
            pstart = int( line.split()[ 1 ] )
            if pstart != 0 and break_kind == "LINE":
                result.append( pstart )
        if line.startswith( break_token ):
            result.append( pstart + int( line.split()[ 1 ] ) )
    result.append( offset )

    if breaks != result:
        print( test_case )
        print( breaks )
        print( output )
        print( result )
        return 1


def grapheme_break_test( test_cases ):

    for test_case, in test_cases:
        if break_test( test_case, "CLUSTER" ):
            return 1

    return 0


def line_break_test( test_cases ):

    for test_case, in test_cases:
        if break_test( test_case, "LINE" ):
            return 1

    return 0

def bidi_character_test( test_cases ):

    index = 0

    for codepoints, paragraph_direction, paragraph_level, levels, visual_order in test_cases:

        # Get test case in terms of integers/string.
        cpoint = [ int( x.strip(), 16 ) for x in codepoints.split() ]
        string = [ chr( x ) for x in cpoint ]
        paragraph_direction = int( paragraph_direction )
        paragraph_level = int( paragraph_level )
        levels = [ int( x ) if x != 'x' else -1 for x in levels.split() ]

        # Removed characters are BN in our result and bind with left run.
        if False:
            bn_level = paragraph_level
            for i in range( len( levels ) ):
                if levels[ i ] == -1:
                    levels[ i ] = bn_level
                else:
                    bn_level = levels[ i ]

        # Run test case.
        args = [ 'f' ]
        if paragraph_direction != 2:
            args.append( str( paragraph_direction ) )

        output = run_ualtest( ''.join( string ).encode( 'utf-16-le' ), *args )
        if output is None:
            return 1

        # Parse output, turning bidi runs into a list of per-character levels.
        pstart = 0
        result = []
        result_paragraph_level = -1

        for line in output.splitlines():
            if line.startswith( "PARAGRAPH " ):
                pstart = int( line.split()[ 1 ] )
            if line.startswith( "PARAGRAPH_LEVEL " ):
                result_paragraph_level = int( line.split()[ 1 ] )
            if line.startswith( "BIDI_RUN " ):
                _, lower, upper, level = line.split()
                lower = int( lower )
                upper = int( upper )
                level = int( level )
                result.extend( ( -2 for x in range( len( result ), upper ) ) )
                for i in range( lower, upper ):
                    result[ i ] = level

        # Check result.
        if paragraph_level != result_paragraph_level or levels != result:
            print( string )
            print( paragraph_level )
            print( result_paragraph_level )
            print( levels )
            print( result )
            print( output )
            return 1

        # There are a lot of test cases.  Indicate that we're still going.
        print( "PASSED", index )
        index += 1

    return 0


if test_name.lower() == 'graphemebreaktest.txt':
    sys.exit( grapheme_break_test( test_cases ) )
elif test_name.lower() == 'linebreaktest.txt':
    sys.exit( line_break_test( test_cases ) )
elif test_name.lower() == 'bidicharactertest.txt':
    sys.exit( bidi_character_test( test_cases ) )



print( "Unknown test file", file = sys.stderr )
sys.exit( 1 )



