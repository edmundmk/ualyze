//
//  ualtest.cpp
//
//  Created by Edmund Kapusniak on 15/12/2019.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
//

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <ualyze.h>
#include "../source/ual_buffer.h"

void bidi_initial( ual_buffer* ub );
void bidi_weak( ual_buffer* ub );
void bidi_brackets( ual_buffer* ub );
void bidi_neutral( ual_buffer* ub );

static char boundary_class( unsigned bc )
{
    switch ( bc )
    {
    case UCDN_BIDI_CLASS_L:     return 'L';
    case UCDN_BIDI_CLASS_R:     return 'R';
    case UCDN_BIDI_CLASS_AL:    return 'A';
    case BC_SEQUENCE:           return '=';
    default:                    return '?';
    }
}

static const char* bidi_class( unsigned bc )
{
    switch ( bc )
    {
    case UCDN_BIDI_CLASS_L:     return "L";
    case UCDN_BIDI_CLASS_R:     return "R";
    case UCDN_BIDI_CLASS_AL:    return "AL";
    case UCDN_BIDI_CLASS_LRE:   return "LRE";
    case UCDN_BIDI_CLASS_LRO:   return "LRO";
    case UCDN_BIDI_CLASS_RLE:   return "RLE";
    case UCDN_BIDI_CLASS_RLO:   return "RLO";
    case UCDN_BIDI_CLASS_PDF:   return "PDF";
    case UCDN_BIDI_CLASS_EN:    return "EN";
    case UCDN_BIDI_CLASS_ES:    return "ES";
    case UCDN_BIDI_CLASS_ET:    return "ET";
    case UCDN_BIDI_CLASS_AN:    return "AN";
    case UCDN_BIDI_CLASS_CS:    return "CS";
    case UCDN_BIDI_CLASS_NSM:   return "NSM";
    case UCDN_BIDI_CLASS_BN:    return "BN";
    case UCDN_BIDI_CLASS_B:     return "B";
    case UCDN_BIDI_CLASS_S:     return "S";
    case UCDN_BIDI_CLASS_WS:    return "WS";
    case UCDN_BIDI_CLASS_ON:    return "ON";
    case UCDN_BIDI_CLASS_LRI:   return "LRI";
    case UCDN_BIDI_CLASS_RLI:   return "RLI";
    case UCDN_BIDI_CLASS_FSI:   return "FSI";
    case UCDN_BIDI_CLASS_PDI:   return "PDI";
    case BC_INVALID:            return "X";
    default:                    return "?";
    }
}

int main( int argc, char* argv[] )
{
    // Reopen stdin and stdout.
#ifdef _WIN32
    _setmode( _fileno( stdin ), _O_BINARY );
    _setmode( _fileno( stdout ), _O_BINARY );
#endif

    // Check for bidi argument.
    enum { LEVEL_RUNS, EXPLICIT, WEAK, WEAK_R, NEUTRAL, NONE } bidi_mode = NONE;
    if ( argc > 1 )
    {
        const char* arg = argv[ 1 ];
        if ( strcmp( arg, "l" ) == 0 ) bidi_mode = LEVEL_RUNS;
        if ( strcmp( arg, "x" ) == 0 ) bidi_mode = EXPLICIT;
        if ( strcmp( arg, "w" ) == 0 ) bidi_mode = WEAK;
        if ( strcmp( arg, "wr" ) == 0 ) bidi_mode = WEAK_R;
        if ( strcmp( arg, "n" ) == 0 ) bidi_mode = NEUTRAL;
    }

    // Read in text from stdin.
    std::vector< char > data;
    while ( true )
    {
        int c = fgetc( stdin );
        if ( c == EOF ) break;
        data.push_back( c );
    }

    if ( data.size() % 2 )
    {
        fprintf( stderr, "incomplete UTF-16 input\n" );
        return EXIT_FAILURE;
    }


    // Create buffer.
    ual_buffer* ub = ual_buffer_create();

    // Process paragraph-by-paragraph.
    size_t plower = 0;
    std::u16string_view text( (const char16_t*)data.data(), data.size() / 2 );
    while ( size_t length = ual_analyze_paragraph( ub, text.data() + plower, text.size() - plower ) )
    {
        // Print paragraph.
        printf( "PARAGRAPH %zu %zu\n", plower, plower + length );
        plower += length;

        // Analyze breaks.
        ual_analyze_breaks( ub );
        const ual_char* c = ual_buffer_chars( ub );
        size_t count = ual_buffer_size( ub );
        for ( size_t index = 0; index < count; ++index )
        {
            if ( c[ index ].bc & UAL_BREAK_CLUSTER )
            {
                printf( "BREAK_CLUSTER %zu\n", index );
            }
            if ( c[ index ].bc & UAL_BREAK_LINE )
            {
                printf( "BREAK_LINE %zu\n", index );
            }
            if ( c[ index ].bc & UAL_BREAK_SPACES )
            {
                printf( "BREAK_SPACES %zu\n", index );
            }
        }

        // Analyze script.
        ual_script_span span;
        ual_script_spans_begin( ub );
        while ( ual_script_spans_next( ub, &span ) )
        {
            printf
            (
                "SCRIPT %c%c%c%c %zu %zu\n",
                ( span.script >> 24 ) & 0xFF,
                ( span.script >> 16 ) & 0xFF,
                ( span.script >> 8  ) & 0xFF,
                ( span.script       ) & 0xFF,
                span.lower,
                span.upper
            );
        }
        ual_script_spans_end( ub );

        // Analyze bidi stages.
        if ( bidi_mode != NONE )
        {
            bidi_initial( ub );
            size_t lrun_length = ub->level_runs.size() - 1;
            for ( size_t irun = 0; irun < lrun_length; ++irun )
            {
                const ual_level_run* prun = &ub->level_runs.at( irun );
                const ual_level_run* nrun = &ub->level_runs.at( irun + 1 );
                printf
                (
                    "LRUN %u:%u:%c%c %u %u\n",
                    prun->level,
                    prun->inext,
                    boundary_class( prun->sos ),
                    boundary_class( prun->eos ),
                    prun->start,
                    nrun->start
                );
            }
            if ( bidi_mode == LEVEL_RUNS )
            {
                continue;
            }

            if ( bidi_mode == WEAK_R )
            {
                // Force a single right-to-left level run.
                ub->level_runs.clear();
                ub->level_runs.push_back( { 0, 1, UCDN_BIDI_CLASS_R, UCDN_BIDI_CLASS_R, 0 } );
                ub->level_runs.push_back( { (unsigned)ub->c.size(), 1, BC_SEQUENCE, BC_SEQUENCE, 0 } );
                ub->bidi_analysis.complexity = BIDI_SOLITARY;
            }

            if ( ub->bidi_analysis.complexity != BIDI_ALL_LEFT )
            {
                if ( bidi_mode != EXPLICIT )
                {
                    bidi_weak( ub );
                    if ( bidi_mode != WEAK && bidi_mode != WEAK_R )
                    {
                        bidi_brackets( ub );
                        bidi_neutral( ub );
                    }
                }
            }

            printf( "BCLASS" );
            size_t length = ub->c.size();
            for ( size_t i = 0; i < length; ++i )
            {
                const ual_char& c = ub->c[ i ];
                printf( " %s", bidi_class( c.bc ) );
            }
            printf( "\n" );
            continue;
        }
    }

    ual_buffer_release( ub );

    // Complete.
    return EXIT_SUCCESS;
}

