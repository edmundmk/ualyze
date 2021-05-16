//
//  ucdb_bracket.cpp
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

#include "ucdb_bracket.h"
#include <stdint.h>
#include <algorithm>

struct ucdb_mapped_entry
{
    uint16_t cp;
    uint16_t mapped;
};

struct ucdb_paired_entry
{
    uint16_t cp;
    uint16_t paired;
    uint16_t kind;
};

#include "generated/bracket_data.h"

char32_t ucdb_mapped_bracket( char32_t cp )
{
    auto end = std::end( UCDB_BRACKET_MAPPED );
    for ( auto i = std::begin( UCDB_BRACKET_MAPPED ); i < end; ++i )
    {
        if ( i->cp == cp )
        {
            return i->mapped;
        }
    }

    return cp;
}

ucdb_bracket_kind ucdb_paired_bracket( char32_t cp, char32_t* out_paired )
{
    auto i = std::lower_bound
    (
        std::begin( UCDB_BRACKET_PAIRED ),
        std::end( UCDB_BRACKET_PAIRED ),
        cp,
        []( const ucdb_paired_entry& entry, char32_t cp ) { return entry.cp < cp; }
    );

    if ( i == std::end( UCDB_BRACKET_PAIRED ) || i->cp != cp )
    {
        return UCDB_BRACKET_NONE;
    }

    *out_paired = i->paired;
    return (ucdb_bracket_kind)i->kind;
}
