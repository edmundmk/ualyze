//
//  ucdb_script.h
//
//  Created by Edmund Kapusniak on 03/06/2020.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the ISC License. See LICENSE file in the project root for
//  full license information.
//

#include "ucdb_script.h"

#define SCRIPT_CODE( a, b, c, d ) ( (uint32_t)a << 24 ) | ( (uint32_t)b << 16 ) | ( (uint32_t)c << 8 ) | (uint32_t)d
#include "generated/script_data.h"

