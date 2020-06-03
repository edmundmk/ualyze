//
//  ucdb_table.h
//
//  Created by Edmund Kapusniak on 03/06/2020.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
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

