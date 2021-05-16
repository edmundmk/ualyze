//
//  ualtest.cpp
//
//  Created by Edmund Kapusniak on 15/12/2019.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the ISC License. See LICENSE file in the project root for
//  full license information.
//

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <ualyze.h>
#include "../source/ual_buffer.h"

void bidi_initial( ual_buffer* ub, unsigned override_paragraph_level );
void bidi_weak( ual_buffer* ub );
void bidi_brackets( ual_buffer* ub );
void bidi_neutral( ual_buffer* ub );

static char boundary_class( unsigned bc )
{
    switch ( bc )
    {
    case UCDB_BIDI_L:   return 'L';
    case UCDB_BIDI_R:   return 'R';
    case UCDB_BIDI_AL:  return 'A';
    case BC_SEQUENCE:   return '=';
    default:            return '?';
    }
}

static const char* bidi_class( unsigned bc )
{
    switch ( bc )
    {
    case UCDB_BIDI_L:   return "L";
    case UCDB_BIDI_R:   return "R";
    case UCDB_BIDI_AL:  return "AL";
    case UCDB_BIDI_LRE: return "LRE";
    case UCDB_BIDI_LRO: return "LRO";
    case UCDB_BIDI_RLE: return "RLE";
    case UCDB_BIDI_RLO: return "RLO";
    case UCDB_BIDI_PDF: return "PDF";
    case UCDB_BIDI_EN:  return "EN";
    case UCDB_BIDI_ES:  return "ES";
    case UCDB_BIDI_ET:  return "ET";
    case UCDB_BIDI_AN:  return "AN";
    case UCDB_BIDI_CS:  return "CS";
    case UCDB_BIDI_NSM: return "NSM";
    case UCDB_BIDI_BN:  return "BN";
    case UCDB_BIDI_B:   return "B";
    case UCDB_BIDI_S:   return "S";
    case UCDB_BIDI_WS:  return "WS";
    case UCDB_BIDI_ON:  return "ON";
    case UCDB_BIDI_LRI: return "LRI";
    case UCDB_BIDI_RLI: return "RLI";
    case UCDB_BIDI_FSI: return "FSI";
    case UCDB_BIDI_PDI: return "PDI";
    case BC_WS_R:       return "WSR";
    case BC_BRACKET:    return "BRK";
    case BC_INVALID:    return "X";
    default:            return "?";
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
    enum { LEVEL_RUNS, EXPLICIT, WEAK, NEUTRAL, FULL } bidi_mode = FULL;
    if ( argc > 1 )
    {
        const char* arg = argv[ 1 ];
        if ( strcmp( arg, "l" ) == 0 ) bidi_mode = LEVEL_RUNS;
        if ( strcmp( arg, "x" ) == 0 ) bidi_mode = EXPLICIT;
        if ( strcmp( arg, "w" ) == 0 ) bidi_mode = WEAK;
        if ( strcmp( arg, "n" ) == 0 ) bidi_mode = NEUTRAL;
        if ( strcmp( arg, "f" ) == 0 ) bidi_mode = FULL;
    }

    // Get paragraph override level.
    unsigned override_paragraph_level = UAL_FROM_TEXT;
    if ( argc > 2 )
    {
        override_paragraph_level = atoi( argv[ 2 ] );
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
        if ( bidi_mode != FULL )
        {
            bidi_initial( ub, override_paragraph_level );
            size_t lrun_length = ub->level_runs.size() - 1;
            for ( size_t irun = 0; irun < lrun_length; ++irun )
            {
                const ual_level_run* prun = &ub->level_runs.at( irun );
                const ual_level_run* nrun = &ub->level_runs.at( irun + 1 );
                printf
                (
                    "LEVEL_RUN %u:%u:%c%c %u %u\n",
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

            if ( ub->bidi_analysis.complexity != BIDI_ALL_LEFT )
            {
                if ( bidi_mode != EXPLICIT )
                {
                    bidi_weak( ub );
                    if ( bidi_mode != WEAK )
                    {
                        bidi_brackets( ub );
                        bidi_neutral( ub );
                    }
                }
            }

            printf( "BIDI_CLASS" );
            size_t length = ub->c.size();
            for ( size_t i = 0; i < length; ++i )
            {
                const ual_char& c = ub->c[ i ];
                printf( " %s", bidi_class( c.bc ) );
            }
            printf( "\n" );
            continue;
        }
        else if ( bidi_mode == FULL )
        {
            unsigned paragraph_level = ual_analyze_bidi( ub, override_paragraph_level );
            printf( "PARAGRAPH_LEVEL %u\n", paragraph_level );

            ual_bidi_run run;
            ual_bidi_runs_begin( ub );
            while ( ual_bidi_runs_next( ub, &run ) )
            {
                printf( "BIDI_RUN %zu %zu %u\n", run.lower, run.upper, run.level );
            }
            ual_bidi_runs_end( ub );
        }
    }

    ual_buffer_release( ub );

    // Complete.
    return EXIT_SUCCESS;
}

