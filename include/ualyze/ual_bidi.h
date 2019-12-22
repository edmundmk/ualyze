//
//  ual_bidi.h
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

#ifndef UAL_BIDI_H
#define UAL_BIDI_H

#include "ual_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
    Find bidi paragraph level.
*/

unsigned ual_bidi_paragraph_level( ual_buffer* ub );

/*
    Perform bidi analysis on a paragraph.  Will clobber the break flags.
    The result of bidi analysis is a set of bidi runs containing characters
    with the same bidi level.
*/

typedef struct ual_bidi_run
{
    unsigned lower;
    unsigned upper;
    unsigned bidi_level;
} ual_bidi_run;

unsigned ual_bidi_analyze( ual_buffer* ub );
ual_bidi_run* ual_bidi_runs( ual_buffer* ub, size_t* out_count );

#ifdef __cplusplus
}
#endif

#endif

