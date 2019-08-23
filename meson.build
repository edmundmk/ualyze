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
    'source/ual_buffer.cpp',
    'source/ual_paragraph.cpp',
    'ucdn/ucdn.c'
]

ualyze = library(
    'ualyze',
    sources : sources,
    include_directories : include_directories( 'include/ualyze', 'ucdn' ),
    install : true
)

install_headers( headers, subdir : 'ualyze' )

