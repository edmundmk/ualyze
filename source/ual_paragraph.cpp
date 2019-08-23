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
#include <stdexcept>
#include "ucdn.h"


bool ual_next_paragraph( ual_buffer* ub, ual_paragraph* out_paragraph )
{
    const UCDRecord* ucdn = ucdn_record_table();
    if ( ucdn_record_table_size() >= IX_INVALID )
    {
        throw std::out_of_range( "ucdn record table is too large" );
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
    size_t i = 0;
    int prev = UCDN_LINEBREAK_CLASS_XX;
    while ( i < ub->s.size() )
    {
        // Decode character from UTF-16.
        size_t inext = i;
        char32_t uc = ub->s[ inext++ ];
        bool is_hi_surrogate = ( uc & 0xFC00 ) == 0xD800;
        bool is_lo_surrogate = ( uc & 0xFC00 ) == 0xDC00;
        if ( is_hi_surrogate || is_lo_surrogate )
        {
            if ( is_hi_surrogate && inext < ub->s.size() && ( ub->s[ inext ] & 0xFC00 ) == 0xDC00 )
                uc = 0x010000 + ( ( uc & 0x3FF ) << 10 ) + ( ub->s[ inext++ ] & 0x3FF );
            else
                uc = 0xFFFD;
        }

        // Look up character in ucdn.
        uint16_t ix = ucdn_get_record_index( uc );

        // Check for line break.
        int curr = ucdn[ ix ].linebreak_class;
        if ( prev == UCDN_LINEBREAK_CLASS_BK
            || prev == UCDN_LINEBREAK_CLASS_NL
            || prev == UCDN_LINEBREAK_CLASS_LF
            || ( prev == UCDN_LINEBREAK_CLASS_CR && curr != UCDN_LINEBREAK_CLASS_LF ) )
        {
            break;
        }

        // No line break, add character to char buffer.
        ub->c.push_back( { ix, 0 } );
        if ( uc >= 0x010000 )
        {
            ub->c.push_back( { IX_INVALID, 0 } );
        }

        prev = curr;
        i = inext;
    }

    // Index is first character of next paragraph (or end of string).
    ub->p.upper = i;

    // Add past-the-end char to simplify algorithms.
    ub->c.push_back( { IX_INVALID, 0 } );
    if ( out_paragraph )
    {
        *out_paragraph = ub->p;
    }
    return true;
}


std::u16string_view ual_paragraph_text( ual_buffer* ub )
{
    return ual_buffer_text( ub, ub->p.lower, ub->p.upper );
}

