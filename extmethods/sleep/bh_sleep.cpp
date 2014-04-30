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
#include <assert.h>
#include <unistd.h>

#include "bh_sleep.h"

bh_error bh_sleep(bh_instruction *instr, void* arg)
{
    bh_base* B = instr->operand[0].base;
    bh_int64 *args  = (bh_int64*) instr->operand[1].base->data;
    bh_int64 microseconds = args[0];
    printf("%li, ", microseconds);
    assert(args != NULL);
    assert(instr->operand[1].base->nelem == 1);
    assert(instr->operand[1].base->type == BH_INT64);
    bh_uint8 *ary = (bh_uint8 *)B->data;
    int tsize = bh_type_size(B->type);
    int s = 0;
    for (int i = 0; i < B->nelem; i++)
    {
        //sleep
        ary[i * tsize] += 1;
        if(microseconds && i % 4096 == 0)
        {
            s += 1;
            usleep(microseconds);
        }
        //printf("i: %i %i ", i, l);
    }//
    printf(" %g, ", (float)(s * microseconds)/1000000.0);


    return BH_SUCCESS;
}


bh_error bh_work(bh_instruction *instr, void* arg)
{
    bh_base* B = instr->operand[0].base;
    bh_int64 *args  = (bh_int64*) instr->operand[1].base->data;
    bh_int64 work = args[0];
    printf("%li, ", work);
    assert(args != NULL);
    assert(instr->operand[1].base->nelem == 1);
    assert(instr->operand[1].base->type == BH_INT64);
    bh_uint8 *ary = (bh_uint8 *)B->data;
    int tsize = bh_type_size(B->type);
    int s = 0;
    for (int i = 0; i < B->nelem; i++)
    {
       for (int j=0; j < work; j++)
       {
           ary[i * tsize] += 1;
       }
    }
    printf(" %g, ", (float)(s * work)/1000000.0);


    return BH_SUCCESS;
}
