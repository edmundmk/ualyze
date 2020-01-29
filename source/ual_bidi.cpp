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
#include "ual_buffer.h"
#include "ucdn.h"

const uint16_t BC_INVALID = 31;

/*
    Look up bidi classes for each codepoint in a paragraph.  The class of
    each codepoint is modified by later stages of the algorithm.
*/

enum bidi_complexity
{
    BIDI_ALL_LEFT, // Paragraph is left-to-right.
    BIDI_SOLITARY, // No directional embeddings, overrides, or isolates.
    BIDI_EXPLICIT, // Requires full processing.
};

static bidi_complexity bidi_lookup( ual_buffer* ub )
{
    const UCDRecord* ucdn = ucdn_record_table();

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

        c.bc = ucdn[ c.ix ].bidi_class;

        switch ( c.bc )
        {
        case UCDN_BIDI_CLASS_R:
        case UCDN_BIDI_CLASS_AL:
        case UCDN_BIDI_CLASS_AN:
            left = false;
            break;

        case UCDN_BIDI_CLASS_LRE:
        case UCDN_BIDI_CLASS_LRO:
        case UCDN_BIDI_CLASS_RLE:
        case UCDN_BIDI_CLASS_RLO:
        case UCDN_BIDI_CLASS_PDF:
        case UCDN_BIDI_CLASS_LRI:
        case UCDN_BIDI_CLASS_RLI:
        case UCDN_BIDI_CLASS_FSI:
        case UCDN_BIDI_CLASS_PDI:
            left = false;
            solitary = false;
            break;
        }
    }

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

static unsigned bidi_solitary( ual_buffer* ub )
{
    unsigned paragraph_level = 0;
    unsigned boundary_class = UCDN_BIDI_CLASS_L;

    size_t length = ub->c.size();
    for ( size_t index = 0; index < length; ++index )
    {
        const ual_char& c = ub->c[ index ];
        if ( c.bc == UCDN_BIDI_CLASS_L )
        {
            break;
        }
        if ( c.bc == UCDN_BIDI_CLASS_AL || c.bc == UCDN_BIDI_CLASS_R )
        {
            paragraph_level = 1;
            boundary_class = UCDN_BIDI_CLASS_R;
            break;
        }
    }

    ub->level_runs.push_back( { 0, paragraph_level, boundary_class, boundary_class, NEXT_INVALID } );
    ub->level_runs.push_back( { length, 0, BOUNDARY_SEQ, BOUNDARY_SEQ, NEXT_INVALID } );
    return paragraph_level;
}

/*
    Peform rules X1 to X10.  This identifies level runs and isolating run
    sequences.  The bidi classes of characters are modified by overrides.
    Characters 'removed' by rule X9 have their class set to BN.

    Return the paragraph embedding level.
*/

const unsigned BIDI_MAX_DEPTH = 125;
const unsigned BIDI_EXSTACK_LIMIT = BIDI_MAX_DEPTH + 2;

static_assert( UCDN_BIDI_CLASS_L == 0 );
static_assert( UCDN_BIDI_CLASS_R == 3 );

const unsigned BIDI_INVALID_LEVEL_RUN = ~(unsigned)0;

enum ual_bidi_override_isolate : uint8_t
{
    BIDI_EMBEDDING_L,       // within the scope of an LRO.
    BIDI_EMBEDDING_R,       // within the scope of an RLO.
    BIDI_EMBEDDING_NEUTRAL, // within the scope of an LRE or RLE.
    BIDI_ISOLATE,           // within the scope of an FSI, LRI, or RLI.
};

struct ual_bidi_exentry
{
    unsigned irun;                  // index of previous level run.
    uint8_t level;                  // bidi level.
    ual_bidi_override_isolate oi;   // override or isolate.
};

struct ual_bidi_exstack
{
    ual_bidi_exentry* base;
    size_t pointer;
};

static ual_bidi_exstack make_exstack( ual_buffer* ub )
{
    return { (ual_bidi_exentry*)ual_stack_bytes( ub, BIDI_EXSTACK_LIMIT, sizeof( ual_bidi_exentry ) ), 0 };
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
        case UCDN_BIDI_CLASS_L:
            if ( isolate_counter == 0 )
            {
                return 0;
            }
            break;

        case UCDN_BIDI_CLASS_AL:
        case UCDN_BIDI_CLASS_R:
            if ( isolate_counter == 0 )
            {
                return 1;
            }
            break;

        case UCDN_BIDI_CLASS_LRI:
        case UCDN_BIDI_CLASS_RLI:
        case UCDN_BIDI_CLASS_FSI:
            isolate_counter += 1;
            break;

        case UCDN_BIDI_CLASS_PDI:
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
    if ( higher & 1 )
        return UCDN_BIDI_CLASS_R;
    else
        return UCDN_BIDI_CLASS_L;
}

static unsigned bidi_explicit( ual_buffer* ub )
{
    ub->level_runs.clear();

    // Create stack.
    ual_bidi_exstack stack = make_exstack( ub );

    // Calculate paragraph embedding level and the base stack entry.
    unsigned paragraph_level = first_strong_level( ub, 0, false );
    ual_bidi_exentry stack_entry = { BIDI_INVALID_LEVEL_RUN, paragraph_level, BIDI_EMBEDDING_NEUTRAL };

    // Embedding/isolate state.
    size_t overflow_isolate_count = 0;
    size_t overflow_embedding_count = 0;
    size_t valid_isolate_count = 0;

    // We are at the start of the string.
    size_t run_isolate_count = 0;
    size_t run_start = 0;
    unsigned run_level = paragraph_level;
    unsigned run_sos = boundary_class( paragraph_level, paragraph_level );

    size_t index = 0;
    size_t length = ub->c.size();
    for ( ; index < length; ++index )
    {
        ual_char& c = ub->c[ index ];
        unsigned clevel = stack_entry.level;

        unsigned bc = c.bc;
        if ( bc == BC_INVALID )
        {
            continue;
        }

        switch ( bc )
        {
        // X2-5.
        case UCDN_BIDI_CLASS_LRE:
        case UCDN_BIDI_CLASS_RLE:
        case UCDN_BIDI_CLASS_LRO:
        case UCDN_BIDI_CLASS_RLO:
        {
            // This character is 'removed' by rule X9.  Don't actually remove
            // it, instead set to BN.
            c.bc = UCDN_BIDI_CLASS_BN;

            // Compute the least odd (RLE/RLO) or even (LRE/LRO) embedding
            // level greater than the embedding level of the last stack entry.
            bool rl = bc == UCDN_BIDI_CLASS_RLE || bc == UCDN_BIDI_CLASS_RLO;
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
            if ( bc == UCDN_BIDI_CLASS_LRO )
                oi = BIDI_EMBEDDING_L;
            else if ( bc == UCDN_BIDI_CLASS_RLO )
                oi = BIDI_EMBEDDING_R;
            else
                oi = BIDI_EMBEDDING_NEUTRAL;

            // Push an entry consisting of the new embedding level, directional
            // override status, and false directional isolate status onto the
            // directional status stack.
            assert( stack.pointer < BIDI_EXSTACK_LIMIT );
            stack.base[ stack.pointer++ ] = stack_entry;
            stack_entry = { BIDI_INVALID_LEVEL_RUN, level, oi };
        }
        break;

        // X7
        case UCDN_BIDI_CLASS_PDF:
        {
            // This character is 'removed' by rule X9.  Dont' actually remove
            // it, instead set to BN.
            c.bc = UCDN_BIDI_CLASS_BN;

            if ( overflow_isolate_count > 0 )
            {
                // If the overflow isolate count is greater than zero,
                // do nothing.
                break;
            }

            if ( overflow_embedding_count > 0 )
            {
                // Otherwise, if the overflow embedding count is greater than
                // zero, decrement it by one.
                overflow_embedding_count -= 1;
                break;
            }

            if ( stack_entry.oi == BIDI_ISOLATE || stack.pointer == 0 )
            {
                // The PDF does not match a valid embedding indicator.
                break;
            }

            // Pop the last entry from the directional status stack.
            stack_entry = stack.base[ --stack.pointer ];

            // If this character hadn't been removed, it would be part of
            // the new level run.
            clevel = stack_entry.level;
        }
        break;

        // X5a-c
        case UCDN_BIDI_CLASS_FSI:
            bc = first_strong_level( ub, index + 1, true ) ? UCDN_BIDI_CLASS_RLI : UCDN_BIDI_CLASS_LRI;
            goto isolate_initiator;

        case UCDN_BIDI_CLASS_LRI:
        case UCDN_BIDI_CLASS_RLI:
        isolate_initiator:
        {
            // This character is part of the current level run.

            // Compute the least odd (RLI) or even (LRI) embedding level
            // greater than the embedding level of the last stack entry.
            unsigned level = next_level( stack_entry.level, bc == UCDN_BIDI_CLASS_RLI );

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
            assert( stack.pointer < BIDI_EXSTACK_LIMIT );
            stack.base[ stack.pointer++ ] = stack_entry;
            stack_entry = { (unsigned)ub->level_runs.size(), level, BIDI_ISOLATE };
        }
        break;

        // X6a
        case UCDN_BIDI_CLASS_PDI:
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
                assert( stack.pointer > 0 );
                stack_entry = stack.base[ --stack.pointer ];
            }

            // Pop the last entry from the directional status stack and
            // decrement the valid isolate count by one.
            valid_isolate_count -= 1;
            unsigned irun = stack_entry.irun;
            assert( stack.pointer > 0 );
            stack_entry = stack.base[ --stack.pointer ];

            // This character is part of the new level run.
            clevel = stack_entry.level;

            // An isolate initiator immediately followed by a PDI does not
            // introduce any new level run.
            if ( clevel == run_level )
            {
                continue;
            }

            // We know this character is not removed, so the current level
            // run ends here.
            unsigned run_eos = boundary_class( run_level, clevel );
            ub->level_runs.push_back( { run_start, 0, run_level, run_sos, run_eos } );

            // This PDI matches an isolate initiator and therefore
            // the next level run continues an isolating run sequence.
            run_isolate_count = valid_isolate_count;
            run_sos = BC_INVALID;
            run_start = index;
            run_level = clevel;

            // Link the previous run to the run we're about to build.
            ual_level_run* prun = &ub->level_runs.at( irun );
            assert( prun->inext == 0 );
            assert( prun->eos = BC_INVALID );
            prun->inext = (unsigned)ub->level_runs.size();

            continue;
        }
        break;

        case UCDN_BIDI_CLASS_B:
        {
            // Give all B characters the paragraph embedding level.
            clevel = paragraph_level;
        }
        break;

        default:
        {
            // Override character class if we are in the scope of an override.
            ual_bidi_override_isolate oi = stack_entry.oi;
            if ( oi == BIDI_EMBEDDING_L )
            {
                c.bc = UCDN_BIDI_CLASS_L;
            }
            else if ( oi == BIDI_EMBEDDING_R )
            {
                c.bc = UCDN_BIDI_CLASS_R;
            }
        }
        break;
        }

        // Check if we've arrived at an unremoved character with a new
        // embedding level.
        if ( c.bc != UCDN_BIDI_CLASS_BN && clevel != run_level )
        {
            unsigned run_eos = BC_INVALID;
            if ( valid_isolate_count <= run_isolate_count )
            {
                // This run ends its isolating run sequence.
                run_eos = boundary_class( run_level, clevel );
            }

            // Add run that ends at this character.
            ub->level_runs.push_back( { run_start, 0, run_level, run_sos, run_eos } );

            // Next run definitely doesn't start with a matching PDI, as
            // that case is handled above.
            run_isolate_count = valid_isolate_count;
            run_sos = boundary_class( run_level, clevel );
            run_start = index;
            run_level = clevel;
        }
    }

    // Close final sequence.
    unsigned run_eos = boundary_class( run_level, paragraph_level );
    ub->level_runs.push_back( { run_start, 0, run_level, run_sos, run_eos } );

    // Close unterminated isolating run sequences.
    while ( valid_isolate_count > 0 )
    {
        // Pop down to the level of the isolate.
        while ( stack_entry.oi != BIDI_ISOLATE )
        {
            assert( stack.pointer > 0 );
            stack_entry = stack.base[ --stack.pointer ];
        }

        // Pop to get to the level of the isolate initiator.
        valid_isolate_count -= 1;
        unsigned irun = stack_entry.irun;
        assert( stack.pointer > 0 );
        stack_entry = stack.base[ --stack.pointer ];

        // Update run to close it.
        ual_level_run* prun = &ub->level_runs.at( irun );
        assert( prun->inext == 0 );
        assert( prun->eos = BC_INVALID );
        prun->eos = boundary_class( prun->level, paragraph_level );
    }

    // Add a final 'run' to simplify lookup of level runs.
    ub->level_runs.push_back( { index, 0, paragraph_level, BC_INVALID, BC_INVALID } );

    // Done.
    return paragraph_level;
}

/*
    Perform rules W1 to W7 on the characters in each level run.

    We do a single pass through the string.

    Both rule W2 and rule W7 require the type of the previous strong character.
    No weak rule changes the previous strong type - W1 changes the current
    character to the previous one, while W3 is super-predictable.  Therefore
    we can keep track of the previous strong type and hand it over to the next
    level run in an isolating run sequence.
*/

static unsigned lookahead( ual_buffer* ub, size_t i, size_t upper )
{
    // Find class of next character, skipping over surrogates and removed
    // characters, or BIDI_CLASS_INVALID at the end of the level run.
    for ( ++i; i < upper; ++i )
    {
        unsigned bc = ub->c.at( i ).bc;
        if ( bc == BC_INVALID || bc == UCDN_BIDI_CLASS_BN )
        {
            continue;
        }

        return bc;
    }

    return BC_INVALID;
}

static void bidi_weak( ual_buffer* ub )
{
    size_t length = ub->level_runs.size() - 1;
    for ( size_t i = 0; i < length; ++i )
    {
        ual_level_run* prun = &ub->level_runs[ i ];
        ual_level_run* nrun = &ub->level_runs[ i + 1 ];

        // Previous strong class.
        unsigned prev_strong = prun->sos;

        // Previous classes after applying rules.
        unsigned prev_w1 = prun->sos;
        unsigned prev_w3 = prun->sos;
        unsigned prev_w5 = prun->sos;

        // Index of first ET in sequence to be updated if we encounter an EN.
        const size_t INVALID_INDEX = (size_t)-1;
        size_t index_et = INVALID_INDEX;

        // Update each of the characters in the run.
        for ( size_t i = prun->start; i < nrun->start; ++i )
        {
            ual_char& c = ub->c.at( i );
            if ( c.bc == BC_INVALID || c.bc == UCDN_BIDI_CLASS_BN )
            {
                continue;
            }

            // TODO: Switch on type first to reduce branching.

            // W1. Update class of NSM to class of previous.
            if ( c.bc == UCDN_BIDI_CLASS_NSM )
            {
                if ( prev_w1 != UCDN_BIDI_CLASS_FSI
                    && prev_w1 != UCDN_BIDI_CLASS_LRI
                    && prev_w1 != UCDN_BIDI_CLASS_RLI
                    && prev_w1 != UCDN_BIDI_CLASS_PDI )
                {
                    c.bc = prev_w1;
                }
                else
                {
                    c.bc = UCDN_BIDI_CLASS_ON;
                }
            }

            prev_w1 = c.bc;

            // W2. Keep track of strong left context.  Change EN to AN if
            // the strong left context is AL.
            if ( c.bc == UCDN_BIDI_CLASS_R || c.bc == UCDN_BIDI_CLASS_L || c.bc == UCDN_BIDI_CLASS_AL )
            {
                prev_strong = c.bc;
            }
            if ( c.bc == UCDN_BIDI_CLASS_EN && prev_strong == UCDN_BIDI_CLASS_AL )
            {
                c.bc = UCDN_BIDI_CLASS_AN;
            }

            // W3. Change AL to R.
            if ( c.bc == UCDN_BIDI_CLASS_AL )
            {
                c.bc = UCDN_BIDI_CLASS_R;
            }

            unsigned self_w3 = c.bc;

            // W4. A single ES between two ENs changes to EN. A single CS
            // between two AN/ENs changes to AN/EN.
            if ( c.bc == UCDN_BIDI_CLASS_ES && prev_w3 == UCDN_BIDI_CLASS_EN
                    && lookahead( ub, i, nrun->start ) == UCDN_BIDI_CLASS_EN )
            {
                c.bc = UCDN_BIDI_CLASS_EN;
            }
            if ( c.bc == UCDN_BIDI_CLASS_CS && prev_w3 == UCDN_BIDI_CLASS_EN
                    && lookahead( ub, i, nrun->start ) == UCDN_BIDI_CLASS_EN )
            {
                c.bc = UCDN_BIDI_CLASS_EN;
            }
            if ( c.bc == UCDN_BIDI_CLASS_CS && prev_w3 == UCDN_BIDI_CLASS_AN
                    && lookahead( ub, i, nrun->start ) == UCDN_BIDI_CLASS_AN )
            {
                c.bc = UCDN_BIDI_CLASS_AN;
            }

            prev_w3 = self_w3;

            // W5. A sequence of ETs adjacent to EN becomes EN.
            if ( c.bc == UCDN_BIDI_CLASS_ET )
            {
                if ( prev_w5 == UCDN_BIDI_CLASS_EN )
                {
                    c.bc = UCDN_BIDI_CLASS_EN;
                }
                else
                {
                    if ( index_et == INVALID_INDEX )
                    {
                        index_et = i;
                    }

                    // Postpone further updates until end of ET sequence.
                    continue;
                }
            }
            else if ( index_et != INVALID_INDEX )
            {
                unsigned change_to;
                if ( c.bc == UCDN_BIDI_CLASS_EN )
                {
                    // W6. A sequence of ETs adjacent to EN becomes EN.
                    change_to = UCDN_BIDI_CLASS_EN;

                    // W7. Change EN to L if the left context is L.
                    if ( prev_strong == UCDN_BIDI_CLASS_L )
                    {
                        change_to = UCDN_BIDI_CLASS_L;
                    }
                }
                else
                {
                    // W6. Otherwise, ETs become ON.
                    change_to = UCDN_BIDI_CLASS_ON;
                }

                while ( index_et < i )
                {
                    ual_char& et = ub->c.at( index_et++ );
                    if ( et.bc == BC_INVALID || et.bc == UCDN_BIDI_CLASS_BN )
                    {
                        continue;
                    }

                    assert( et.bc = UCDN_BIDI_CLASS_ET );
                    et.bc = change_to;
                }

                index_et = INVALID_INDEX;
            }

            prev_w5 = c.bc;

            // W6. Otherwise, ET, ES, or CS become ON.
            assert( c.bc != UCDN_BIDI_CLASS_ET );
            if ( c.bc == UCDN_BIDI_CLASS_ES || c.bc == UCDN_BIDI_CLASS_CS )
            {
                c.bc = UCDN_BIDI_CLASS_ON;
            }

            // W7. Change EN to L if the left context is L.
            if ( c.bc == UCDN_BIDI_CLASS_EN && prev_strong == UCDN_BIDI_CLASS_L )
            {
                c.bc = UCDN_BIDI_CLASS_L;
            }

        }

        // Deal with case where the level run ends with an ET.
        if ( index_et != INVALID_INDEX )
        {
            while ( index_et < nrun->start )
            {
                ual_char& et = ub->c.at( index_et++ );
                if ( et.bc == BC_INVALID || et.bc == UCDN_BIDI_CLASS_BN )
                {
                    continue;
                }

                assert( et.bc = UCDN_BIDI_CLASS_ET );
                et.bc = UCDN_BIDI_CLASS_ON;
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
*/

static void bidi_brackets( ual_buffer* ub )
{
}

/*
    Perform rules N1 and N2, resolving types for runs of neutrals.
*/

static void bidi_neutral( ual_buffer* ub )
{
}

/*
    Perform bidi analysis, generating a final set of bidi runs.
*/

unsigned ual_bidi_analyze( ual_buffer* ub )
{
    ub->level_runs.clear();
    ub->bidi_runs.clear();

    // Look up initial bidi classes.
    unsigned paragraph_level = 0;
    switch ( bidi_lookup( ub ) )
    {
    case BIDI_ALL_LEFT:
        // Entire string is left to right.
        ub->bidi_runs.push_back( { 0, 0 } );
        ub->bidi_runs.push_back( { (unsigned)ub->c.size(), 0 } );
        return 0;

    case BIDI_SOLITARY:
        // There is only one level run at rule X10.
        paragraph_level = bidi_solitary( ub );
        break;

    case BIDI_EXPLICIT:
        // We require full processing of embeddings/overrides/isolates.
        paragraph_level = bidi_explicit( ub );
        break;
    }

    // Perform weak and neutral processing.
    bidi_weak( ub );
    bidi_brackets( ub );
    bidi_neutral( ub );

    // Generate output runs from resolved bidi classes.


    // Done.
    return paragraph_level;
}

ual_bidi_run* ual_bidi_runs( ual_buffer* ub, size_t* out_count )
{
    if ( out_count )
    {
        *out_count = ub->bidi_runs.size();
    }
    return ub->bidi_runs.data();
}

