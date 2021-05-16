//
//  ual_script.cpp
//
//  Created by Edmund Kapusniak on 22/12/2019.
//  Copyright © Edmund Kapusniak.
//
//  Licensed under the ISC License. See LICENSE file in the project root for
//  full license information.
//

#include "ualyze.h"
#include <assert.h>
#include "ual_buffer.h"
#include "ucdb_script.h"
#include "ucdb_bracket.h"

/*
    Bracket stack.
*/

const size_t SCRIPT_BRSTACK_LIMIT = 64;

struct ual_script_bracket
{
    unsigned closing_bracket;
    unsigned script;
};

struct ual_script_brstack
{
    ual_script_bracket* ss;
    unsigned sp;
};

static ual_script_brstack get_brstack( ual_buffer* ub )
{
    assert( ub->script_analysis.index != INVALID_INDEX );
    return { ual_stack< ual_script_bracket, SCRIPT_BRSTACK_LIMIT >( ub ), ub->script_analysis.sp };
}

static void trim_brstack( ual_script_brstack* stack, size_t index )
{
    assert( index <= stack->sp );
    stack->sp = index;
}

static bool push_bracket( ual_script_brstack* stack, const ual_script_bracket& bracket )
{
    if ( stack->sp < SCRIPT_BRSTACK_LIMIT )
    {
        stack->ss[ stack->sp ] = bracket;
        stack->sp += 1;
        return true;
    }
    else
    {
        return false;
    }
}

const unsigned BELOW_BRACKET_LEVEL = ~(unsigned)0;

static unsigned match_bracket( ual_script_brstack* stack, size_t bracket_level, char32_t uc, unsigned curr_script )
{
    size_t i = stack->sp;
    while ( i-- )
    {
        const ual_script_bracket& b = stack->ss[ i ];
        if ( b.closing_bracket == uc )
        {
            if ( i < bracket_level )
            {
                return BELOW_BRACKET_LEVEL;
            }

            unsigned script = b.script;
            trim_brstack( stack, i );
            return script;
        }
    }

    return curr_script;
}

/*
    Lookahead past characters with common and inherited script to find
    first character with a real script (skipping brackets).
*/

static unsigned lookahead( ual_buffer* ub, ual_script_brstack* stack, size_t index, unsigned curr_script )
{
    size_t bracket_level = stack->sp;
    unsigned fallback = UCDB_SCRIPT_COMMON;

    size_t length = ub->c.size();
    for ( ; index < length; ++index )
    {
        const ual_char& c = ub->c[ index ];
        if ( c.ix == IX_INVALID )
        {
            continue;
        }

        const ucdb_entry& uentry = UCDB_TABLE[ c.ix ];
        unsigned char_script = uentry.script;

        if ( uentry.paired )
        {
            // Match paired bracket.
            char32_t closing_bracket = '\0';
            char32_t uc = ucdb_mapped_bracket( ual_codepoint( ub, index ) );
            ucdb_bracket_kind bracket_kind = ucdb_paired_bracket( uc, &closing_bracket );
            assert( bracket_kind != UCDB_BRACKET_NONE );

            if ( bracket_kind == UCDB_BRACKET_OPEN )
            {
                // Opening bracket.
                push_bracket( stack, { closing_bracket, UCDB_SCRIPT_COMMON } );
            }
            else if ( bracket_kind == UCDB_BRACKET_CLOSE )
            {
                // Closing bracket.
                if ( match_bracket( stack, bracket_level, uc, UCDB_SCRIPT_COMMON ) == BELOW_BRACKET_LEVEL )
                {
                    break;
                }
            }
        }

        if ( char_script == UCDB_SCRIPT_COMMON || char_script == UCDB_SCRIPT_INHERITED )
        {
            continue;
        }

        // If we're at the original bracket level, we have actual script.
        if ( stack->sp <= bracket_level )
        {
            return char_script;
        }

        // Otherwise, remember first actual script, even inside brackets.
        if ( fallback == UCDB_SCRIPT_COMMON )
        {
            fallback = char_script;
        }
    }

    // Fall back to current script.
    if ( fallback == UCDB_SCRIPT_COMMON )
    {
        fallback = curr_script;
    }

    // Failed to find a character with an actual script in these brackets.
    trim_brstack( stack, bracket_level );
    return fallback;
}

/*
    Iterator-style interface to produce script spans.
*/

UAL_API void ual_script_spans_begin( ual_buffer* ub )
{
    // Start at beginning of paragraph.
    ub->script_analysis = { 0, UCDB_SCRIPT_LATIN, 0 };

    // Lookahead to determine script of first character.
    ual_script_brstack stack = get_brstack( ub );
    ub->script_analysis.script = lookahead( ub, &stack, 0, UCDB_SCRIPT_LATIN );
}

UAL_API bool ual_script_spans_next( ual_buffer* ub, ual_script_span* out_span )
{
    // Load state.
    ual_script_brstack stack = get_brstack( ub );
    size_t index = ub->script_analysis.index;
    unsigned curr_script = ub->script_analysis.script;

    // Build span.
    out_span->lower = index;

    // Check if we've reached the end.
    size_t length = ub->c.size();
    if ( index >= length )
    {
        out_span->upper = index;
        out_span->script = UCDB_SCRIPT_CODE[ UCDB_SCRIPT_LATIN ];
        return false;
    }

    // Check each character.
    unsigned char_script = curr_script;
    for ( ++index; index < length; ++index )
    {
        // Get character.  Skip surrogates.
        const ual_char& c = ub->c[ index ];
        if ( c.ix == IX_INVALID )
        {
            continue;
        }

        // Look up script for character.
        const ucdb_entry& uentry = UCDB_TABLE[ c.ix ];
        char_script = uentry.script;

        // Check for brackets.
        if ( uentry.paired )
        {
            // Check for bracket.
            char32_t closing_bracket = '\0';
            char32_t uc = ucdb_mapped_bracket( ual_codepoint( ub, index ) );
            ucdb_bracket_kind bracket_kind = ucdb_paired_bracket( uc, &closing_bracket );
            assert( bracket_kind != UCDB_BRACKET_NONE );

            if ( bracket_kind == UCDB_BRACKET_OPEN )
            {
                // This bracket inherits the current script.  Move past it.
                index += 1;
                while ( index < length && ub->c[ index ].ix == IX_INVALID )
                {
                    index += 1;
                }
                if ( index >= length )
                {
                    break;
                }

                // Opening bracket.
                push_bracket( &stack, { closing_bracket, curr_script } );

                // Determine script of now-current character by lookahead.
                char_script = lookahead( ub, &stack, index, curr_script );
            }
            else if ( bracket_kind == UCDB_BRACKET_CLOSE )
            {
                // Closing bracket.  Might change script here.
                char_script = match_bracket( &stack, 0, uc, curr_script );
            }
        }

        // Common and inherited characters do not change script.
        if ( char_script == UCDB_SCRIPT_COMMON || char_script == UCDB_SCRIPT_INHERITED )
        {
            continue;
        }

        if ( char_script != curr_script )
        {
            // Script has changed.
            break;
        }
    }

    assert( index >= length || char_script != curr_script );
    assert( out_span->lower < index );

    // Save state.
    ub->script_analysis.index = index;
    ub->script_analysis.script = char_script;
    ub->script_analysis.sp = stack.sp;

    // Return resulting span.
    out_span->upper = index;
    out_span->script = UCDB_SCRIPT_CODE[ curr_script ];
    return true;
}

UAL_API void ual_script_spans_end( ual_buffer* ub )
{
    ub->script_analysis.index = INVALID_INDEX;
}

