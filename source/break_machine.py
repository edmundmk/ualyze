#
#  break_machine.py
#
#  Created by Edmund Kapusniak on 26/08/2019.
#  Copyright (c) 2019 Edmund Kapusniak.
#
#  Licensed under the MIT License. See LICENSE file in the project root for
#  full license information.
#

#
#  Parse a .rules file and construct a simple state machine that implements
#  breaking rules.  Rules have the following forms:
#
#   A -> B
#       In all subsequent rules, mentions of B also include A on both the left
#       and right sides.
#
#   A B -> C
#       When in state A and matched B, go to state C.
#
#   A - B
#   A x B
#       When in state A and matched B, a break is allowed with - and prohibited
#       with x.  A or B (or both) may be omitted, in which case the rule
#       applies to all states/classes.  The first rule that matches applies.
#
#       Unless there is also a matching arrow rule, enter state B.
#

import sys

token = []
rules = []
rname = ''
start = ''

with open( sys.argv[ 1 ], 'rb' ) as f:
    for line in f:

        # Get rid of comment.
        comment_index = line.find( '--' )
        if comment_index != -1:
            line = line[:comment_index]

        # Split into tokens.
        tokens = line.split()

        # Ignore empty lines.
        if len( tokens ) == 0:
            continue

        # Build token list.
        if len( tokens ) == 1 and tokens[ 0 ] != '-' and tokens[ 0 ] != 'x':
            token.append( tokens[ 0 ] )
            continue

        # Retain rule names.
        if tokens[ -1 ] == ':':
            rname = tokens[ 0 ]
            continue

        # Check for start state.
        if tokens[ 0 ] == '->':
            start = tokens[ 1 ]
            continue

        if '->' in tokens:
            if len( tokens ) == 4:
                # A B -> C
                rules.append( ( rname, tokens[ 0 ], tokens[ 1 ], tokens[ 3 ] ) )
            else:
                # A -> B
                rules.append( ( rname, '->', tokens[ 0 ], tokens[ 2 ] ) )
        else:
            # Break/no-break rule.
            i = 0
            left = '.'
            if tokens[ i ] != '-' and tokens[ i ] != 'x':
                left = tokens[ i ]
                i += 1
            action = tokens[ i ]
            i += 1
            right = '.'
            if i < len( tokens ):
                right = tokens[ i ]
            rules.append( ( rname, left, right, action ) )

print( token )
for rule in rules:
    print( rule )

states = {}
state_list = []
build_stack = [ start ]
while len( build_stack ):
    build_state = build_stack.pop()
    if build_state in states:
        continue

    # Build this state by processing rules.
    state = build_state
    actions = []
    for shift in token:

        brk_action = '?'
        next_state = shift

        for rname, lhs, rhs, action in rules:

            if lhs == '->':
                action, lhs, rhs = lhs, rhs, action

                if state == lhs:
                    state = rhs
                if shift == lhs:
                    shift = rhs

                continue

            if lhs != '.' and lhs != state:
                continue

            if rhs != '.' and rhs != shift:
                continue

            if action == '-' or action == 'x':
                if rhs != '.' and rhs not in token:
                    print( "invalid token on rhs:", rhs )
                if brk_action == '?':
                    brk_action = action
            else:
                next_state = action

        actions.append( ( brk_action, next_state ) )
        build_stack.append( next_state )

    states[ build_state ] = actions
    state_list.append( ( [ build_state ], actions ) )

# Merge identical states.
states = {}

i = 0
while i < len( state_list ):
    slist, actions = state_list[ i ]

    j = i+1
    while j < len( state_list ):

        s, a = state_list[ j ]
        if a == actions:
            slist += s
            del state_list[ j ]
        else:
            j += 1

    for state in slist:
        states[ state ] = state_list[ i ]

    i += 1

i = 0
for state, actions in state_list:
    state.insert( 0, i )
    i += 1

for state, actions in state_list:
    print( state, actions )

