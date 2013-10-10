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

#include <cassert>
#include <cstring>
#include <iostream>
#include <bh.h>
#include <map>

#include "bh_vem_intro.h"
#include "bh_marray.h"
#include "bh_minstruction.h"

//Function pointers to the VE.
static bh_init ve_init;
static bh_execute ve_execute;
static bh_shutdown ve_shutdown;
static bh_reg_func ve_reg_func;

//The VE components
static bh_component **vem_node_components;

//Our self
static bh_component *vem_node_myself;

//Number of user-defined functions registered.
static bh_intp userfunc_count = 0;

//The timing ID for executions
static bh_intp exec_timing;

#ifdef BH_TIMING
    //Number of elements executed
    static bh_intp total_execution_size = 0;
#endif

/* Initialize the VEM
 *
 * @return Error codes (BH_SUCCESS)
 */
bh_error bh_vem_intro_init(bh_component *self)
{
    printf("bh_vem_intro_init()\n");
    bh_intp children_count;
    bh_error err;
    vem_node_myself = self;

    err = bh_component_children(self, &children_count, &vem_node_components);
    if (children_count != 1)
    {
		std::cerr << "Unexpected number of child nodes for VEM, must be 1" << std::endl;
		return BH_ERROR;
    }

    if (err != BH_SUCCESS)
	    return err;

    ve_init = vem_node_components[0]->init;
    ve_execute = vem_node_components[0]->execute;
    ve_shutdown = vem_node_components[0]->shutdown;
    ve_reg_func = vem_node_components[0]->reg_func;

    //Let us initiate the simple VE and register what it supports.
    if((err = ve_init(vem_node_components[0])) != 0)
        return err;

    exec_timing = bh_timing_new("node-execution");

    return BH_SUCCESS;
}


/* Shutdown the VEM, which include a instruction flush
 *
 * @return Error codes (BH_SUCCESS)
 */
bh_error bh_vem_intro_shutdown(void)
{
    bh_error err;
    err = ve_shutdown();
    bh_component_free(vem_node_components[0]);//Only got one child.
    ve_init     = NULL;
    ve_execute  = NULL;
    ve_shutdown = NULL;
    ve_reg_func = NULL;
    bh_component_free_ptr(vem_node_components);
    vem_node_components = NULL;

    return err;
}


/* Register a new user-defined function.
 *
 * @lib Name of the shared library e.g. libmyfunc.so
 *      When NULL the default library is used.
 * @fun Name of the function e.g. myfunc
 * @id Identifier for the new function. The bridge should set the
 *     initial value to Zero. (in/out-put)
 * @return Error codes (BH_SUCCESS)
 */
bh_error bh_vem_intro_reg_func(const char *fun, bh_intp *id)
{
	bh_error e;

    if(*id == 0)//Only if parent didn't set the ID.
        *id = ++userfunc_count;

    if((e = ve_reg_func(fun, id)) != BH_SUCCESS)
    {
        *id = 0;
        return e;
    }
    return e;
}

static bool is_reduction(bh_opcode op)
{
    switch (op) {
        case BH_ADD_REDUCE:
        case BH_MULTIPLY_REDUCE:
        case BH_MINIMUM_REDUCE:
        case BH_MAXIMUM_REDUCE:
        case BH_LOGICAL_AND_REDUCE:
        case BH_BITWISE_AND_REDUCE:
        case BH_LOGICAL_OR_REDUCE:
        case BH_LOGICAL_XOR_REDUCE:
        case BH_BITWISE_OR_REDUCE:
        case BH_BITWISE_XOR_REDUCE:
            return true;
        default:
            return false;
    }
}


/* Execute a list of instructions (blocking, for the time being).
 * It is required that the VEM supports all instructions in the list.
 *
 * @instruction A list of instructions to execute
 * @return Error codes (BH_SUCCESS)
 */
bh_error bh_vem_intro_execute(bh_intp count,
                              bh_minstruction inst_list[])
{
    printf("bh_vem_intro_execute\n");

    if (count <= 0)
        return BH_SUCCESS;

    std::map<bh_array*,bh_base*> ary2base;
    bh_instruction new_inst[count];
    for(bh_intp i=0; i<count; ++i)
    {
        bh_minstruction* old_i = &inst_list[i];
        bh_instruction* new_i  = &new_inst[i];
        new_i->opcode = old_i->opcode;
        for(bh_intp o=0; o<bh_operands(old_i->opcode); ++o)
        {
            bh_array *old_o = old_i->operand[o];
            bh_view  *new_o = &new_i->operand[o];
            if(old_o == NULL)//is a constant
            {
                new_i->constant = old_i->constant;
                bh_flag_constant(new_o);
                continue;
            }
            bh_array *old_b = (old_o->base == NULL)?old_o:old_o->base;

            if(old_i->opcode == BH_DISCARD && old_b != old_o && 0)
            {
                new_i->opcode = BH_NONE;
                printf("DISCARD\n");
                break;
            }
            bh_base *new_b;
            if(ary2base.find(old_b) == ary2base.end())
            {
                new_b = (bh_base*) malloc(sizeof(bh_base));
                assert(new_b != NULL);
                new_b->type  = old_b->type;
                new_b->nelem = bh_nelements(old_b->ndim, old_b->shape);
                new_b->data  = old_b->data;
                ary2base[old_b] = new_b;
            }
            else
            {
                new_b = ary2base[old_b];
                assert(new_b != NULL);
            }
            new_o->base  = new_b;
            new_o->ndim  = old_o->ndim;
            new_o->start = old_o->start;
            memcpy(new_o->shape, old_o->shape, sizeof(bh_intp) * old_o->ndim);
            memcpy(new_o->stride, old_o->stride, sizeof(bh_intp) * old_o->ndim);
        }
        if(is_reduction(old_i->opcode))
        {
            new_i->constant = old_i->constant;
            bh_flag_constant(&new_i->operand[2]);
        }
    }
    bh_pprint_instr_list(new_inst, count, "INPUT");


    bh_ir bhir;
    bh_error error = bh_ir_create(&bhir, count, new_inst);
    assert(error == BH_SUCCESS);
    error = ve_execute(&bhir);
    assert(error == BH_SUCCESS);
    assert(bhir.ninstr == count);
    bh_instruction *ret_inst = bhir.instr_list;
    bh_pprint_instr_list(ret_inst, count, "OUTPUT");


    for(bh_intp i=0; i<count; ++i)
    {
        bh_instruction* new_i  = &ret_inst[i];
        bh_minstruction* old_i = &inst_list[i];

        for(bh_intp o=0; o<bh_operands(old_i->opcode); ++o)
        {
            bh_view  *new_o = &new_i->operand[o];
            bh_array *old_o = old_i->operand[o];
            if(bh_is_constant(new_o))
            {
                assert(old_o == NULL);
                continue;
            }
            bh_array *old_b = (old_o->base == NULL)?old_o:old_o->base;
            error = bh_data_get(new_o,&old_b->data);
            assert(error == BH_SUCCESS);
        }

    }


    bh_ir_destroy(&bhir);


    return 0;
}
