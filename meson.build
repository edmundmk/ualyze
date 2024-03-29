#
#  ualyze
#
#  Created by Edmund Kapusniak on 22/08/2019.
#
#  Licensed under the ISC License. See LICENSE file in the project root for
#  full license information.
#

project( 'ualyze', 'cpp', default_options : [ 'cpp_std=c++17' ] )

include = include_directories( 'include' )

sources = [
    'source/ual_bidi.cpp',
    'source/ual_break.cpp',
    'source/ual_buffer.cpp',
    'source/ual_paragraph.cpp',
    'source/ual_script.cpp',
    'ucdb/ucdb_bracket.cpp',
    'ucdb/ucdb_script.cpp',
    'ucdb/ucdb_table.cpp',
]

cpp_args = [ '-DUAL_BUILD' ]

break_machine = generator( find_program( 'source/break_machine.py' ), output : '@BASENAME@.h', arguments : [ '@INPUT@', '@OUTPUT@' ] )
sources += break_machine.process( 'source/uax14.rules', 'source/uax29p3.rules' )

ualyze_lib = library( 'ualyze', sources : sources, include_directories : include_directories( 'include', 'ucdb' ), cpp_args : cpp_args, gnu_symbol_visibility : 'hidden', install : true )
ualyze_lic = files( 'LICENSE' )
ualyze_dep = declare_dependency( include_directories : include, link_with : ualyze_lib )
install_headers( 'include/ualyze.h' )

testbidi = executable( 'testbidi', sources : sources + [ 'tests/testbidi.cpp' ], include_directories : include_directories( 'include', 'ucdb' ) )
testcase = executable( 'testcase', sources : sources + [ 'tests/testcase.cpp' ], cpp_args : [ '-DUAL_BUILD_TESTS' ], include_directories : include_directories( 'include', 'ucdb' ) )
testfuzz = executable( 'testfuzz', sources : sources + [ 'tests/testfuzz.cpp' ], include_directories : include_directories( 'include', 'ucdb' ) )
subdir( 'tests' )
