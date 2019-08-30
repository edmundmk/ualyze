//
//  ual_buffer.h
//
//  Created by Edmund Kapusniak on 22/08/2019.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//

#ifndef UAL_BUFFER_H
#define UAL_BUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <uchar.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
    All analysis happens on text in a ual_buffer.  Text is always UTF-16.
*/

typedef uint32_t ual_index;
typedef struct ual_buffer ual_buffer;

typedef struct ual_string_view
{
    const char16_t* data;
    size_t size;
} ual_string_view;

ual_buffer* ual_buffer_create();
void ual_buffer_retain( ual_buffer* ub );
void ual_buffer_release( ual_buffer* ub );

void ual_buffer_clear();
void ual_buffer_append( ual_buffer* ub, ual_string_view text );
ual_string_view ual_buffer_text( ual_buffer* ub, size_t lower, size_t upper );

/*
    Analysis proceeds one paragraph at a time.  Paragraphs are delimited by
    hard line breaks - \n, \r, \r\n, \v, \f, U+0085 NEXT LINE, U+2028 LINE
    SEPARATOR, or U+2029 PARAGRAPH SEPARATOR.

    All indexes into the text produced by other analysis stages are relative
    to the current paragraph.  Spans of text are always a half-open interval
    lower <= index < upper.
*/

typedef struct ual_paragraph
{
    ual_index lower;
    ual_index upper;
} ual_paragraph;

bool ual_next_paragraph( ual_buffer* ub, ual_paragraph* out_paragraph );
ual_string_view ual_paragraph_text( ual_buffer* ub );

/*
    Direct access to the analysis buffer.  Only valid for the currently
    analyzed paragraph.  The bc member is either a (private) bidi class after
    bidi analysis, or contains breaking flags after break analysis.
*/

typedef struct ual_char
{
    uint16_t ix : 11;   // private
    uint16_t bc : 5;    // bidi class or break flags
} ual_char;

ual_char* ual_char_buffer( ual_buffer* ub, size_t* out_count );

#ifdef __cplusplus
}
#endif

#endif

