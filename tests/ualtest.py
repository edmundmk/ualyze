#!/usr/bin/env python3
#
#  ualtest.py
#
#  Created by Edmund Kapusniak on 15/12/2019.
#  Copyright (c) 2019 Edmund Kapusniak.
#
#  Licensed under the ISC License. See LICENSE file in the project root for
#  full license information.
#

import sys
import subprocess

ualtest = sys.argv[ 1 ]
exitcode = 0

# Read file with test cases.
chars = {}
cases = []
with open( sys.argv[ 2 ], 'r' ) as f:
    for line in f:

        # Strip comment.
        comment_index = line.find( '--' )
        if comment_index != -1:
            line = line[:comment_index]

        # Strip whitespace.
        line = line.strip()

        # Check for character.
        if line.startswith( "CHAR" ):
            chardef = line.split()
            code = chr( int( chardef[ 2 ], 16 ) ).encode( 'utf-16-le', errors = 'surrogatepass' )
            chars[ chardef[ 1 ] ] = code
            continue

        # Check for new test case
        if line == "SCRIPT" or line == "PARAGRAPH" or line == "LINEBREAK" or line == "CLUSTERBREAK":
            cases.append( [ line, None, "" ] )
        elif line == "BIDILRUN":
            cases.append( [ line, "l", "" ] )
        elif line == "BIDIEXPLICIT":
            cases.append( [ line, "x", "" ] )
        elif line == "BIDIWEAK":
            cases.append( [ line, "w", "" ] )
        elif line == "BIDIWEAKR":
            cases.append( [ line, "wr", "" ] )
        elif line == "BIDINEUTRAL":
            cases.append( [ line, "n", "" ] )
        elif line != "":
            cases[ -1 ][ 2 ] += line

# Perform cases.
for case in cases:

    kind = case[ 0 ]
    code = case[ 2 ]

    text = bytearray()
    p = [ [ [ 0, -1 ] ] ]
    index = 0

    i = 0
    while i < len( code ):
        c = code[ i ]
        i += 1

        if c == '[':
            j = code.find( ']', i )
            if j != -1:
                if p[ -1 ][ -1 ][ -1 ] == -1:
                    p[ -1 ][ -1 ][ -1 ] = index
                p[ -1 ].append( [ code[ i : j ], index, -1 ] )
                i = j + 1
                continue

        elif c == '"':
            j = code.find( '"', i )
            if j != -1:
                utf16 = code[ i : j ].encode( 'utf-16-le' )
                text += utf16
                index += len( utf16 ) // 2
                i = j + 1
                continue

        elif c == '<':
            j = code.find( '>', i )
            if j != -1:
                char = chars[ code[ i : j ] ]
                text += char
                index += len( char ) // 2
                i = j + 1
                continue

        elif c == '/':
            if code[ i ] == '/':
                if p[ -1 ][ -1 ][ -1 ] == -1:
                    p[ -1 ][ -1 ][ -1 ] = index
                upper = len( text ) // 2
                p[ -1 ][ 0 ][ -1 ] = upper
                p.append( [ [ upper, upper ] ] )
                index = 0
                i += 1
                continue

        elif c == '(':
            j = code.find( ')', i )
            if j != -1:
                p[ -1 ].append( [ "BIDI_CLASS" ] + code[ i : j ].split() )
                i = j + 1
                continue

        elif c == ',':
            p[ -1 ].append( [ "BREAK_CLUSTER", index ] )
            continue

        elif c == ':':
            p[ -1 ].append( [ "BREAK_SPACES", index ] )
            continue

        elif c == ';':
            p[ -1 ].append( [ "BREAK_LINE", index ] )
            continue

        elif c == ' ':
            continue

        print( "'", code, "'", sep = "" )
        raise Exception( 'invalid test case code' )

    if p[ -1 ][ -1 ][ -1 ] == -1:
        p[ -1 ][ -1 ][ -1 ] = index
    p[ -1 ][ 0 ][ -1 ] = len( text ) // 2

    # Remove empty paragraph
    if p == [ [ [ 0, 0 ] ] ]:
        p.pop()

    # Invoke test case.
    command = [ ualtest ]
    if case[ 1 ] != None:
        if case[ 1 ] == 'wr':
            command.extend( [ 'w', '1' ] )
        else:
            command.append( case[ 1 ] )
    result = subprocess.run( command, input = text, stdout = subprocess.PIPE, stderr = subprocess.STDOUT )

    # Print test case and check for failure.
    output = result.stdout.decode( 'utf-8' )
    if result.returncode != 0:
        print( kind, text.decode( 'utf-16-le', errors = 'replace' ) )
        print( "    FAILED:" )
        print( output )
        exitcode = result.returncode
        continue

    # Decode results from analysis.
    q = []
    for line in output.splitlines():
        info = line.split()

        if info[ 0 ] == 'PARAGRAPH':
            q.append( [ [ int( info[ 1 ] ), int( info[ 2 ] ) ] ] )
            continue

        if kind == 'SCRIPT' and info[ 0 ] == 'SCRIPT':
            q[ -1 ].append( [ info[ 1 ], int( info[ 2 ] ), int( info[ 3 ] ) ] )
            continue

        if kind == 'CLUSTERBREAK' and info[ 0 ] == 'BREAK_CLUSTER':
            q[ -1 ].append( [ info[ 0 ], int( info[ 1 ] ) ] )
            continue

        if kind == 'LINEBREAK' and ( info[ 0 ] == 'BREAK_SPACES' or info[ 0 ] == 'BREAK_LINE' ):
            q[ -1 ].append( [ info[ 0 ], int( info[ 1 ] ) ] )
            continue

        if kind == 'BIDILRUN' and info[ 0 ] == 'LEVEL_RUN':
            q[ -1 ].append( [ info[ 1 ], int( info[ 2 ] ), int( info[ 3 ] ) ] )

        if ( kind == 'BIDIEXPLICIT' or kind == 'BIDIWEAK' or kind == 'BIDIWEAKR' or kind == 'BIDINEUTRAL' ) and info[ 0 ] == 'BIDI_CLASS':
            q[ -1 ].append( info )

    if p != q:
        print( kind, text.decode( 'utf-16-le', errors = 'replace' ) )
        print( code )
        print( "   FAILED:" )
        print( output )
        print( p )
        print( q )
        exitcode = 1
        continue


sys.exit( exitcode )

