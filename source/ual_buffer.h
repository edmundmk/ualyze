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

const uint16_t IX_INVALID = ( 1 << 11 ) - 1;

const unsigned BOUNDARY_SEQ = 1;
const unsigned NEXT_INVALID = ( 1 << 20 ) - 1;

struct ual_level_run
{
    unsigned start;     // index of first character in run.
    unsigned inext;     // index of next level run in isolating run sequence
    uint8_t level;      // bidi level of run.
    uint8_t sos;        // sos type
    uint8_t eos;        // eos type
};

struct ual_buffer
{
    ual_buffer();
    ~ual_buffer();

    // Reference count.
    intptr_t refcount;

    // Text data.
    std::u16string s;
    std::vector< ual_char > c;
    ual_paragraph p;

    // Stack memory.
    void* stack_bytes;

    // Span results.
    std::vector< ual_script_span > script_spans;

    // Bidi results.
    std::vector< ual_level_run > level_runs;
    std::vector< ual_bidi_run > bidi_runs;
};

void* ual_stack_bytes( ual_buffer* ub, size_t count, size_t size );
char32_t ual_codepoint( ual_buffer* ub, size_t index );

#endif

