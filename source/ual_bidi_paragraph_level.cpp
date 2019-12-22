//
//  ual_bidi_paragraph_level.cpp
//
//  Created by Edmund Kapusniak on 22/12/2019.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
//

#include "ual_internal.h"
#include "ual_bidi.h"
#include <ucdn/ucdn.h>

unsigned ual_bidi_paragraph_level( ual_buffer* ub )
{
    const UCDRecord* ucdn = ucdn_record_table();

    /*
        Calculate paragraph embedding level for this paragraph.

        P2.  In each paragraph, find the first character of type L, AL, or R
        while skipping over any characters between an isolate initiator and its
        matching PDI or, if it has no matching PDI, the end of the paragraph.

        P3.  If a character is found in P2 and it is of type AL or R, then set
        the paragraph embedding level to one; otherwise, set it to zero.
    */

    int isolate_count = 0;
    size_t length = ub->c.size();
    for ( size_t i = 0; i < length; ++i )
    {
        const ual_char& c = ub->c[ i ];
        if ( c.ix == IX_INVALID )
        {
            continue;
        }

        unsigned bidi_class = ucdn[ c.ix ].bidi_class;
        switch ( bidi_class )
        {
        case UCDN_BIDI_CLASS_L:
        case UCDN_BIDI_CLASS_AL:
        case UCDN_BIDI_CLASS_R:
            if ( isolate_count == 0 )
            {
                return bidi_class != UCDN_BIDI_CLASS_L ? 1 : 0;
            }
            break;

        case UCDN_BIDI_CLASS_LRI:
        case UCDN_BIDI_CLASS_RLI:
        case UCDN_BIDI_CLASS_FSI:
            isolate_count += 1;
            break;

        case UCDN_BIDI_CLASS_PDI:
            if ( isolate_count > 0 )
            {
                isolate_count -= 1;
            }
            break;

        default:
            break;
        }
    }

    return 0;
}

