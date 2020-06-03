//
//  ucdb_script.h
//
//  Created by Edmund Kapusniak on 03/06/2020.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
//

#ifndef UCDB_SCRIPT_H
#define UCDB_SCRIPT_H

#include <stdint.h>
#include "generated/script_enum.h"

// This table contains the 4-character ISO 15924 code for each script, packed
// into a 32-bit integer with the first character in the high byte.

extern const uint32_t UCDB_SCRIPT_CODE[];

#endif

