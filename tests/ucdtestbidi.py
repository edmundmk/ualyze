#!/usr/bin/env python3
#
#   ucdtest.py
#
#   Created by Edmund Kapusniak on 15/07/2023.
#   Copyright © 2023 Edmund Kapusniak.
#
#   Licensed under the ISC License. See LICENSE file in the project root for
#   full license information.
#

import os
import sys
import subprocess


BIDI_CHARS = {
    'L'     : 'A',
    'R'     : '\u05D8',
    'AL'    : '\u0628',
    'AN'    : '\u0667',
    'B'     : '\u000A',
    'BN'    : '\u200B',
    'CS'    : ',',
    'EN'    : '0',
    'ES'    : '+',
    'ET'    : '$',
    'FSI'   : '\u2068',
    'LRE'   : '\u202A',
    'LRI'   : '\u2066',
    'LRO'   : '\u202D',
    'NSM'   : '\u0300',
    'ON'    : '!',
    'PDF'   : '\u202C',
    'PDI'   : '\u2069',
    'RLE'   : '\u202B',
    'RLI'   : '\u2067',
    'RLO'   : '\u202E',
    'S'     : '\t',
    'WS'    : ' ',
}


ualtestbidi = sys.argv[ 1 ]
test_path = sys.argv[ 2 ]
test_name = os.path.basename( test_path )


def parse_biditest( file ):
    test_cases = []

    levels = ''
    reorder = ''

    for line in file:
        split = line.split( '#' )
        sline = split[ 0 ].strip( ' \t\n;' )
        if not sline:
            continue

        if sline.startswith( '@Levels:' ):
            levels = sline[ 8 : ].strip()
            continue

        if sline.startswith( '@Reorder:' ):
            reorder = sline[ 9 : ].strip()
            continue

        split = sline.split( ';' )

        case = split[ 0 ].strip()
        text = ''.join( [ BIDI_CHARS[ code ] for code in case.split() ] )

        overrides = int( split[ 1 ].strip() )
        if ( overrides & 1 ) != 0:
            test_cases.append( ( case, text, 2, None, levels, reorder ) )
        if ( overrides & 2 ) != 0:
            test_cases.append( ( case, text, 0, None, levels, reorder ) )
        if ( overrides & 4 ) != 0:
            test_cases.append( ( case, text, 1, None, levels, reorder ) )

    return test_cases


def parse_bidicharactertest( file ):
    test_cases = []

    for line in file:
        split = line.split( '#' )
        sline = split[ 0 ].strip( ' \t\n;' )
        if not sline:
            continue

        split = sline.split( ';' )

        case = split[ 0 ].strip()
        text = ''.join( [ chr( int( code, 16 ) ) for code in case.split() ] )

        test_cases.append( ( case, text, int( split[ 1 ] ), int( split[ 2 ] ), split[ 3 ].strip(), split[ 4 ].strip() ) )

    return test_cases


with open( test_path, 'r', encoding='utf-8' ) as file:
    if test_name.lower() == 'biditest.txt':
        test_cases = parse_biditest( file )
    elif test_name.lower() == 'bidicharactertest.txt':
        test_cases = parse_bidicharactertest( file )

if len( sys.argv ) > 3:
    lower = min( int( sys.argv[ 3 ] ), len( test_cases ) )
    upper = min( int( sys.argv[ 4 ] ), len( test_cases ) )
    test_cases = test_cases[ lower : upper ]


def run_ualtestbidi( text, *args ):

    text = text.encode( 'utf-16-le' )

    # Run command.
    command = [ ualtestbidi ]
    command.extend( args )
    result = subprocess.run( command, input = text, stdout = subprocess.PIPE, stderr = subprocess.STDOUT )
    output = result.stdout.decode( 'utf-8' )

    levels = ''
    reorder = ''

    for sline in output.split( '\n' ):

        if sline.startswith( '@Paragraph:' ):
            paragraph = int( sline[ 11 : ].strip() )
            continue

        if sline.startswith( '@Levels:' ):
            levels = sline[ 8 : ].strip()
            continue

        if sline.startswith( '@Reorder:' ):
            reorder = sline[ 9 : ].strip()
            continue

    return ( output, paragraph, levels, reorder )


total = 0
countdown = 1000

for case in test_cases:

    if case[ 2 ] == 2:
        result = run_ualtestbidi( case[ 1 ] )
    else:
        result = run_ualtestbidi( case[ 1 ], str( case[ 2 ] ) )

    check = True
    if case[ 3 ] != None and case[ 3 ] != result[ 1 ]:
        check = False
    if case[ 4 ] != result[ 2 ]:
        check = False
    if case[ 5 ] != result[ 3 ]:
        check = False

    if not check:
        print( f'TEST {total}' )
        print( f'\tText: { case[ 0 ] }' )
        print( f'\tOverride: { case[ 2 ] }' )
        if case[ 3 ] != None:
            print( f'\tParagraph: expected: { case[ 3 ] } result: { result[ 1 ] }' )
        print( f'\tLevels (expected):  \'{ case[ 4 ] }\'' )
        print( f'\tLevels (result):    \'{ result[ 2 ] }\'' )
        print( f'\tReorder (expected): \'{ case[ 5 ] }\'' )
        print( f'\tReorder (result):   \'{ result[ 3 ] }\'' )

    if not check:
        print( '\t!!!! FAILED !!!!' )
        sys.exit( 1 )

    total += 1

    """
    countdown -= 1
    if countdown <= 0:
        print( "total", total )
        countdown = 1000
    """
