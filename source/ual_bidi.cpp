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
    return 0;
}

/*
    Peform rules X1 to X10.  This identifies level runs and isolating run
    sequences.  The bidi classes of characters are modified by overrides.
    Characters 'removed' by rule X9 have their class set to BN.

    Return the paragraph embedding level.
*/

static unsigned bidi_explicit( ual_buffer* ub )
{
    return 0;
}

/*
    Perform rules W1 to W7 on the characters in each level run.  We do a single
    pass through the string.
*/

static void bidi_weak( ual_buffer* ub )
{
}

/*
    Perform rules N0 to N2, including bracket pair identification.
*/

static void bidi_neutral( ual_buffer* ub )
{
}

/*
    Perform bidi analysis, generating a final set of bidi runs.
*/

unsigned ual_bidi_analyze( ual_buffer* ub )
{
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

