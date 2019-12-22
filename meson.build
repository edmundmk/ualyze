#
#  ualyze
#
#  Created by Edmund Kapusniak on 22/08/2019.
#
#  Licensed under the MIT License. See LICENSE file in the project root for
#  full license information.
#

project( 'ualyze', 'cpp', 'c', default_options : [ 'cpp_std=c++11' ] )

headers = [
    'include/ualyze/ual_bidi.h',
    'include/ualyze/ual_break.h',
    'include/ualyze/ual_buffer.h',
    'include/ualyze/ual_script.h'
]

sources = [
    'source/ual_bidi_paragraph_level.cpp',
    'source/ual_break.cpp',
    'source/ual_buffer.cpp',
    'source/ual_paragraph.cpp',
    'ucdn/ucdn.c'
]

break_machine = generator(
    find_program( 'source/break_machine.py' ),
    output : '@BASENAME@.h',
    arguments : [ '@INPUT@', '@OUTPUT@' ]
)

sources += break_machine.process( 'source/uax14.rules', 'source/uax29p3.rules' )

ualyze = library(
    'ualyze',
    sources : sources,
    include_directories : include_directories( 'include/ualyze', 'ucdn' ),
    install : true
)

install_headers( headers, subdir : 'ualyze' )

ualtest = executable(
    'ualtest',
    sources : [ 'tests/ualtest.cpp' ],
    include_directories : include_directories( 'include' ),
    link_with : ualyze
)

