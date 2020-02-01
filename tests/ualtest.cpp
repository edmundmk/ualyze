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
#include <ualyze.h>

int main( int argc, char* argv[] )
{
    // Reopen stdin and stdout.
#ifdef _WIN32
    _setmode( _fileno( stdin ), _O_BINARY );
    _setmode( _fileno( stdout ), _O_BINARY );
#endif

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
    ual_buffer_append( ub, { (const char16_t*)data.data(), data.size() / 2 } );

    // Process paragraph-by-paragraph.
    ual_paragraph paragraph;
    while ( ual_paragraph_next( ub, &paragraph ) )
    {
        // Print paragraph.
        printf( "PARAGRAPH %zu %zu\n", paragraph.lower, paragraph.upper );

        // Analyze breaks.
        size_t count = 0;
        const ual_char* c = ual_break_analyze( ub, &count );
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

/*
        // Analyze breaks.
        ual_break_analyze( ub );
        ual_char* chars = ual_char_buffer( ub, nullptr );
        for ( size_t i = paragraph.lower; i < paragraph.upper; ++i )
        {
            ual_char c = chars[ i - paragraph.lower ];
            if ( c.bc & UAL_BREAK_SPACES )
            {
                printf( "BREAK_SPACES %zu\n", i );
            }
            if ( c.bc & UAL_BREAK_LINE )
            {
                printf( "BREAK_LINE %zu\n", i );
            }
            if ( c.bc & UAL_BREAK_CLUSTER )
            {
                printf( "BREAK_CLUSTER %zu\n", i );
            }
        }

        // Write hard break at end of paragraph.
        printf( "HARD_BREAK %zu\n", paragraph.upper );
*/
    }

    ual_buffer_release( ub );

    // Complete.
    return EXIT_SUCCESS;
}

