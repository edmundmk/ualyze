//
//  ucdb_table.cpp
//
//  Created by Edmund Kapusniak on 03/06/2020.
//  Copyright © 2020 Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
//

#include "ucdb_table.h"
#include "generated/script_enum.h"
#include "generated/table_data.h"

size_t ucdb_lookup( char32_t c )
{
    return ucdb_index( c );
}
