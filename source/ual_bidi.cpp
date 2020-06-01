//
//  ual_bidi.cpp
//
//  Created by Edmund Kapusniak on 01/01/2020.
//  Copyright © 2020 Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
//

#include "ualyze.h"
#include <assert.h>
#include "ual_buffer.h"

/*
    sos/eos types.  BC_SEQUENCE represents a link to next/previous run in the
    isolating run sequence.
*/

static_assert( UCDU_BIDI_L == 0 );
static_assert( UCDU_BIDI_R == 1 );
static_assert( UCDU_BIDI_AL == 2 );
static_assert( BC_SEQUENCE == 3 );

/*
    Debug print.
*/


static const char* bidi_os( unsigned bidi_class )
{
    switch ( bidi_class )
    {
    case UCDU_BIDI_L:   return "L";
    case UCDU_BIDI_R:   return "R";
    case UCDU_BIDI_AL:  return "AL";
    case BC_SEQUENCE:   return "SEQ";
    }
    return "??";
}

static const char* bidi_cs( unsigned bidi_class )
{
    switch ( bidi_class )
    {
    case UCDU_BIDI_L:   return "L";
    case UCDU_BIDI_R:   return "R";
    case UCDU_BIDI_AL:  return "AL";
    case UCDU_BIDI_LRE: return "LRE";
    case UCDU_BIDI_LRO: return "LRO";
    case UCDU_BIDI_RLE: return "RLE";
    case UCDU_BIDI_RLO: return "RLO";
    case UCDU_BIDI_PDF: return "PDF";
    case UCDU_BIDI_EN:  return "EN";
    case UCDU_BIDI_ES:  return "ES";
    case UCDU_BIDI_ET:  return "ET";
    case UCDU_BIDI_AN:  return "AN";
    case UCDU_BIDI_CS:  return "CS";
    case UCDU_BIDI_NSM: return "NSM";
    case UCDU_BIDI_BN:  return "BN";
    case UCDU_BIDI_B:   return "B";
    case UCDU_BIDI_S:   return "S";
    case UCDU_BIDI_WS:  return "WS";
    case UCDU_BIDI_ON:  return "ON";
    case UCDU_BIDI_LRI: return "LRI";
    case UCDU_BIDI_RLI: return "RLI";
    case UCDU_BIDI_FSI: return "FSI";
    case UCDU_BIDI_PDI: return "PDI";
    case BC_INVALID:    return "INV";
    }

    return "??";
}

static void debug_print_bidi( ual_buffer* ub )
{
    switch ( ub->bidi_analysis.complexity )
    {
    case BIDI_ALL_LEFT: printf( "BIDI_COMPLEXITY ALL_LEFT\n" ); break;
    case BIDI_SOLITARY: printf( "BIDI_COMPLEXITY SOLITARY\n" ); break;
    case BIDI_EXPLICIT: printf( "BIDI_COMPLEXITY EXPLICIT\n" ); break;
    }

    unsigned index = 0;
    for ( ual_level_run lrun : ub->level_runs )
    {
        printf( "LEVEL_RUN [%u] %u %u %s %s %u\n", index++, lrun.start, lrun.level, bidi_os( lrun.sos ), bidi_os( lrun.eos ), lrun.inext );
    }

    printf( "BIDI_STRING" );
    for ( ual_char c : ub->c )
    {
        printf( " %s", bidi_cs( c.bc ) );
    }
    printf( "\n" );
}


/*
    Look up bidi classes for each codepoint in a paragraph.  The class of
    each codepoint is modified by later stages of the algorithm.
*/

static ual_bidi_complexity bidi_lookup( ual_buffer* ub )
{
    bool left = true;
    bool solitary = true;

    size_t length = ub->c.size();
    for ( size_t index = 0; index < length; ++index )
    {
        ual_char& c = ub->c[ index ];
        if ( c.ix == IX_INVALID )
        {
            c.bc = BC_INVALID;
            continue;
        }

        c.bc = UCDU_TABLE[ c.ix ].bidi_class;

        switch ( c.bc )
        {
        case UCDU_BIDI_R:
        case UCDU_BIDI_AL:
        case UCDU_BIDI_AN:
            left = false;
            break;

        case UCDU_BIDI_LRE:
        case UCDU_BIDI_LRO:
        case UCDU_BIDI_RLE:
        case UCDU_BIDI_RLO:
        case UCDU_BIDI_PDF:
        case UCDU_BIDI_LRI:
        case UCDU_BIDI_RLI:
        case UCDU_BIDI_FSI:
        case UCDU_BIDI_PDI:
            left = false;
            solitary = false;
            break;
        }
    }

    ub->bc_usage = BC_BIDI_CLASS;

    debug_print_bidi( ub );

    if ( left )
        return BIDI_ALL_LEFT;
    else if ( solitary )
        return BIDI_SOLITARY;
    else
        return BIDI_EXPLICIT;
}

/*
    In a string with no embeddings, overrides, or isolates, build a single
    level run in a single isolating run sequence with the correct direction.

    Return the paragraph embedding level.
*/

static unsigned bidi_solitary( ual_buffer* ub, unsigned override_paragraph_level )
{
    unsigned paragraph_level = 0;
    unsigned boundary_class = UCDU_BIDI_L;

    size_t length = ub->c.size();
    if ( override_paragraph_level == UAL_FROM_TEXT )
    {
        for ( size_t index = 0; index < length; ++index )
        {
            const ual_char& c = ub->c[ index ];
            if ( c.bc == UCDU_BIDI_L )
            {
                break;
            }
            if ( c.bc == UCDU_BIDI_AL || c.bc == UCDU_BIDI_R )
            {
                paragraph_level = 1;
                boundary_class = UCDU_BIDI_R;
                break;
            }
        }
    }
    else
    {
        paragraph_level = override_paragraph_level;
        boundary_class = paragraph_level & 1;
    }

    ub->level_runs.push_back( { 0, paragraph_level, boundary_class, boundary_class, 0 } );
    ub->level_runs.push_back( { (unsigned)length, paragraph_level, BC_SEQUENCE, BC_SEQUENCE, 0 } );
    return paragraph_level;
}

/*
    Peform rules X1 to X10.  This identifies level runs and isolating run
    sequences.  The bidi classes of characters are modified by overrides.
    Characters 'removed' by rule X9 have their class set to BN.

    Return the paragraph embedding level.
*/

const unsigned BIDI_MAX_DEPTH = 125;
const size_t BIDI_EXSTACK_LIMIT = BIDI_MAX_DEPTH + 2;

const unsigned BIDI_INVALID_LEVEL_RUN = 0xFFFFF;

enum ual_bidi_override_isolate
{
    BIDI_EMBEDDING_L = UCDU_BIDI_L,     // within the scope of an LRO.
    BIDI_EMBEDDING_R = UCDU_BIDI_R,     // within the scope of an RLO.
    BIDI_EMBEDDING_NEUTRAL,             // within the scope of an LRE or RLE.
    BIDI_ISOLATE,                       // within the scope of an FSI, LRI, or RLI.
};

struct ual_bidi_exentry
{
    unsigned level  : 8;    // bidi level.
    unsigned oi     : 4;    // override or isolate status.
    unsigned iprev  : 20;   // index of previous level run (for BIDI_ISOLATE).
};

struct ual_bidi_exstack
{
    ual_bidi_exentry* ss;
    size_t sp;
};

static ual_bidi_exstack make_exstack( ual_buffer* ub )
{
    assert( ub->script_analysis.index == INVALID_INDEX );
    return { ual_stack< ual_bidi_exentry, BIDI_EXSTACK_LIMIT >( ub ), 0 };
}

static unsigned first_strong_level( ual_buffer* ub, size_t index, bool fsi )
{
    /*
        P2.  In each paragraph, find the first character of type L, AL, or R
        while skipping over any characters between an isolate initiator and its
        matching PDI or, if it has no matching PDI, the end of the paragraph.

        P3.  If a character is found in P2 and it is of type AL or R, then set
        the paragraph embedding level to one; otherwise, set it to zero.
    */

    size_t isolate_counter = 0;
    size_t length = ub->c.size();
    while ( index < length )
    {
        unsigned bc = ub->c[ index++ ].bc;
        switch ( bc )
        {
        case UCDU_BIDI_L:
            if ( isolate_counter == 0 )
            {
                return 0;
            }
            break;

        case UCDU_BIDI_AL:
        case UCDU_BIDI_R:
            if ( isolate_counter == 0 )
            {
                return 1;
            }
            break;

        case UCDU_BIDI_LRI:
        case UCDU_BIDI_RLI:
        case UCDU_BIDI_FSI:
            isolate_counter += 1;
            break;

        case UCDU_BIDI_PDI:
            if ( isolate_counter > 0 )
            {
                isolate_counter -= 1;
            }
            else if ( fsi )
            {
                return 0;
            }
            break;
        }
    }

    return 0;
}

static unsigned next_level( unsigned level, bool odd )
{
    /*
        next_even = ( level | 1 ) + 1
        next_odd  = ( level + 1 ) | 1
        next      = ( ( level | even ) + 1 ) | odd
    */

    unsigned even = odd ^ 1;
    return ( ( level | even ) + 1 ) | (unsigned)odd;
}

static unsigned boundary_class( unsigned a, unsigned b )
{
    unsigned higher = std::max( a, b );
    return higher & 1;
}

static unsigned bidi_explicit( ual_buffer* ub, unsigned override_paragraph_level )
{
    ub->level_runs.clear();

    // Create stack.
    ual_bidi_exstack stack = make_exstack( ub );

    // Calculate paragraph embedding level and the base stack entry.
    unsigned paragraph_level = 0;
    if ( override_paragraph_level == UAL_FROM_TEXT )
    {
        paragraph_level = first_strong_level( ub, 0, false );
    }
    else
    {
        paragraph_level = override_paragraph_level;
    }
    ual_bidi_exentry stack_entry = { paragraph_level, BIDI_EMBEDDING_NEUTRAL, BIDI_INVALID_LEVEL_RUN };

    // Embedding/isolate state.
    size_t overflow_isolate_count = 0;
    size_t overflow_embedding_count = 0;
    size_t valid_isolate_count = 0;

    // We are at the start of the string.
    unsigned run_start = 0;
    unsigned run_level = paragraph_level;
    unsigned run_sos = boundary_class( paragraph_level, paragraph_level );
    size_t run_isolate_count = 0;

    size_t index = 0;
    size_t length = ub->c.size();
    for ( ; index < length; ++index )
    {
        ual_char& c = ub->c[ index ];

        unsigned bc = c.bc;
        if ( bc == BC_INVALID )
        {
            continue;
        }

        if ( bc == UCDU_BIDI_B )
        {
            break;
        }

        switch ( bc )
        {
        // X2-5.
        case UCDU_BIDI_LRE:
        case UCDU_BIDI_RLE:
        case UCDU_BIDI_LRO:
        case UCDU_BIDI_RLO:
        {
            // Compute the least odd (RLE/RLO) or even (LRE/LRO) embedding
            // level greater than the embedding level of the last stack entry.
            bool rl = bc == UCDU_BIDI_RLE || bc == UCDU_BIDI_RLO;
            unsigned level = next_level( stack_entry.level, rl );

            // Check for overflow embedding.
            if ( level > BIDI_MAX_DEPTH || overflow_isolate_count > 0 || overflow_embedding_count > 0 )
            {
                // If the overflow isolate count is zero, increment the
                // overflow embedding count by one.
                if ( overflow_isolate_count == 0 )
                {
                    overflow_embedding_count += 1;
                }
                break;
            }

            // Determine directional override depending on embedding type.
            ual_bidi_override_isolate oi;
            if ( bc == UCDU_BIDI_LRO )
                oi = BIDI_EMBEDDING_L;
            else if ( bc == UCDU_BIDI_RLO )
                oi = BIDI_EMBEDDING_R;
            else
                oi = BIDI_EMBEDDING_NEUTRAL;

            // Push an entry consisting of the new embedding level, directional
            // override status, and false directional isolate status onto the
            // directional status stack.
            assert( stack.sp < BIDI_EXSTACK_LIMIT );
            stack.ss[ stack.sp++ ] = stack_entry;
            stack_entry = { level, oi, BIDI_INVALID_LEVEL_RUN };

            // This character is 'removed' by rule X9.  Don't actually remove
            // it, instead set to BN.
            c.bc = UCDU_BIDI_BN;
            continue;
        }
        break;

        // X7
        case UCDU_BIDI_PDF:
        {
            // This character is 'removed' by rule X9.  Dont' actually remove
            // it, instead set to BN.
            c.bc = UCDU_BIDI_BN;

            if ( overflow_isolate_count > 0 )
            {
                // If the overflow isolate count is greater than zero,
                // just remove the unmatched PDF.
                continue;
            }

            if ( overflow_embedding_count > 0 )
            {
                // Otherwise, if the overflow embedding count is greater than
                // zero, decrement it by one, and remove unmatched PDF.
                overflow_embedding_count -= 1;
                continue;
            }

            if ( stack_entry.oi == BIDI_ISOLATE || stack.sp == 0 )
            {
                // The PDF does not match a valid embedding indicator.
                continue;
            }

            // Pop the last entry from the directional status stack.
            stack_entry = stack.ss[ --stack.sp ];
            continue;
        }
        break;

        // X5a-c
        case UCDU_BIDI_FSI:
        {
            // This character is part of the current level run.  Act as if it
            // was LRI or RLI based on the first strong level inside it.
            bc = first_strong_level( ub, index + 1, true ) ? UCDU_BIDI_RLI : UCDU_BIDI_LRI;
            break;
        }

        case UCDU_BIDI_LRI:
        case UCDU_BIDI_RLI:
        {
            // This character is part of the current level run.
            break;
        }

        // X6a
        case UCDU_BIDI_PDI:
        {
            if ( overflow_isolate_count > 0 )
            {
                // If the overflow isolate count is greater than zero, this
                // PDI matches an overflow isolate initiator. Decrement the
                // overflow isolate count by one.
                overflow_isolate_count -= 1;
                break;
            }

            if ( valid_isolate_count <= 0 )
            {
                // This PDI does not match a valid isolate initiator.
                break;
            }

            // Reset the overflow embedding count to zero.
            overflow_embedding_count = 0;

            // While the directional isolate status of the last entry on
            // the stack is false, pop the last entry from the directional
            // status stack.
            while ( stack_entry.oi != BIDI_ISOLATE )
            {
                assert( stack.sp > 0 );
                stack_entry = stack.ss[ --stack.sp ];
            }

            // Pop the last entry from the directional status stack and
            // decrement the valid isolate count by one.
            valid_isolate_count -= 1;
            unsigned iprev = stack_entry.iprev;
            assert( stack.sp > 0 );
            stack_entry = stack.ss[ --stack.sp ];

            // Override character class if we are in the scope of an override.
            ual_bidi_override_isolate oi = (ual_bidi_override_isolate)stack_entry.oi;
            if ( oi == BIDI_EMBEDDING_L || oi == BIDI_EMBEDDING_R )
            {
                c.bc = oi;
            }

            // An isolate initiator immediately followed by a PDI does not
            // introduce any new level run.
            if ( stack_entry.level == run_level )
            {
                continue;
            }

            // We know this character is not removed, so the current level
            // run ends here.
            unsigned run_eos = boundary_class( run_level, stack_entry.level );
            ub->level_runs.push_back( { run_start, run_level, run_sos, run_eos, 0 } );

            // This PDI matches an isolate initiator and therefore
            // the next level run continues an isolating run sequence.
            run_sos = BC_SEQUENCE;
            run_start = index;
            run_level = stack_entry.level;
            run_isolate_count = valid_isolate_count;

            // Link the previous run to the run we're about to build.
            ual_level_run* prun = &ub->level_runs.at( iprev );
            assert( prun->inext == 0 );
            assert( prun->eos == BC_SEQUENCE );
            prun->inext = (unsigned)ub->level_runs.size();

            continue;
        }
        break;

        // X6. Remove BNs.
        case UCDU_BIDI_BN:
        {
            continue;
        }
        }

        // This character was not removed.  Override character class if we are
        // in the scope of an override.
        ual_bidi_override_isolate oi = (ual_bidi_override_isolate)stack_entry.oi;
        if ( oi == BIDI_EMBEDDING_L || oi == BIDI_EMBEDDING_R )
        {
            c.bc = oi;
        }

        // If the embedding level has changed since the last unremoved
        // character, then close old level run and start a new one.
        if ( run_level != stack_entry.level )
        {
            unsigned run_eos = BC_INVALID;
            if ( valid_isolate_count <= run_isolate_count )
            {
                // This run ends its isolating run sequence.
                run_eos = boundary_class( run_level, stack_entry.level );
            }

            // Add run that ends at this character.
            ub->level_runs.push_back( { run_start, run_level, run_sos, run_eos, 0 } );

            // Next run definitely doesn't start with a matching PDI, as
            // that case is handled above.
            run_sos = boundary_class( run_level, stack_entry.level );
            run_start = index;
            run_level = stack_entry.level;
            run_isolate_count = valid_isolate_count;
        }

        // X5a-c, now initiator has been added to current level run sequence.
        if ( bc == UCDU_BIDI_LRI || bc == UCDU_BIDI_RLI )
        {
            // Compute the least odd (RLI) or even (LRI) embedding level
            // greater than the embedding level of the last stack entry.
            unsigned level = next_level( stack_entry.level, bc == UCDU_BIDI_RLI );

            // Check for overflow isolate.
            if ( level > BIDI_MAX_DEPTH || overflow_embedding_count > 0 || overflow_embedding_count > 0 )
            {
                // Increment the overflow isolate count by one, and leave all
                // other variables unchanged.
                overflow_isolate_count += 1;
                break;
            }

            // Increment the valid isolate count by one.
            valid_isolate_count += 1;

            // Push an entry consisting of the new embedding level, neutral
            // directional override status, and true directional isolate
            // status onto the directional status stack.
            assert( stack.sp < BIDI_EXSTACK_LIMIT );
            stack.ss[ stack.sp++ ] = stack_entry;
            stack_entry = { level, BIDI_ISOLATE, (unsigned)ub->level_runs.size() };
        }
    }

    // Close unterminated isolating run sequences.
    while ( valid_isolate_count > 0 )
    {
        // Pop down to the level of the isolate.
        while ( stack_entry.oi != BIDI_ISOLATE )
        {
            assert( stack.sp > 0 );
            stack_entry = stack.ss[ --stack.sp ];
        }

        // Pop to get to the level of the isolate initiator.
        valid_isolate_count -= 1;
        unsigned iprev = stack_entry.iprev;
        assert( stack.sp > 0 );
        stack_entry = stack.ss[ --stack.sp ];

        // Update run to close it.
        ual_level_run* pprev = &ub->level_runs.at( iprev );
        assert( pprev->inext == 0 );
        assert( pprev->eos == BC_SEQUENCE );
        pprev->eos = boundary_class( pprev->level, paragraph_level );
    }

    // Add paragraph separators, which have the paragraph embedding level.
    if ( index < length )
    {
        if ( run_level != paragraph_level )
        {
            unsigned run_eos = boundary_class( run_level, paragraph_level );
            ub->level_runs.push_back( { run_start, run_level, run_sos, run_eos, 0 } );
            run_sos = run_eos;
            run_start = index;
            run_level = paragraph_level;
            run_isolate_count = valid_isolate_count;
        }
        index = length;
    }

    // Close final level run.
    unsigned run_eos = boundary_class( run_level, paragraph_level );
    ub->level_runs.push_back( { run_start, run_level, run_sos, run_eos, 0 } );

    // Add a final 'run' to simplify lookup of level runs.
    ub->level_runs.push_back( { (unsigned)index, paragraph_level, BC_SEQUENCE, BC_SEQUENCE, 0 } );

    // Done.
    return paragraph_level;
}

/*
    Perform rules W1 to W7 on the characters in each level run.

    We make a single pass through the string.  This works because:

        - Level runs in an isolating run sequence which do not terminate the
          isolating run sequence must end with an isolate initiator.
        - Level runs in an isolating run sequence which continue the isolating
          run sequence must begin with a PDI.
        - Neither isolate initiators nor PDI are removed by rule X9.
        - Therefore the lookahead in rule W5 must terminate before or at the
          end of a single level run.

    Both rule W2 and rule W7 require the type of the previous strong character.
    No weak rule changes the previous strong type - W1 changes the current
    character to the previous one, while W3 is super-predictable.  Therefore
    we can keep track of the previous strong type and hand it over to the next
    level run in the isolating run sequence.
*/

void bidi_weak( ual_buffer* ub )
{
    const size_t INVALID_INDEX = (size_t)-1;

    size_t length = ub->level_runs.size() - 1;
    for ( size_t i = 0; i < length; ++i )
    {
        ual_level_run* prun = &ub->level_runs[ i ];
        ual_level_run* nrun = &ub->level_runs[ i + 1 ];

        // Previous strong class.
        unsigned prev_strong = prun->sos;

        // Previous classes after dealing with NSMs
        unsigned prev_w1 = prun->sos;

        // Index of separator and class of number preceding it.
        enum { NONE, MATCH_EN, MATCH_EN_XS, MATCH_AN, MATCH_AN_CS } state_w4 = NONE;
        size_t index_w4 = INVALID_INDEX;

        // Index of first ET in sequence to be updated if we encounter an EN.
        unsigned prev_w5 = prun->sos;
        size_t index_w5 = INVALID_INDEX;

        // Update each of the characters in the run.
        for ( size_t i = prun->start; i < nrun->start; ++i )
        {
            ual_char& c = ub->c[ i ];
            if ( c.bc == BC_INVALID || c.bc == UCDU_BIDI_BN )
            {
                continue;
            }

            // W1. Update class of NSM to class of previous.  We leave NSM
            // sequences that follow an ON as NSMs, since rule N0 needs to
            // differentiate them.
            if ( c.bc == UCDU_BIDI_NSM )
            {
                if ( prev_w1 != UCDU_BIDI_ON
                    && prev_w1 != UCDU_BIDI_FSI
                    && prev_w1 != UCDU_BIDI_LRI
                    && prev_w1 != UCDU_BIDI_RLI
                    && prev_w1 != UCDU_BIDI_PDI )
                {
                    c.bc = prev_w1;
                }
                else if ( prev_w1 != UCDU_BIDI_ON )
                {
                    c.bc = UCDU_BIDI_ON;
                }
            }

            prev_w1 = c.bc;

            // W2. Keep track of strong left context.  Change EN to AN if
            // the strong left context is AL.
            if ( c.bc == UCDU_BIDI_R || c.bc == UCDU_BIDI_L || c.bc == UCDU_BIDI_AL )
            {
                prev_strong = c.bc;
            }
            if ( c.bc == UCDU_BIDI_EN && prev_strong == UCDU_BIDI_AL )
            {
                c.bc = UCDU_BIDI_AN;
            }

            // W3. Change AL to R.
            if ( c.bc == UCDU_BIDI_AL )
            {
                c.bc = UCDU_BIDI_R;
            }

            // W4. A single ES between two ENs changes to EN. A single CS
            // between two AN/ENs changes to AN/EN.
            if ( c.bc == UCDU_BIDI_EN )
            {
                if ( state_w4 != MATCH_EN_XS )
                {
                    state_w4 = MATCH_EN;
                }
                else
                {
                    // W7. Change EN to L if the left context is L.
                    if ( prev_strong == UCDU_BIDI_L )
                    {
                        ub->c[ index_w4 ].bc = UCDU_BIDI_L;
                    }
                    else
                    {
                        ub->c[ index_w4 ].bc = UCDU_BIDI_EN;
                    }
                    state_w4 = NONE;
                    index_w4 = INVALID_INDEX;
                }
            }
            else if ( c.bc == UCDU_BIDI_AN )
            {
                if ( state_w4 != MATCH_AN_CS )
                {
                    state_w4 = MATCH_AN;
                }
                else
                {
                    ub->c[ index_w4 ].bc = UCDU_BIDI_AN;
                    state_w4 = NONE;
                    index_w4 = INVALID_INDEX;
                }
            }
            else if ( c.bc == UCDU_BIDI_ES )
            {
                // Unmatched separators become ON.  Do this now.
                c.bc = UCDU_BIDI_ON;

                // Update state.
                if ( state_w4 == MATCH_EN )
                {
                    state_w4 = MATCH_EN_XS;
                    index_w4 = i;
                }
                else
                {
                    state_w4 = NONE;
                }
            }
            else if ( c.bc == UCDU_BIDI_CS )
            {
                // Unmatched separators become ON.  Do this now.
                c.bc = UCDU_BIDI_ON;

                // Update state.
                if ( state_w4 == MATCH_EN )
                {
                    state_w4 = MATCH_EN_XS;
                    index_w4 = i;
                }
                else if ( state_w4 == MATCH_AN )
                {
                    state_w4 = MATCH_AN_CS;
                    index_w4 = i;
                }
                else
                {
                    state_w4 = NONE;
                }
            }
            else
            {
                state_w4 = NONE;
            }

            // W5. A sequence of ETs adjacent to EN becomes EN.
            if ( c.bc == UCDU_BIDI_ET )
            {
                if ( prev_w5 == UCDU_BIDI_EN )
                {
                    c.bc = UCDU_BIDI_EN;
                }
                else
                {
                    if ( index_w5 == INVALID_INDEX )
                    {
                        index_w5 = i;
                    }

                    // Postpone further updates until end of ET sequence.
                    continue;
                }
            }
            else if ( index_w5 != INVALID_INDEX )
            {
                unsigned change_to;
                if ( c.bc == UCDU_BIDI_EN )
                {
                    // W6. A sequence of ETs adjacent to EN becomes EN.
                    change_to = UCDU_BIDI_EN;

                    // W7. Change EN to L if the left context is L.
                    if ( prev_strong == UCDU_BIDI_L )
                    {
                        change_to = UCDU_BIDI_L;
                    }
                }
                else
                {
                    // W6. Otherwise, ETs become ON.
                    change_to = UCDU_BIDI_ON;
                }

                while ( index_w5 < i )
                {
                    ual_char& et = ub->c[ index_w5++ ];
                    if ( et.bc == BC_INVALID || et.bc == UCDU_BIDI_BN )
                    {
                        continue;
                    }

                    assert( et.bc == UCDU_BIDI_ET );
                    et.bc = change_to;
                }

                index_w5 = INVALID_INDEX;
            }

            prev_w5 = c.bc;

            // W6. Otherwise, ET, ES, or CS become ON.
            assert( c.bc != UCDU_BIDI_ET );
            if ( c.bc == UCDU_BIDI_ES || c.bc == UCDU_BIDI_CS )
            {
                c.bc = UCDU_BIDI_ON;
            }

            // W7. Change EN to L if the left context is L.
            if ( c.bc == UCDU_BIDI_EN && prev_strong == UCDU_BIDI_L )
            {
                c.bc = UCDU_BIDI_L;
            }

        }

        // Deal with case where the level run ends with an ET.
        if ( index_w5 != INVALID_INDEX )
        {
            while ( index_w5 < nrun->start )
            {
                ual_char& et = ub->c[ index_w5++ ];
                if ( et.bc == BC_INVALID || et.bc == UCDU_BIDI_BN )
                {
                    continue;
                }

                assert( et.bc == UCDU_BIDI_ET );
                et.bc = UCDU_BIDI_ON;
            }
        }

        // If this run is not the last one in the isolating run sequence, set
        // sos on the next run in the sequence to provide left context.
        if ( prun->inext )
        {
            ual_level_run* pnext = &ub->level_runs.at( prun->inext );
            pnext->sos = prev_strong;
        }
    }
}

/*
    Perform rule N0, identifying bracket pairs and resolving types.

    The only types remaining are:

        strong L    -> L
        strong R    -> R, EN, AN
        neutral     -> B, S, WS, ON, FSI, LRI, RLI, PDI
        removed     -> BN, INVALID

    A strong type which matches the embedding direction is /e/.  One which
    is opposite the embedding direction is /o/.

    Rule N0 considers matched pairs of brackets in order:

      - If the brackets contain /e/, brackets become /e/.
      - If the brackets contain /o/, brackets become the preceding strong type.
      - Otherwise, brackets remain neutral.

    We avoid building an explicit list of bracket pairs, because I feel like
    a string with a lot of brackets may well be plausible.  So we match
    brackets and resolve their types at the same time.

    Per isolating run sequence:

      - Track preceding strong type.  It can also be /e-guess/.
      - When /e/, top bracket contains /e/, strong type is /e/.
      - When /o/, top bracket contains /o/, strong type is /o/.
      - When ON and not a bracket, set to B.

    When we open a bracket:

      - Push strong type with bracket.
      - Strong type becomes /e-guess/.
      - Current bracket doesn't contain /e/ or /o/ yet.

    When we match a bracket:

        Bracket does not contain /e/ or /o/:
          - Brackets become B, to avoid reprocessing them.

        Bracket contains /e/:
        Bracket contains /o/ and bracket strong type is /e/:
          - Brackets become /e/.
          - Strong type is /e/.
          - If bracket contains /e/, so does new top.
          - If bracket contains /o/, so does new top.

        Bracket contains /o/ and bracket strong type is /e-guess/:
          - Leave bracket as ON, in case it needs reprocessed.
          - Strong type is /e-guess/.
          - New top contains /o/.
          - New top is a rewind point.

        Bracket contains /o/ and bracket strong type is /o/:
          - Brackets become /o/.
          - Strong type is /o/.
          - If top is a rewind point, rewind and reprocess.

    When we mismatch a bracket:

        Always:
          - If strong type is /e-guess/, pop strong type with bracket.
          - If bracket contains /e/, so does new top.
          - If bracket contains /o/, so does new top.
          - Set bracket to B to avoid reprocessing it.

        If mismatched bracket was a rewind point and popped strong is /o/:
          - Resume from here.

        If mismatched bracket was a rewind point and popped strong is /e-guess/:
          - New top is a rewind point.

    When we rewind, we only need to update guessed brackets to /o/.  When we
    hit a strong type which isolates further brackets from the rewind point,
    then we can continue from where we rewound from.
*/

const size_t BIDI_BRSTACK_LIMIT = 63;

enum ual_bidi_eo_strong
{
    BIDI_E,
    BIDI_O,
    BIDI_E_GUESS,
};

struct ual_bidi_brentry
{
    unsigned index;                     // index of opening bracket.
    unsigned closing_bracket    : 24;   // codepoint of closing bracket.
    unsigned prev_strong        : 2;    // previous strong /e/, /o/, /e-guess/.
    unsigned prev_contains_e    : 1;    // before bracket contains e.
    unsigned prev_contains_o    : 1;    // before bracket contains o.
    unsigned rewind_point       : 1;    // bracket contains at least one internal pair guessed as e based on this bracket.
};

struct ual_bidi_brstack
{
    ual_bidi_brentry* ss;
    size_t sp;
};

static ual_bidi_brstack make_brstack( ual_buffer* ub )
{
    assert( ub->script_analysis.index == INVALID_INDEX );
    return { ual_stack< ual_bidi_brentry, BIDI_BRSTACK_LIMIT >( ub ), 0 };
}

static void rewind_o( ual_buffer* ub, size_t irun, unsigned lower, unsigned upper, unsigned o )
{
    ual_level_run* prun = &ub->level_runs.at( irun );
    ual_level_run* nrun = &ub->level_runs.at( irun + 1 );

    // Check if bracket has crossed level run boundary.
    if ( lower < prun->start )
    {
        size_t length = ub->level_runs.size() - 1;
        for ( irun = 0; irun < length; ++irun )
        {
            prun = &ub->level_runs[ irun ];
            nrun = &ub->level_runs[ irun + 1 ];

            if ( prun->start <= lower && nrun->start < upper )
            {
                break;
            }
        }

        if ( irun >= length )
        {
            assert( ! "invalid rewind index" );
            return;
        }
    }

    // I *think* it is sufficient to resolve all ON in the range to /o/.  All
    // brackets that don't depend on the incorrect guess should already have
    // been resolved.  But it requires more testing to confirm.
    for ( unsigned index = lower; index < upper; ++index )
    {
        while ( index >= nrun->start )
        {
            // Move to next level run in isolating run sequence.
            assert( prun->eos == BC_SEQUENCE );
            assert( prun->inext != 0 );
            irun = prun->inext;
            prun = &ub->level_runs.at( irun );
            nrun = &ub->level_runs.at( irun + 1 );
            index = prun->start;
        }

        assert( index < upper );
        ual_char& c = ub->c[ index ];
        if ( c.bc == UCDU_BIDI_ON )
        {
            c.bc = o;
        }
    }
}

static void bidi_isolating_brackets( ual_buffer* ub, ual_bidi_brstack* stack, size_t irun )
{
    ual_level_run* prun = &ub->level_runs[ irun ];
    ual_level_run* nrun = &ub->level_runs[ irun + 1 ];

    // Get /e/ and /o/ directions.
    unsigned e = prun->level & 1;
    unsigned o = ! e;
    assert( prun->sos == UCDU_BIDI_L || prun->sos == UCDU_BIDI_R );
    ual_bidi_eo_strong prev_strong = prun->sos == o ? BIDI_O : BIDI_E;

    // Bracket stack is empty, context contains no strong characters.
    stack->sp = 0;
    bool contains_e = false;
    bool contains_o = false;
    bool give_up = false;

    // Go through every level run in isolating run sequence.
    while ( true )
    {
        // Process characters in level run.
        for ( unsigned index = prun->start; index < nrun->start; ++index )
        {
            ual_char& c = ub->c[ index ];
            switch ( c.bc )
            {
            case UCDU_BIDI_L:
            case UCDU_BIDI_R:
            case UCDU_BIDI_EN:
            case UCDU_BIDI_AN:
            {
                unsigned direction = c.bc != UCDU_BIDI_L;
                prev_strong = direction == o ? BIDI_O : BIDI_E;
                contains_e |= prev_strong == BIDI_E;
                contains_o |= prev_strong == BIDI_O;
            }
            break;

            case UCDU_BIDI_ON:
            {
                // If the bracket stack overflowed even once, we've given up.
                if ( give_up )
                {
                    c.bc = UCDU_BIDI_B;
                    break;
                }

                // Check for bracket.
                ucdu_bracket_kind bracket_kind = UCDU_BRACKET_NONE;
                char32_t closing_bracket = '\0';
                char32_t uc = '\0';

                const ucdu_record& record = UCDU_TABLE[ c.ix ];
                if ( record.paired )
                {
                    uc = ual_codepoint( ub, index );
                    bracket_kind = ucdu_paired_bracket( uc, &closing_bracket );
                }

                if ( bracket_kind == UCDU_BRACKET_NONE )
                {
                    // Not a bracket.  Convert to B to avoid reprocessing.
                    c.bc = UCDU_BIDI_B;
                    break;
                }

                if ( bracket_kind == UCDU_BRACKET_OPEN )
                {
                    // If the stack is full, give up and ignore this bracket.
                    if ( stack->sp >= BIDI_BRSTACK_LIMIT )
                    {
                        give_up = true;
                        c.bc = UCDU_BIDI_B;
                        break;
                    }

                    // Push open bracket.
                    stack->ss[ stack->sp++ ] =
                    {
                        index,
                        closing_bracket,
                        prev_strong,
                        contains_e,
                        contains_o,
                        false
                    };

                    // After bracket does not contain /e/ or /o/ yet.
                    contains_e = false;
                    contains_o = false;

                    // Strong type is /e-guess/, assume that we'll find an /e/.
                    prev_strong = BIDI_E_GUESS;
                    break;
                }

                assert( bracket_kind == UCDU_BRACKET_CLOSE );

                // Search bracket stack for matching bracket.
                size_t match = stack->sp;
                while ( match-- )
                {
                    ual_bidi_brentry* entry = stack->ss + match;
                    if ( entry->closing_bracket == uc )
                    {
                        break;
                    }
                }

                if ( match >= stack->sp )
                {
                    // Closing bracket was not matched.
                    c.bc = UCDU_BIDI_B;
                    break;
                }

                // All brackets above match are mismatched.
                while ( --stack->sp > match )
                {
                    ual_bidi_brentry* entry = stack->ss + stack->sp;

                    // Clear opening bracket to B to avoid reprocessing.
                    ub->c[ entry->index ].bc = UCDU_BIDI_B;

                    // Add context up until bracket to our current context.
                    contains_e |= entry->prev_contains_e;
                    contains_o |= entry->prev_contains_o;

                    // If strong type is /e-guess/, recover previous strong.
                    if ( prev_strong == BIDI_E_GUESS )
                    {
                        prev_strong = (ual_bidi_eo_strong)entry->prev_strong;
                    }

                    // Deal with rewind.
                    if ( entry->rewind_point )
                    {
                        if ( entry->prev_strong == BIDI_E_GUESS )
                        {
                            // Move rewind point to contained bracket.
                            assert( stack->sp > 0 );
                            stack->ss[ stack->sp - 1 ].rewind_point = true;
                        }
                        else if ( entry->prev_strong == BIDI_O )
                        {
                            // Brackets dependent on this context become /o/.
                            rewind_o( ub, irun, entry->index, index, o );
                        }
                    }
                }

                // Process matching bracket.
                assert( stack->sp == match );
                ual_bidi_brentry* entry = stack->ss + stack->sp;
                ual_char& b = ub->c[ entry->index ];

                // Outer context contains inner one.
                bool inner_contains_e = contains_e;
                bool inner_contains_o = contains_o;
                contains_e |= entry->prev_contains_e;
                contains_o |= entry->prev_contains_o;

                if ( ! inner_contains_e && ! inner_contains_o )
                {
                    // Neutral brackets.
                    b.bc = UCDU_BIDI_B;
                    c.bc = UCDU_BIDI_B;
                    prev_strong = (ual_bidi_eo_strong)entry->prev_strong;

                    //printf( "BIDI_BRACKET %u %u NEUTRAL\n", entry->index, index );
                }
                else if ( inner_contains_e || entry->prev_strong == BIDI_E )
                {
                    assert( inner_contains_e || inner_contains_o );

                    // This bracket pair is /e/.
                    b.bc = e;
                    c.bc = e;
                    contains_e = true;
                    prev_strong = BIDI_E;

                    //printf( "BIDI_BRACKET %u %u %s\n", entry->index, index, contains_e ? "CONTAINS_E" : "PREV_E" );
                }
                else if ( entry->prev_strong == BIDI_E_GUESS )
                {
                    assert( inner_contains_o );

                    // This pair depends on guessed strong from outer bracket.
                    assert( stack->sp > 0 );
                    stack->ss[ stack->sp - 1 ].rewind_point = true;

                    // Later brackets may also depend on the same guess.
                    prev_strong = BIDI_E_GUESS;

                    //printf( "BIDI_BRACKET %u %u GUESSED_E\n", entry->index, index );
                }
                else
                {
                    assert( inner_contains_o );
                    assert( entry->prev_strong == BIDI_O );

                    // Brackets are /o/.
                    b.bc = o;
                    c.bc = o;
                    prev_strong = BIDI_O;

                    // Guess was wrong.  If any inner pairs depend on it, rewind.
                    if ( entry->rewind_point )
                    {
                        rewind_o( ub, irun, entry->index, index, o );
                    }

                    //printf( "BIDI_BRACKET %u %u O\n", entry->index, index );
                }
            }
            break;
            }
        }

        // Move to next run in isolating run sequence.
        size_t inext = prun->inext;
        if ( ! inext )
        {
            assert( prun->eos != BC_SEQUENCE );
            break;
        }

        assert( prun->eos == BC_SEQUENCE );
        prun = &ub->level_runs.at( inext );
        nrun = &ub->level_runs.at( inext + 1 );

        // sos was clobbered by weak processing, set back to SEQUENCE.
        prun->sos = BC_SEQUENCE;
    }

    // All remaining opening brackets are mismatched.
    while ( stack->sp-- )
    {
        ual_bidi_brentry* entry = stack->ss + stack->sp;

        // Clear opening bracket to B to avoid reprocessing.
        ub->c[ entry->index ].bc = UCDU_BIDI_B;

        // Deal with rewind.
        if ( entry->rewind_point )
        {
            if ( entry->prev_strong == BIDI_E_GUESS )
            {
                // Move rewind point to contained bracket.
                assert( stack->sp > 0 );
                stack->ss[ stack->sp - 1 ].rewind_point = true;
            }
            else if ( entry->prev_strong == BIDI_O )
            {
                // Brackets dependent on this context become /o/.
                rewind_o( ub, irun, entry->index, nrun->start, o );
            }
        }
    }
}

void bidi_brackets( ual_buffer* ub )
{
    // Create stack.
    ual_bidi_brstack stack = make_brstack( ub );

    // Process each isolating run sequence independently.
    size_t length = ub->level_runs.size() - 1;
    for ( size_t irun = 0; irun < length; ++irun )
    {
        // Skip runs that continue an isolating run sequence.
        ual_level_run* prun = &ub->level_runs[ irun ];
        if ( prun->sos == BC_SEQUENCE )
        {
            continue;
        }

        // Process this isolating run sequence.
        bidi_isolating_brackets( ub, &stack, irun );
    }
}

/*
    Perform rules N1 and N2, resolving types for runs of neutrals.

    The only types remaining in the string are:

        strong L    -> L
        strong R    -> R, EN, AN
        strong /e/  -> ON
        non-spacing -> NSM
        neutral     -> B, S, WS, FSI, LRI, RLI, PDI
        removed     -> BN, INVALID

    ON is treated as a strong type matching embedding direction, either L or R.
    The only remaining ON characters are guessed brackets.

    The only NSMs remaining are those which originally followed an ON.  That
    ON will have been updated to L, R, or B (meaning neutral).  Each NSM is
    given the class of the preceding non-removed character, since rule N0
    requires that they are updated with the class of matched brackets.

    We resolve to the following non-neutral types:

        L, R, AN, EN, BN, INVALID
*/

enum ual_bidi_neutral_kind
{
    NEUTRAL_NONE,
    NEUTRAL_SPAN,
};

static void bidi_resolve_neutrals( ual_buffer* ub, size_t irun, unsigned lower, unsigned upper, unsigned bc )
{
    ual_level_run* prun = &ub->level_runs.at( irun );
    ual_level_run* nrun = &ub->level_runs.at( irun + 1 );

    for ( unsigned index = lower; index < upper; ++index )
    {
        while ( index >= nrun->start )
        {
            // Move to next level run in isolating run sequence.
            assert( prun->eos == BC_SEQUENCE );
            assert( prun->inext != 0 );
            irun = prun->inext;
            prun = &ub->level_runs.at( irun );
            nrun = &ub->level_runs.at( irun + 1 );
            index = prun->start;
        }

        assert( index < upper );
        ual_char& c = ub->c[ index ];
        if ( c.bc != UCDU_BIDI_BN && c.bc != BC_INVALID )
        {
            c.bc = bc;
        }
    }
}

static void bidi_isolating_neutral( ual_buffer* ub, size_t irun )
{
    ual_level_run* prun = &ub->level_runs[ irun ];
    ual_level_run* nrun = &ub->level_runs[ irun + 1 ];

    // Get embedding direction /e/, and sos type.
    unsigned e = prun->level & 1;
    assert( prun->sos == UCDU_BIDI_L || prun->sos == UCDU_BIDI_R );
    unsigned prev_strong = prun->sos;

    // No run of neutrals yet.
    ual_bidi_neutral_kind neutrals = NEUTRAL_NONE;
    size_t lirun = irun;
    unsigned lower = 0;
    unsigned prev_w1 = prun->sos;

    // Go through every level run in isolating run sequence.
    while ( true )
    {
        // Process characters in level run.
        for ( unsigned index = prun->start; index < nrun->start; ++index )
        {
            ual_char& c = ub->c[ index ];
            if ( c.bc == BC_INVALID )
            {
                continue;
            }

            switch ( c.bc )
            {
            case UCDU_BIDI_L:
            case UCDU_BIDI_R:
            case UCDU_BIDI_EN:
            case UCDU_BIDI_AN:
            {
                // Strong L or R.
                unsigned strong = c.bc != UCDU_BIDI_L;
                if ( neutrals == NEUTRAL_SPAN )
                {
                    unsigned bc = prev_strong == strong ? strong : e;
                    bidi_resolve_neutrals( ub, lirun, lower, index, bc );
                }
                neutrals = NEUTRAL_NONE;
                prev_strong = strong;
            }
            break;

            case UCDU_BIDI_ON:
            {
                // This is a bracket guessed as embedding direction.
                c.bc = e;

                //  Strong /e/.
                if ( neutrals == NEUTRAL_SPAN )
                {
                    bidi_resolve_neutrals( ub, lirun, lower, index, e );
                }
                neutrals = NEUTRAL_NONE;
                prev_strong = e;
            }
            break;

            case UCDU_BIDI_NSM:
            {
                // Update with previous class.  Cannot start or end a neutral
                // span, since the class is not changing.
                c.bc = prev_w1;
            }
            break;

            case UCDU_BIDI_BN:
            {
                // Ignore BN.
                continue;
            }
            break;

            default:
            {
                // Remaining classes are neutral.
                if ( neutrals == NEUTRAL_NONE)
                {
                    lirun = irun;
                    lower = index;
                    neutrals = NEUTRAL_SPAN;
                }
            }
            break;
            }

            prev_w1 = c.bc;
        }

        // Move to next run in isolating run sequence.
        size_t inext = prun->inext;
        if ( ! inext )
        {
            assert( prun->eos != BC_SEQUENCE );
            break;
        }

        assert( prun->eos == BC_SEQUENCE );
        prun = &ub->level_runs.at( inext );
        nrun = &ub->level_runs.at( inext + 1 );
    }

    assert( prun->eos == UCDU_BIDI_L || prun->eos == UCDU_BIDI_R );

    // Resolve neutrals at end of isolating run sequence.
    if ( neutrals == NEUTRAL_SPAN )
    {
        unsigned bc = prev_strong == prun->eos ? prun->eos : e;
        bidi_resolve_neutrals( ub, lirun, lower, nrun->start, bc );
    }
}


void bidi_neutral( ual_buffer* ub )
{
    // Process each isolating run sequence independently.
    size_t length = ub->level_runs.size() - 1;
    for ( size_t irun = 0; irun < length; ++irun )
    {
        // Skip runs that continue an isolating runs equence.
        ual_level_run* prun = &ub->level_runs[ irun ];
        if ( prun->sos == BC_SEQUENCE )
        {
            continue;
        }

        bidi_isolating_neutral( ub, irun );
    }
}

/*
    Perform bidi analysis, generating a final set of bidi runs.
*/

void bidi_initial( ual_buffer* ub, unsigned override_paragraph_level )
{
    ub->level_runs.clear();

    // Look up initial bidi classes.
    ual_bidi_complexity complexity = bidi_lookup( ub );
    if ( complexity == BIDI_ALL_LEFT && override_paragraph_level != 0 && override_paragraph_level != UAL_FROM_TEXT )
    {
        complexity = BIDI_SOLITARY;
    }

    // Perform level run anlysis.
    size_t index = 0;
    unsigned paragraph_level = 0;
    switch ( complexity )
    {
    case BIDI_ALL_LEFT:
        // Entire string is left to right.  No further analysis required.
        index = ub->c.size();
        ub->level_runs.push_back( { 0, 0, UCDU_BIDI_L, UCDU_BIDI_L, 0 } );
        ub->level_runs.push_back( { (unsigned)index, paragraph_level, BC_SEQUENCE, BC_SEQUENCE, 0 } );
        break;

    case BIDI_SOLITARY:
        // There is only one level run at rule X10.
        paragraph_level = bidi_solitary( ub, override_paragraph_level );
        break;

    case BIDI_EXPLICIT:
        // We require full processing of embeddings/overrides/isolates.
        paragraph_level = bidi_explicit( ub, override_paragraph_level );
        break;
    }

    // Set up analysis state.
    ub->bidi_analysis.ilrun = 0;
    ub->bidi_analysis.index = index;
    ub->bidi_analysis.paragraph_level = paragraph_level;
    ub->bidi_analysis.complexity = complexity;
}

UAL_API unsigned ual_analyze_bidi( ual_buffer* ub, unsigned override_paragraph_level )
{
    bidi_initial( ub, override_paragraph_level );
    debug_print_bidi( ub );

    if ( ub->bidi_analysis.complexity != BIDI_ALL_LEFT )
    {
        bidi_weak( ub );
        debug_print_bidi( ub );

        bidi_brackets( ub );
        debug_print_bidi( ub );

        bidi_neutral( ub );
        debug_print_bidi( ub );
    }

    return ub->bidi_analysis.paragraph_level;
}

/*
    Iterator-style interface for constructing bidi runs from resolved classes.
*/

UAL_API void ual_bidi_runs_begin( ual_buffer* ub )
{
    assert( ub->bc_usage == BC_BIDI_CLASS );
    assert( ub->level_runs.back().start == ub->c.size() );

    //debug_print_bidi( ub );

    ub->bidi_analysis.ilrun = 0;
    ub->bidi_analysis.index = 0;
}

UAL_API bool ual_bidi_runs_next( ual_buffer* ub, ual_bidi_run* out_run )
{
    assert( ub->bc_usage == BC_BIDI_CLASS );

    size_t ilrun = ub->bidi_analysis.ilrun;
    size_t index = ub->bidi_analysis.index;
    assert( ilrun != INVALID_INDEX );
    assert( index != INVALID_INDEX );

    // Run starts at index.
    out_run->lower = index;

    // Check if we've reached the end.
    if ( index >= ub->c.size() )
    {
        out_run->upper = index;
        out_run->level = ub->bidi_analysis.paragraph_level;
        return false;
    }

    // Start at the level of the run.
    ual_level_run* prun = &ub->level_runs[ ilrun ];
    ual_level_run* nrun = &ub->level_runs[ ilrun + 1 ];

    assert( prun->start < nrun->start );
    assert( nrun->start <= ub->c.size() );
    assert( prun->start <= index );
    assert( index < nrun->start );

    if ( ub->bidi_analysis.complexity == BIDI_ALL_LEFT )
    {
        // Move over level run.
        ilrun += 1;
        index = nrun->start;

        // Save state.
        ub->bidi_analysis.index = index;
        ub->bidi_analysis.ilrun = ilrun;

        // Return run.
        out_run->upper = index;
        out_run->level = prun->level;
        return true;
    }

    unsigned level = prun->level;

    unsigned bc = ub->c[ index ].bc;
    ++index;

    if ( ( level & 1 ) == 0 )
    {
        // Embedding level is even.
        if ( bc == UCDU_BIDI_L || bc == UCDU_BIDI_BN || bc == BC_INVALID )
        {
            while ( index < nrun->start )
            {
                bc = ub->c[ index ].bc;
                if ( bc != UCDU_BIDI_L && bc != UCDU_BIDI_BN && bc != BC_INVALID ) break;
                ++index;
            }
        }
        else if ( bc == UCDU_BIDI_R )
        {
            level += 1;
            while ( index < nrun->start )
            {
                bc = ub->c[ index ].bc;
                if ( bc != UCDU_BIDI_R && bc != UCDU_BIDI_BN && bc != BC_INVALID ) break;
                ++index;
            }
        }
        else
        {
            level += 2;
            while ( index < nrun->start )
            {
                bc = ub->c[ index ].bc;
                if ( bc != UCDU_BIDI_EN && bc != UCDU_BIDI_AN && bc != UCDU_BIDI_BN && bc != BC_INVALID ) break;
                ++index;
            }
        }
    }
    else
    {
        // Embedding level is odd.
        if ( bc == UCDU_BIDI_R || bc == UCDU_BIDI_BN || bc == BC_INVALID )
        {
            while ( index < nrun->start )
            {
                bc = ub->c[ index ].bc;
                if ( bc != UCDU_BIDI_R && bc != UCDU_BIDI_BN && bc != BC_INVALID ) break;
                ++index;
            }
        }
        else
        {
            level += 1;
            while ( index < nrun->start )
            {
                bc = ub->c[ index ].bc;
                if ( bc != UCDU_BIDI_L && bc != UCDU_BIDI_EN && bc != UCDU_BIDI_AN && bc != UCDU_BIDI_BN && bc != BC_INVALID ) break;
                ++index;
            }
        }
    }

    assert( index <= nrun->start );
    assert( out_run->lower < index );

    // Move to next run.
    if ( index >= nrun->start )
    {
        ilrun += 1;
    }

    // Save state.
    ub->bidi_analysis.index = index;
    ub->bidi_analysis.ilrun = ilrun;

    // Return resulting run.
    out_run->upper = index;
    out_run->level = level;
    return true;
}

UAL_API void ual_bidi_runs_end( ual_buffer* ub )
{
    ub->bidi_analysis.ilrun = INVALID_INDEX;
    ub->bidi_analysis.index = INVALID_INDEX;
}

