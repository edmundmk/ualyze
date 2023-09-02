//
//  testfuzz.cpp
//
//  Created by Edmund Kapusniak on 15/07/2023.
//  Copyright © 2023 Edmund Kapusniak.
//
//  Licensed under the ISC License. See LICENSE file in the project root for
//  full license information.
//

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <vector>
#include "ualyze.h"
#include "ucdb_table.h"

static const size_t MAX_SIZE = 1024 * 1024;
std::vector< char16_t > data;

int main( int argc, char* argv[] )
{
    srand(time(nullptr));

    ual_buffer* ub = ual_buffer_create();

    while ( true )
    {
#if 1
        size_t size = rand() % MAX_SIZE;
        data.resize( size );
        FILE* f = fopen( "/dev/random", "rb" );
        size = fread( data.data(), 2, data.size(), f );
        fclose( f );
#if 0
        f = fopen( "test.dat", "wb" );
        fwrite( data.data(), 2, data.size(), f );
        fclose( f );
#endif
#else
        FILE* f = fopen( "test.dat", "rb" );
        fseek( f, 0, SEEK_END );
        size_t size = ftell( f );
        fseek( f, 0, SEEK_SET );
        data.resize( size );
        size = fread( data.data(), 2, data.size(), f );
        fclose( f );
#endif
        printf( "BLOCK: %zu\n", size );
        const char16_t* text = data.data();
        size = data.size();
        while ( size )
        {
            /* identify paragraph. */
            size_t length = ual_analyze_paragraph( ub, text, size );

            /* analyze this paragraph. */
            ual_analyze_breaks( ub );

            ual_script_span span;
            ual_script_spans_begin( ub );
            while ( ual_script_spans_next( ub, &span ) )
            {
                /* process this script span. */
                assert( span.lower <= size );
                assert( span.upper <= size );
                assert( span.lower <= span.upper );
            }
            ual_script_spans_end( ub );

            ual_analyze_bidi( ub, UAL_FROM_TEXT );
            ual_bidi_run run;
            ual_bidi_runs_begin( ub );
            while ( ual_bidi_runs_next( ub, &run ) )
            {
                /* process this bidi run. */
                assert( run.lower <= size );
                assert( run.upper <= size );
                assert( span.lower <= span.upper );
            }
            ual_bidi_runs_end( ub );

            /* move to next paragraph. */
            assert( length <= size );
            text += length;
            size -= length;
        }
    }
}
