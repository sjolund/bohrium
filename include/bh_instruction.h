/*
This file is part of Bohrium and copyright (c) 2012 the Bohrium
team <http://www.bh107.org>.

Bohrium is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.

Bohrium is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the
GNU Lesser General Public License along with Bohrium.

If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __BH_INSTRUCTION_H
#define __BH_INSTRUCTION_H

#include "bh_opcode.h"
#include "bh_array.h"
#include "bh_error.h"

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of operands in a instruction.
#define BH_MAX_NO_OPERANDS (3)

//Memory layout of the Bohrium instruction
typedef struct
{
    //Opcode: Identifies the operation
    bh_opcode  opcode;
    //Id of each operand
    bh_view  operand[BH_MAX_NO_OPERANDS];
    //Constant included in the instruction (Used if one of the operands == NULL)
    bh_constant constant;
} bh_instruction;

#ifdef __cplusplus
}
#endif

// Serialize the bh_instruction object
#include <boost/serialization/is_bitwise_serializable.hpp>
BOOST_IS_BITWISE_SERIALIZABLE(bh_instruction)

#endif
