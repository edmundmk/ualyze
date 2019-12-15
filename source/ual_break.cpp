//
//  ual_break.cpp
//
//  Created by Edmund Kapusniak on 23/08/2019.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
//

#include "ual_internal.h"
#include "ual_break.h"
#include <ucdn/ucdn.h>

/*
    Include state machines.
*/

#define ACTION signed char
#define BREAK( x ) +x
#define NO_BREAK( x ) -x

#include "uax14.h"
#include "uax29p3.h"

#undef ACTION
#undef BREAK
#undef NO_BREAK

/*
    Run both state machines at the same time.
*/

void ual_break_analyze( ual_buffer* ub )
{
    const UCDRecord* ucdn = ucdn_record_table();

    int lb_state = STATE_SOT;
    int cb_state = STATE_CONTROL_PLUS;

    size_t length = ub->c.size() - 1;
    for ( size_t i = 0; i < length; ++i )
    {
        ual_char& c = ub->c[ i ];
        if ( c.ix == IX_INVALID )
        {
            c.bc = 0;
            continue;
        }

        const UCDRecord& u = ucdn[ c.ix ];
        lb_state = UAX14[ lb_state ][ u.linebreak_class ];
        cb_state = UAX29P3[ cb_state ][ u.grapheme_cluster_break ];

        unsigned bc = 0;
        if ( lb_state < 0 )
        {
            bc |= UAL_BREAK_LINE;
            lb_state = -lb_state;
        }
        if ( cb_state < 0 )
        {
            bc |= UAL_BREAK_CLUSTER;
            cb_state = -cb_state;
        }

        c.bc = bc;
    }

    ub->c[ length ].bc |= UAL_BREAK_CLUSTER;
}

