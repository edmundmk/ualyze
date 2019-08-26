#
#  break_machine.py
#
#  Created by Edmund Kapusniak on 26/08/2019.
#  Copyright © 2019 Edmund Kapusniak.
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



