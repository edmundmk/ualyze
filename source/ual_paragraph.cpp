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


bool ual_next_paragraph( ual_buffer* ub, ual_paragraph* out_paragraph )
{
    ub->c.clear();
    ub->p.lower = ub->p.upper;
    if ( ub->p.lower >= ub->s.size() )
    {
        if ( out_paragraph )
        {
            *out_paragraph = ub->p;
        }
        return false;
    }

    for ( ual_index i = 0; i < ub->s.size(); ++i )
    {

    }

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

