//
//  ucdb_bracket.h
//
//  Created by Edmund Kapusniak on 03/06/2020.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
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
