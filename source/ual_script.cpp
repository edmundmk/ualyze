//
//  ual_script.cpp
//
//  Created by Edmund Kapusniak on 22/12/2019.
//  Copyright © Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
//

#include "ualyze.h"
#include "ual_buffer.h"

size_t ual_script_analyze( ual_buffer* ub )
{

}

ual_script_span* ual_script_spans( ual_buffer* ub, size_t* out_count )
{
    if ( out_count )
    {
        *out_count = ub->script_spans.size();
    }
    return ub->script_spans.data();
}

