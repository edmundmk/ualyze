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
    ,   ucdn( ucdn_record_table() )
    ,   bc_usage( BC_NONE )
    ,   p{ 0, 0 }
    ,   script_analysis{ INVALID_INDEX }
    ,   bidi_analysis{ INVALID_INDEX, INVALID_INDEX }
{
}

ual_buffer::~ual_buffer()
{
}

ual_buffer* ual_buffer_create()
{
    if ( ucdn_record_table_size() >= IX_INVALID )
    {
        assert( ! "ucdn record table is too large" );
        return nullptr;
    }

    return new ual_buffer();
}

ual_buffer* ual_buffer_retain( ual_buffer* ub )
{
    ++ub->refcount;
    return ub;
}

void ual_buffer_release( ual_buffer* ub )
{
    if ( ub && --ub->refcount == 0 )
    {
        delete ub;
    }
}

void ual_buffer_clear( ual_buffer* ub )
{
    ub->s.clear();
    ub->p = { 0, 0 };
}

void ual_buffer_append( ual_buffer* ub, ual_string_view text )
{
    assert( ub->p.upper == 0 );
    ub->s.append( text.data, text.size );
}

ual_string_view ual_buffer_text( ual_buffer* ub, size_t lower, size_t upper )
{
    assert( lower <= upper );
    assert( upper <= ub->s.size() );
    return { ub->s.data() + lower, upper - lower };
}

ual_char* ual_char_buffer( ual_buffer* ub, size_t* out_count )
{
    if ( out_count )
    {
        *out_count = ub->c.size();
    }
    return ub->c.data();
}

char32_t ual_codepoint( ual_buffer* ub, size_t index )
{
    index += ub->p.lower;
    char32_t uc = ub->s[ index++ ];

    // Check for surrogate.
    if ( ( uc & 0xF800 ) == 0xD800 )
    {
        // Get next code unit.
        char32_t ul = index < ub->s.size() ? ub->s[ index++ ] : 0;

        // Check for high/low surrogate pair.
        bool have_hi_surrogate = ( uc & 0xFC00 ) == 0xD800;
        bool have_lo_surrogate = ( ul & 0xFC00 ) == 0xDC00;
        if ( have_hi_surrogate && have_lo_surrogate )
        {
            // Decode surrogate pair.
            uc = 0x010000 + ( ( uc & 0x3FF ) << 10 ) + ( ul & 0x3FF );
        }
        else
        {
            // Treat lone surrogates as U+FFFD REPLACEMENT CHARACTER.
            uc = 0xFFFD;
        }
    }

    return uc;
}


