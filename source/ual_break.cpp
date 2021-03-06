//
//  ual_break.cpp
//
//  Created by Edmund Kapusniak on 23/08/2019.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the ISC License. See LICENSE file in the project root for
//  full license information.
//

#include "ualyze.h"
#include <assert.h>
#include "ual_buffer.h"

/*
    Include state machines.
*/

#define ACTION signed char
#define BREAK( x ) -x-1
#define NO_BREAK( x ) +x
#define LOOKAHEAD_NU( x ) -x-63

#include "uax14.h"
#include "uax29p3.h"

#undef ACTION
#undef BREAK
#undef NO_BREAK

/*
    Run both state machines at the same time.
*/

const size_t NO_SPACE = SIZE_MAX;

static bool lookahead_nu( ual_buffer* ub, size_t index, size_t length )
{
    // Skip past low surrogates.
    while ( index < length && ub->c[ index ].ix == IX_INVALID )
    {
        ++index;
    }

    // Check for NU linebreak class.
    if ( index < length )
    {
        const ual_char& c = ub->c[ index ];
        const ucdb_entry& uentry = UCDB_TABLE[ c.ix ];
        return uentry.lbreak == UCDB_LBREAK_NU;
    }

    // Not found.
    return false;
}

UAL_API void ual_analyze_breaks( ual_buffer* ub )
{
    int lb_state = STATE_SOT_ZWJ;
    int cb_state = STATE_CONTROL_LF;

    size_t space_index = NO_SPACE;
    bool was_space = false;

    size_t length = ub->c.size();
    for ( size_t i = 0; i < length; ++i )
    {
        // Get character, skip surrogates.
        ual_char& c = ub->c[ i ];
        if ( c.ix == IX_INVALID )
        {
            c.bc = 0;
            continue;
        }

        // Look up properties.
        const ucdb_entry& uentry = UCDB_TABLE[ c.ix ];
        unsigned lb_class = uentry.lbreak;
        unsigned cb_class = uentry.cbreak;

        // Read state machine.
        lb_state = UAX14[ lb_state ][ lb_class ];
        cb_state = UAX29P3[ cb_state ][ cb_class ];

        // Determine break class.
        unsigned bc = 0;
        if ( lb_state < 0 )
        {
            if ( lb_state > -62 )
            {
                lb_state = -lb_state-1;
                bc |= UAL_BREAK_LINE;
            }
            else
            {
                lb_state = -lb_state-63;
                if ( ! lookahead_nu( ub, i + 1, length ) )
                {
                    bc |= UAL_BREAK_LINE;
                }
            }

            if ( was_space )
            {
                assert( space_index != NO_SPACE );
                ub->c[ space_index ].bc |= UAL_BREAK_SPACES;
            }

            space_index = NO_SPACE;
            was_space = false;
        }
        if ( cb_state < 0 )
        {
            bc |= UAL_BREAK_CLUSTER;
            cb_state = -cb_state-1;
        }
        c.bc = bc;

        // Check for space.
        bool is_space =
               uentry.zspace                    // space characters
            || lb_class == UCDB_LBREAK_ZW       // ZERO WIDTH SPACE
            || lb_state == STATE_NL_LF_CR_BK;   // newlines
        if ( is_space && ! was_space )
        {
            space_index = i;
        }
        was_space = is_space;
    }

    // Set last space index, if any.
    if ( was_space )
    {
        assert( space_index != NO_SPACE );
        ub->c[ space_index ].bc |= UAL_BREAK_SPACES;
    }

    ub->bc_usage = BC_BREAK_FLAGS;
}

