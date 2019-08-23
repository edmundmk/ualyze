//
//  ual_internal.h
//
//  Created by Edmund Kapusniak on 23/08/2019.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
//


#ifndef UAL_INTERNAL_H
#define UAL_INTERNAL_H

#include "ual_buffer.h"
#include <string>
#include <vector>

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
};

#endif

