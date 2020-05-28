//
//  ucdu.cpp
//
//  Created by Edmund Kapusniak on 28/05/2020.
//  Copyright © 2020 Edmund Kapusniak. All rights reserved.
//

#include "ucdu.h"
#include <algorithm>
#include "ucdu_table.h"

size_t ucdu_lookup( char32_t c )
{
    return ucdu_index_get( c );
}

ucdu_bracket_kind ucdu_paired_bracket( char32_t bracket, char32_t* out_paired )
{
    auto i = std::lower_bound
    (
        std::begin( UCDU_BRACKETS ),
        std::end( UCDU_BRACKETS ),
        bracket,
        []( const ucdu_bracket_record& record, char32_t bracket ) { return record.bracket < bracket; }
    );

    if ( i == std::end( UCDU_BRACKETS ) || i->bracket != bracket )
    {
        return UCDU_BRACKET_NONE;
    }

    *out_paired = i->paired;
    return (ucdu_bracket_kind)i->kind;
}

