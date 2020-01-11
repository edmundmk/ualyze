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
const size_t MAX_BRACKET_STACK_DEPTH = 64;

struct ual_bracket
{
    uint32_t closing_bracket : 21;
    uint32_t script : 11;
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

    // Bracket stack.
    ual_bracket bracket_stack[ MAX_BRACKET_STACK_DEPTH ];
    size_t bracket_stack_pointer;

    // Span results.
    std::vector< ual_script_span > script_spans;

    // Bidi results.
    std::vector< ual_bidi_run > bidi_runs;
};

char32_t ual_codepoint( ual_buffer* ub, size_t index );

bool ual_push_bracket( ual_buffer* ub, const ual_bracket& bracket );
void ual_trim_bracket_stack( ual_buffer* ub, size_t index );

#endif

