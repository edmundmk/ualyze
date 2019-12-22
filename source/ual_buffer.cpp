//
//  ual_buffer.cpp
//
//  Created by Edmund Kapusniak on 23/08/2019.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
//

#include "ual_internal.h"
#include <assert.h>

ual_buffer::ual_buffer()
    :   refcount( 1 )
    ,   p{ 0, 0 }
{
}

ual_buffer::~ual_buffer()
{
}

ual_buffer* ual_buffer_create()
{
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
}

void ual_buffer_append( ual_buffer* ub, ual_string_view text )
{
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

