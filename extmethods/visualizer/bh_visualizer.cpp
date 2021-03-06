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
#include <bh.h>
#include <assert.h>
#include <iostream>

#include <iostream>
#include "bh_visualizer.h"
#include "visualizer.hpp"

/**
 *
 * Implementation of the user-defined function "visualize".
 * Note that we follow the function signature defined by bh_userfunc_impl.
 *
 */
bool bh_visualize_initialized = false;
bh_error bh_visualizer(bh_instruction *instr, void* arg)
{
    bh_view *subject = &instr->operand[0];

    bh_float32 *args;
    bh_data_get(&(instr->operand[1]), (void**) &args);
    
    assert(args != NULL);
    assert(instr->operand[1].base->nelem == 5);
    assert(instr->operand[1].base->type == BH_FLOAT32);

    for(bh_intp i=0; i<subject->ndim; ++i) {
        if(subject->shape[i] < 16) {
           fprintf(stderr, "Cannot visualize because of input shape\n");
           return BH_ERROR;
        }
    }
    if (subject->base->data == NULL) {
        fprintf(stderr, "You are trying to visualize non-existing data.\n");
        return BH_ERROR;
    }

    bh_int32 cm    = args[0];
    bh_bool flat   = args[1];
    bh_bool cube   = args[2];
    bh_float32 min = args[3];
    bh_float32 max = args[4];

    if (!bh_visualize_initialized) {
        if (subject->ndim == 3) {
            Visualizer::getInstance().setValues(
                subject,
                subject->shape[0],
                subject->shape[1],
                subject->shape[2],
                cm, flat, cube, min, max
            );
        } else {
            Visualizer::getInstance().setValues(
                subject,
                subject->shape[0],
                subject->shape[1],
                1,
                cm, flat, cube, min, max
            );
        }
        bh_visualize_initialized = true;
    }

    Visualizer::getInstance().run(subject);
    
    return BH_SUCCESS;
}
