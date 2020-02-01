#!/usr/bin/env python3
#
#  ualtest.py
#
#  Created by Edmund Kapusniak on 15/12/2019.
#  Copyright (c) 2019 Edmund Kapusniak.
#
#  Licensed under the MIT License. See LICENSE file in the project root for
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
            chars[ chardef[ 1 ] ] = bytes.fromhex( chardef[ 2 ] )[ : : -1 ]
            continue

        # Check for new test case
        if line == "SCRIPT":
            cases.append( [ line, "" ] )
        else:
            cases[ -1 ][ 1 ] += line

# Perform cases.
for case in cases:

    kind = case[ 0 ]
    code = case[ 1 ]

    text = bytearray()
    p = [ [ [ 0, 0 ] ] ]
    index = 0

    i = 0
    while i < len( code ):
        c = code[ i ]
        i += 1

        if c == '[':
            j = code.find( ']', i )
            if j != -1:
                p[ -1 ][ -1 ][ -1 ] = index
                p[ -1 ].append( [ code[ i : j ], index, index ] )
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
                p[ -1 ][ -1 ][ -1 ] = index
                upper = len( text ) // 2
                p[ -1 ][ 0 ][ -1 ] = upper
                p.append( [ [ upper, upper ] ] )
                index = 0
                i += 1
                continue

        elif c == ' ':
            continue

        raise Exception( 'invalid test case code' )

    p[ -1 ][ -1 ][ -1 ] = index
    p[ -1 ][ 0 ][ -1 ] = len( text ) // 2

    # Remove empty paragraph
    if len( p ) == 1 and len( p[ 0 ] ) == 1:
        p.pop()

    # Invoke test case.
    result = subprocess.run( ualtest, input = text, stdout = subprocess.PIPE, stderr = subprocess.STDOUT )

    # Print test case and check for failure.
    output = result.stdout.decode( 'utf-8' )
    if result.returncode != 0:
        print( kind, text.decode( 'utf-16-le', errors = 'replace' ) )
        print( "    FAILED:", output, end="" )
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

        print( info )

    if p != q:
        print( kind, text.decode( 'utf-16-le', errors = 'replace' ) )
        print( "   FAILED:" )
        print( p )
        print( q )
        exitcode = 1
        continue


sys.exit( exitcode )

