//
//  testbidi.cpp
//
//  Created by Edmund Kapusniak on 15/07/2023.
//  Copyright © 2023 Edmund Kapusniak.
//
//  Licensed under the ISC License. See LICENSE file in the project root for
//  full license information.
//

#include <stdlib.h>
#include <stdio.h>
#include <ualyze.h>
#include <vector>
#include <string_view>
#include "generated/table_enum.h"

int main( int argc, char* argv[] )
{
    // Reopen stdin and stdout.
#ifdef _WIN32
    _setmode( _fileno( stdin ), _O_BINARY );
    _setmode( _fileno( stdout ), _O_BINARY );
#endif

    // Get paragraph override level.
    unsigned override_paragraph_level = UAL_FROM_TEXT;
    if ( argc > 1 )
    {
        override_paragraph_level = atoi( argv[ 1 ] );
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
    std::u16string_view text( (const char16_t*)data.data(), data.size() / 2 );
    ual_bidi_run run;

    ual_analyze_paragraph( ub, text.data(), text.size() );

    // Levels.
    unsigned paragraph_level = ual_analyze_bidi( ub, override_paragraph_level );
    printf( "@Paragraph:\t%u\n", paragraph_level );

    const ual_char* b = ual_buffer_chars( ub );

    ual_bidi_runs_begin( ub );
    while ( ual_bidi_runs_next( ub, &run ) )
    {
        printf( "RUN: %zu..%zu %d\n", run.lower, run.upper, run.level );
    }
    ual_bidi_runs_end( ub );

    std::vector< ual_bidi_run > runs;
    unsigned high_level = 0;
    unsigned lodd_level = UINT_MAX;

    printf( "@Levels:\t" );
    ual_bidi_runs_begin( ub );
    while ( ual_bidi_runs_next( ub, &run ) )
    {
        runs.push_back( run );
        if ( run.level > high_level ) high_level = run.level;
        if ( ( run.level & 1 ) != 0 && run.level < lodd_level ) lodd_level = run.level;

        for ( size_t index = run.lower; index < run.upper; ++index )
        {
            if ( b[ index ].bc != UCDB_BIDI_BN )
            {
                printf( "%u ", run.level );
            }
            else
            {
                printf( "x " );
            }
        }
    }
    ual_bidi_runs_end( ub );
    printf( "\n" );

    if ( lodd_level != UINT_MAX )
    {
        for ( unsigned level = high_level; level >= lodd_level; --level )
        {
            size_t left_swap = ~0;
            for ( size_t index = 0; index <= runs.size(); ++index )
            {
                if ( index < runs.size() && runs[ index ].level >= level )
                {
                    if ( left_swap == ~0 )
                    {
                        left_swap = index;
                    }
                }
                else
                {
                    if ( left_swap != ~0 )
                    {
                        std::reverse( runs.begin() + left_swap, runs.begin() + index );
                        left_swap = ~0;
                    }
                }
            }
        }
    }

    printf( "@Reorder:\t" );
    for ( const ual_bidi_run& run : runs )
    {
        if ( ( run.level & 1 ) == 0 )
        {
            for ( size_t index = run.lower; index < run.upper; ++index )
            {
                if ( b[ index ].bc != UCDB_BIDI_BN )
                {
                    printf( "%zu ", index );
                }
            }
        }
        else
        {
            size_t index = run.upper;
            while ( index-- > run.lower )
            {
                if ( b[ index ].bc != UCDB_BIDI_BN )
                {
                    printf( "%zu ", index );
                }
            }
        }
    }
    printf( "\n" );

    // Complete.
    return EXIT_SUCCESS;
}
