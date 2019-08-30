//
//  ual_break.h
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

#ifndef UAL_BREAK_H
#define UAL_BREAK_H

#include "ual_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
    Perform cluster and line breaking analysis.  After analysis, the char
    buffer will have break flags set at the start of each cluster, and at
    each break opportunity (at the character that would start the new line).

    For each break opportunity, a trailing space flag is set at the space
    character that starts a contiguous run of space characters before the
    break, or at the break opportunity itself if the run is empty.
*/

const uint16_t UAL_BREAK_CLUSTER    = 1 << 0;
const uint16_t UAL_BREAK_LINE       = 1 << 1;
const uint16_t UAL_BREAK_SPACES     = 1 << 2;

void ual_break_analyze( ual_buffer* ub );

#ifdef __cplusplus
}
#endif

#endif

