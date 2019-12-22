//
//  ual_paragraph.cpp
//
//  Created by Edmund Kapusniak on 23/08/2019.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
//

#include "ual_internal.h"
#include <assert.h>
#include "ucdn.h"

bool ual_next_paragraph( ual_buffer* ub, ual_paragraph* out_paragraph )
{
    const UCDRecord* ucdn = ucdn_record_table();
    if ( ucdn_record_table_size() >= IX_INVALID )
    {
        assert( ! "ual_next_paragraph : ucdn record table is too large" );
        return false;
    }

    ub->c.clear();
    ub->p.lower = ub->p.upper;

    // Check if we're already at the end of the string.
    if ( ub->p.lower >= ub->s.size() )
    {
        ub->c.push_back( { IX_INVALID, 0 } );
        if ( out_paragraph )
        {
            *out_paragraph = ub->p;
        }
        return false;
    }

    // Perform analysis.
    size_t i = ub->p.lower;
    unsigned prev = UCDN_LINEBREAK_CLASS_XX;
    while ( i < ub->s.size() )
    {
        // Decode character from UTF-16.
        size_t inext = i;
        char32_t uc = ub->s[ inext++ ];

        // Check for surrogate.
        if ( ( uc & 0xF800 ) == 0xD800 )
        {
            // Get next code unit.
            char32_t ul = inext < ub->s.size() ? ub->s[ inext++ ] : 0;

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

        // Look up character in ucdn.
        unsigned ix = ucdn_get_record_index( uc );

        // Check for line break.
        unsigned curr = ucdn[ ix ].linebreak_class;
        if ( prev == UCDN_LINEBREAK_CLASS_BK
            || prev == UCDN_LINEBREAK_CLASS_NL
            || prev == UCDN_LINEBREAK_CLASS_LF
            || ( prev == UCDN_LINEBREAK_CLASS_CR && curr != UCDN_LINEBREAK_CLASS_LF ) )
        {
            break;
        }

        // No line break, add character to char buffer.
        ub->c.push_back( { (uint16_t)ix, 0 } );
        if ( uc >= 0x010000 )
        {
            ub->c.push_back( { IX_INVALID, 0 } );
        }

        prev = curr;
        i = inext;
    }

    // Index is first character of next paragraph (or end of string).
    ub->p.upper = i;
    if ( out_paragraph )
    {
        *out_paragraph = ub->p;
    }

    return true;
}

ual_string_view ual_paragraph_text( ual_buffer* ub )
{
    return ual_buffer_text( ub, ub->p.lower, ub->p.upper );
}

