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

# Read file, stripping comments.
lines = []
with open( sys.argv[ 1 ], 'r' ) as f:
    for line in f:
        comment_index = line.find( '--' )
        if comment_index != -1:
            line = line[:comment_index]
    lines.append( line )




