//
//  ual_buffer.h
//
//  Created by Edmund Kapusniak on 23/08/2019.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
//

#ifndef UAL_BUFFER_H
#define UAL_BUFFER_H

#include "ualyze.h"
#include <string>
#include <vector>
#include <ucdn.h>

const uint16_t IX_INVALID = ( 1 << 11 ) - 1;
const size_t INVALID_INDEX = ~(size_t)0;
const size_t STACK_BYTES = sizeof( unsigned ) * 128;

enum ual_bc_usage
{
    BC_NONE,
    BC_BREAK_FLAGS,
    BC_BIDI_CLASS,
};

struct ual_script_analysis
{
    size_t index;
    unsigned script;
    unsigned sp;
};

enum ual_bidi_complexity
{
    BIDI_ALL_LEFT,  // Paragraph is left-to-right.
    BIDI_ALL_DONE,  // Paragraph is left-to-right, but reported it already.
    BIDI_SOLITARY,  // No directional embeddings, overrides, or isolates.
    BIDI_EXPLICIT,  // Requires full processing.
};

struct ual_bidi_analysis
{
    size_t ilrun;
    size_t index;
    unsigned paragraph_level;
    ual_bidi_complexity complexity;
};

struct ual_level_run
{
    unsigned start;         // index of first character in run.
    unsigned level  : 8;    // bidi level of run.
    unsigned sos    : 2;    // sos type (L, R, AL, or BC_SEQUENCE).
    unsigned eos    : 2;    // eos type (L, R, AL, or BC_SEQUENCE).
    unsigned inext  : 20;   // index of next level run in isolating sequence.
};

struct ual_buffer
{
    ual_buffer();
    ~ual_buffer();

    // Reference count.
    intptr_t refcount;

    // UCDN record table.
    const UCDRecord* ucdn;

    // Text data.
    std::u16string s;
    std::vector< ual_char > c;
    ual_bc_usage bc_usage;
    ual_paragraph p;

    // Current analysis state.
    ual_script_analysis script_analysis;
    ual_bidi_analysis bidi_analysis;
    std::vector< ual_level_run > level_runs;

    // Stack bytes.
    char stack_bytes[ STACK_BYTES ];
};

char32_t ual_codepoint( ual_buffer* ub, size_t index );

template < typename T, size_t count >
inline T* ual_stack( ual_buffer* ub )
{
    static_assert( sizeof( T ) * count <= sizeof( ub->stack_bytes ) );
    return (T*)ub->stack_bytes;
}

#endif

