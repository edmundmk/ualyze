#
#  ualyze
#
#  Created by Edmund Kapusniak on 22/08/2019.
#
#  Licensed under the MIT License. See LICENSE file in the project root for
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
    'ucdu/ucdu.cpp',
]

cpp_args = [ '-DUAL_BUILD' ]

break_machine = generator( find_program( 'source/break_machine.py' ), output : '@BASENAME@.h', arguments : [ '@INPUT@', '@OUTPUT@' ] )
sources += break_machine.process( 'source/uax14.rules', 'source/uax29p3.rules' )

ualyze_lib = library( 'ualyze', sources : sources, include_directories : include_directories( 'include', 'ucdu' ), cpp_args : cpp_args, gnu_symbol_visibility : 'hidden', install : true )
ualyze_lic = files( 'LICENSE' )
ualyze_dep = declare_dependency( include_directories : include, link_with : ualyze_lib )
install_headers( 'include/ualyze.h' )

ualtest = executable( 'ualtest', sources : sources + [ 'tests/ualtest.cpp' ], cpp_args : [ '-DUAL_BUILD_TESTS' ], include_directories : include_directories( 'include', 'ucdu' ) )
test_script = find_program( 'tests/ualtest.py' )
test( 'paragraph.test', test_script, args : [ ualtest.full_path(), files( 'tests/paragraph.test' ) ] )
test( 'linebreak.test', test_script, args : [ ualtest.full_path(), files( 'tests/linebreak.test' ) ] )
test( 'clusterbreak.test', test_script, args : [ ualtest.full_path(), files( 'tests/linebreak.test' ) ] )
test( 'script.test', test_script, args : [ ualtest.full_path(), files( 'tests/script.test' ) ] )
test( 'bidi.test', test_script, args : [ ualtest.full_path(), files( 'tests/bidi.test' ) ] )
test_script = find_program( 'tests/ucdtest.py' )
test( 'GraphemeBreakTest.txt', test_script, args : [ ualtest.full_path(), files( 'tests/GraphemeBreakTest.txt' ) ] )
test( 'LineBreakTest.txt', test_script, args : [ ualtest.full_path(), files( 'tests/LineBreakTest.txt' ) ] )
