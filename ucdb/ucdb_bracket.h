//
//  ucdb_bracket.h
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

#ifndef UCDB_BRACKET_H
#define UCDB_BRACKET_H

// Map a codepoint which is potentially a bracket to its canonical
// decomposition.  Only brackets are mapped.

char32_t ucdb_mapped_bracket( char32_t cp );

// Return the bracket type for a codepoint and its resulting paired bracket.

enum ucdb_bracket_kind
{
    UCDB_BRACKET_NONE,
    UCDB_BRACKET_OPEN,
    UCDB_BRACKET_CLOSE,
};

ucdb_bracket_kind ucdb_paired_bracket( char32_t cp, char32_t* out_paired );

#endif
