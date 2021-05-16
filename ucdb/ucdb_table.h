//
//  ucdb_table.h
//
//  Created by Edmund Kapusniak on 03/06/2020.
//
//  Copyright © 2020 Edmund Kapusniak.
//  Copyright (C) 2012 Grigori Goronzy <greg@kinoho.net>
//
//  Permission to use, copy, modify, and/or distribute this software for any
//  purpose with or without fee is hereby granted, provided that the above
//  copyright notice and this permission notice appear in all copies.
//
//  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
//  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
//  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
//  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
//  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
//  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
//  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#ifndef UCDB_TABLE_H
#define UCDB_TABLE_H

#include <stddef.h>
#include <stdint.h>
#include "generated/table_enum.h"

// Map each codepoint to an entry in the table containing its properties.

struct ucdb_entry
{
    uint8_t script;
    uint8_t bclass;
    uint8_t lbreak : 7;
    uint8_t zspace : 1;
    uint8_t cbreak : 7;
    uint8_t paired : 1;
};

extern const ucdb_entry UCDB_TABLE[];
size_t ucdb_lookup( char32_t cp );

#endif

