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
        fprintf( stderr, "incomplete UTF-16 input" );
        return EXIT_FAILURE;
    }

    // Create buffer.
    ual_buffer* ub = ual_buffer_create();
    ual_buffer_append( ub, { (const char16_t*)data.data(), data.size() / 2 } );

    // Process paragraph-by-paragraph.
    ual_paragraph paragraph;
    while ( ual_next_paragraph( ub, &paragraph ) )
    {
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
    }

    // Complete.
    return EXIT_SUCCESS;
}

