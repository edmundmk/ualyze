//
//  ual_buffer.cpp
//
//  Created by Edmund Kapusniak on 23/08/2019.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
//

#include "ual_buffer.h"
#include <assert.h>

ual_buffer::ual_buffer()
    :   refcount( 1 )
    ,   bc_usage( BC_NONE )
    ,   script_analysis{ INVALID_INDEX }
    ,   bidi_analysis{ INVALID_INDEX, INVALID_INDEX }
{
}

ual_buffer::~ual_buffer()
{
}

UAL_API ual_buffer* ual_buffer_create()
{
    return new ual_buffer();
}

UAL_API ual_buffer* ual_buffer_retain( ual_buffer* ub )
{
    ++ub->refcount;
    return ub;
}

UAL_API void ual_buffer_release( ual_buffer* ub )
{
    if ( ub && --ub->refcount == 0 )
    {
        delete ub;
    }
}

UAL_API const char16_t* ual_buffer_text( ual_buffer* ub )
{
    return ub->text.data();
}

UAL_API const ual_char* ual_buffer_chars( ual_buffer* ub )
{
    return ub->c.data();
}

UAL_API size_t ual_buffer_size( ual_buffer* ub )
{
    return ub->text.size();
}

char32_t ual_codepoint( ual_buffer* ub, size_t index )
{
    assert( index < ub->text.size() );
    char32_t uc = ub->text[ index ];

    // Check for surrogate.
    if ( ( uc & 0xF800 ) == 0xD800 )
    {
        // Get next code unit.
        char32_t ul = index < ub->text.size() ? ub->text[ index ] : 0;

        // Check for high/low surrogate pair.
        bool have_hi_surrogate = ( uc & 0xFC00 ) == 0xD800;
        bool have_lo_surrogate = ( ul & 0xFC00 ) == 0xDC00;
        if ( have_hi_surrogate && have_lo_surrogate )
        {
            // Decode surrogate pair.
            uc = 0x010000 + ( ( uc & 0x3FF ) << 10 ) + ( ul & 0x3FF );
            index += 1;
        }
        else
        {
            // Treat lone surrogates as U+FFFD REPLACEMENT CHARACTER.
            uc = 0xFFFD;
        }
    }

    return uc;
}


