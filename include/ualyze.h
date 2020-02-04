//
//  ualyze.h
//
//  Created by Edmund Kapusniak on 22/12/2019.
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

#ifndef UALYZE_H
#define UALYZE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#if defined( __GNUC__ )
#define UAL_EXPORT __attribute__(( visibility( "default" ) ))
#define UAL_IMPORT __attribute__(( visibility( "default" ) ))
#elif defined( _MSC_VER )
#define UAL_EXPORT __declspec( dllexport )
#define UAL_IMPORT __declspec( dllimport )
#else
#define UAL_EXPORT
#define UAL_IMPORT
#endif

#if defined( UAL_BUILD_TESTS )
#define UAL_API
#elif defined( UAL_BUILD )
#define UAL_API UAL_EXPORT
#else
#define UAL_API UAL_IMPORT
#endif
#ifdef __cplusplus
extern "C" {
#endif

/*
    A ual_buffer holds the results of analysis.
*/

typedef struct ual_buffer ual_buffer;

UAL_API ual_buffer* ual_buffer_create();
UAL_API ual_buffer* ual_buffer_retain( ual_buffer* ub );
UAL_API void ual_buffer_release( ual_buffer* ub );

/*
    Analysis is performed on UTF-16 text.  The buffer retains an internal
    pointer to the string.  The caller is responsible for keeping the string
    alive during analysis.

    Analysis proceeds on a single paragraph.  Paragraphs are delimited by
    hard line breaks - \n, \r, \r\n, \v, \f, U+0085 NEXT LINE, U+2028 LINE
    SEPARATOR, or U+2029 PARAGRAPH SEPARATOR.

    This function sets up analysis for the text between the start of text and
    the end of the first paragraph separator (or the end of the string).  It
    returns the length of the paragraph, in encoding units.
*/

UAL_API size_t ual_analyze_paragraph( ual_buffer* ub, const char16_t* text, size_t size );

/*
    Direct access to the analysis buffer.  Buffer stores the currently analyzed
    paragraph.  In each ual_char, the bc member is either a (private) bidi
    class after bidi analysis, or contains breaking flags after break analysis.

    Text is a pointer to the same UTF-16 string provided by the client.
*/

typedef struct ual_char
{
    uint16_t ix : 11;   // private
    uint16_t bc : 5;    // bidi class or break flags
} ual_char;


UAL_API const char16_t* ual_buffer_text( ual_buffer* ub );
UAL_API const ual_char* ual_buffer_chars( ual_buffer* ub );
UAL_API size_t ual_buffer_size( ual_buffer* ub );

/*
    Perform cluster and line breaking analysis.  After analysis, the char
    buffer will have break flags set at the start of each cluster, and at each
    break opportunity (at the character that would start the new line).

    For each break opportunity, a space flag is set at the space character that
    starts a contiguous run of space characters before it.  Space characters
    are those in Unicode category Zs as well as line break categories BK, CR,
    LF, and NL (hard line break characters), or ZW (zero width space).
*/

const uint16_t UAL_BREAK_CLUSTER    = 1 << 0;
const uint16_t UAL_BREAK_LINE       = 1 << 1;
const uint16_t UAL_BREAK_SPACES     = 1 << 2;

UAL_API const ual_char* ual_analyze_breaks( ual_buffer* ub );

/*
    Split the paragraph into spans containing runs of the same script.  The
    script code is a 4-character identifier from ISO 15924, with the first
    character in the high byte (this is the same as Harfbuzz).

    Active script analysis locks the internal stack.  Bidi analysis cannot
    be run at the same time.
*/

typedef struct ual_script_span
{
    size_t lower;
    size_t upper;
    uint32_t script;
} ual_script_span;

UAL_API void ual_script_spans_begin( ual_buffer* ub );
UAL_API bool ual_script_spans_next( ual_buffer* ub, ual_script_span* out_span );
UAL_API void ual_script_spans_end( ual_buffer* ub );

/*
    Perform bidi analysis on a paragraph.  Sets the bc value to the resolved
    bidi class of each character.  Returns the paragraph level.

    This routine requires the internal stack.  Script analysis cannot be run
    at the same time.
*/

UAL_API unsigned ual_analyze_bidi( ual_buffer* ub );

/*
    With resolved bidi classes,  split the paragraph into bidi runs.  This
    process does *not* require the internal stack.
*/

typedef struct ual_bidi_run
{
    size_t lower;
    size_t upper;
    unsigned level;
} ual_bidi_run;

UAL_API void ual_bidi_runs_begin( ual_buffer* ub );
UAL_API bool ual_bidi_runs_next( ual_buffer* ub, ual_bidi_run* out_run );
UAL_API void ual_bidi_runs_end( ual_buffer* ub );

#ifdef __cplusplus
}
#endif

#endif

