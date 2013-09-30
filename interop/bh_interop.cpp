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
#include <bh_ir.h>
#include <bh_array.h>
#include <bh_instruction.h>

#include "bh_interop.h" 


/* Creates a bh_base entry
 *
 * @type  The type of the data array
 * @nelem The number of elements in the array
 * @data  The number of data elements
 * @base  A reference to storage where the pointer will be stored
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_base_create(bh_type type, bh_index nelem, bh_data_ptr data, bh_base** base)
{
    if (base == NULL)
        return BH_ERROR;

    bh_base* v = (bh_base*)malloc(sizeof(bh_base));
    if (v == NULL)
        return BH_OUT_OF_MEMORY;
    
    v->type = type;
    v->nelem = nelem;
    v->data = data;
    
    *base = v;
    return BH_SUCCESS;
}

/* Gets the type value of the bh_base
 * 
 * @base The base pointer
 * @type A pointer to storage for the returned type value
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_base_get_type(bh_base* base, bh_type* type)
{
    if (base == NULL)
        return BH_ERROR;

    *type = base->type;
    return BH_SUCCESS;
}

/* Sets the type value of the bh_base
 * 
 * @base The base pointer
 * @type The new type value
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_base_set_type(bh_base* base, bh_type type)
{
    if (base == NULL)
        return BH_ERROR;

    base->type = type;
    return BH_SUCCESS;
}

/* Gets the number of elements in the bh_base
 * 
 * @base The base pointer
 * @nelem A pointer to storage for the returned nelem value
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_base_get_nelem(bh_base* base, bh_index* nelem)
{
    if (base == NULL)
        return BH_ERROR;

    *nelem = base->nelem;
    return BH_SUCCESS;
}
/* Sets the number of elements in the bh_base
 * 
 * @base The base pointer
 * @nelem The new nelem value
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_base_set_nelem(bh_base* base, bh_index nelem)
{
    if (base == NULL)
        return BH_ERROR;

    base->nelem = nelem;
    return BH_SUCCESS;
}

/* Gets the data value of the bh_base
 * 
 * @base The base pointer
 * @data A pointer to storage for the returned data value
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_base_get_data(bh_base* base, bh_data_ptr* data)
{
    if (base == NULL)
        return BH_ERROR;

    *data = base->data;
    return BH_SUCCESS;
}
/* Sets the data value of the bh_base
 * 
 * @base The base pointer
 * @data The new data value
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_base_set_data(bh_base* base, bh_data_ptr data)
{
    if (base == NULL)
        return BH_ERROR;

    base->data = data;
    return BH_SUCCESS;
}

/* Destroys an allocated base pointer obtained by
 * a previous call to bh_interop_base_create
 *
 * @base The base pointer
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_base_destroy(bh_base* base)
{
    if (base == NULL)
        return BH_ERROR;
    
    free(base);
    return BH_SUCCESS;
}


/* Creates a new instruction list
 *
 * @nelem        The number of instructions in the list
 * @instructions A pointer to storage for the returned list
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_instructionlist_create(bh_index nelem, bh_instruction** instructions)
{
    if (instructions == NULL)
        return BH_ERROR;

    bh_instruction* v = (bh_instruction*)malloc(sizeof(bh_instruction) * nelem);
    if (v == NULL)
        return BH_OUT_OF_MEMORY;
        
    *instructions = v;
    return BH_SUCCESS;
}


/* Resizes an instruction list previously created with
 * bh_interop_instructionlist_create
 *
 * @instructions A pointer to the list of instructions
 * @nelem        The new number of instructions in the list
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_instructionlist_resize(bh_instruction** instructions, bh_index nelem)
{
    if (instructions == NULL)
        return BH_ERROR;

    bh_instruction* v = (bh_instruction*)realloc(*instructions, sizeof(bh_instruction) * nelem);
    if (v == NULL)
        return BH_OUT_OF_MEMORY;
    
    *instructions = v;
    return BH_SUCCESS;
}


/* Sets the values for an instruction
 *
 * @instructions A pointer to the list of instructions
 * @instr        The instruction to modify
 * @opcode       The instruction opcode
 * @constant     The instruction constant
 * @userfunc     The userfunc pointer or NULL
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_instructionlist_set(bh_instruction* instructions, bh_index instr, bh_opcode opcode, bh_constant constant, bh_userfunc* userfunc)
{
    if (instructions == NULL)
        return BH_ERROR;

    bh_instruction* v = &instructions[instr];
    v->opcode = opcode;
    v->constant = constant;
    v->userfunc = userfunc;
    
    return BH_SUCCESS;
}

/* Gets the opcode for an instruction
 *
 * @instructions A pointer to the list of instructions
 * @instr        The instruction to retrieve
 * @opcode       The returned instruction opcode
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_instructionlist_get_opcode(bh_instruction* instructions, bh_index instr, bh_opcode* opcode)
{
    if (instructions == NULL || opcode == NULL)
        return BH_ERROR;

    *opcode = instructions[instr].opcode;
    return BH_SUCCESS;
}

/* Gets the userfunc for an instruction
 *
 * @instructions A pointer to the list of instructions
 * @instr        The instruction to retrieve
 * @userfunc     The returned instruction userfunc
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_instructionlist_get_userfunc(bh_instruction* instructions, bh_index instr, bh_userfunc** userfunc)
{
    if (instructions == NULL || userfunc == NULL)
        return BH_ERROR;

    *userfunc = instructions[instr].userfunc;
    return BH_SUCCESS;
}

/* Gets the operand base for an instruction
 *
 * @instructions A pointer to the list of instructions
 * @instr        The instruction to retrieve
 * @operand      The operand to get, allowed range is [0 - BH_MAX_NO_OPERANDS[
 * @opcode       The returned instruction opcode
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_instructionlist_get_operandbase(bh_instruction* instructions, bh_index instr, bh_index operand, bh_base** base)
{
    if (instructions == NULL || base == NULL)
        return BH_ERROR;

    if (operand < 0 || operand >= BH_MAX_NO_OPERANDS)
        return BH_ERROR;

    *base = instructions[instr].operand[operand].base;
    return BH_SUCCESS;    
}

/* Sets the operand values for an instruction
 * 
 * @instructions A pointer to the list of instructions
 * @instr        The instruction to modify
 * @operand      The operand to set allowed range is [0 - MAX_OPERANDS]
 * @base         The operand base pointer
 * @ndim         The number of dimensions in the view
 * @start        The data offset
 * @shape        The shape sizes
 * @stride       The stride sizes
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_instructionlist_set_view(bh_instruction* instructions, bh_index instr, bh_index operand, bh_base* base, bh_intp ndim, bh_index start, bh_index* shape, bh_index* stride)
{
    if (instructions == NULL)
        return BH_ERROR;

    if (operand < 0 || operand >= BH_MAX_NO_OPERANDS)
        return BH_ERROR;

    bh_view* v = &(instructions[instr].operand[operand]);
    v->base = base;
    v->ndim = ndim;
    v->start = start;
    memcpy(v->shape, shape, sizeof(bh_index) * ndim);
    memcpy(v->stride, stride, sizeof(bh_index) * ndim);
    
    return BH_SUCCESS;
}

/* Sets the constant value for an instruction
 * 
 * @instructions A pointer to the list of instructions
 * @instr        The instruction to modify
 * @operand      The operand to set allowed range is [0 - BH_MAX_NO_OPERANDS[
 * @constant     The constant value to use
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_instructionlist_set_constant(bh_instruction* instructions, bh_index instr, bh_index operand, bh_constant constant)
{
    if (instructions == NULL)
        return BH_ERROR;

    if (operand < 0 || operand >= BH_MAX_NO_OPERANDS)
        return BH_ERROR;

    bh_view* v = &(instructions[instr].operand[operand]);
    bh_flag_constant(v);

    instructions[instr].constant = constant;

    return BH_SUCCESS;
}

/* Sets the operand to an empty view for an instruction
 * 
 * @instructions A pointer to the list of instructions
 * @instr        The instruction to modify
 * @operand      The operand to set allowed range is [0 - BH_MAX_NO_OPERANDS[
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_instructionlist_set_view_empty(bh_instruction* instructions, bh_index instr, bh_index operand)
{
    if (instructions == NULL)
        return BH_ERROR;

    if (operand < 0 || operand >= BH_MAX_NO_OPERANDS)
        return BH_ERROR;

    bh_view* v = &(instructions[instr].operand[operand]);
    v->ndim = 0;
    v->base = NULL;

    return BH_SUCCESS;
}

/* Destroys an instruction list previously created with
 * bh_interop_instructionlist_create
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_instructionlist_destroy(bh_instruction* instructions)
{
    if (instructions == NULL)
        return BH_ERROR;

    free(instructions);
    return BH_SUCCESS;
}



/* Allocates space for a bh_ir struct,
 * and initializes it with the instruction list
 *
 * @bh_ir       A reference to storage where the pointer will be stored
 * @ninstr       The number of instructions in the list
 * @instructions An array of instructions
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_ir_create(bh_ir** bhir, bh_index ninstr, bh_instruction* instructions)
{
    bh_ir* v;
    bh_error e;

    e = bh_interop_ir_malloc(&v);
    if (e != BH_SUCCESS)
        return e;

    e = bh_ir_create(v, ninstr, instructions);
    if (e != BH_SUCCESS)
    {
        bh_interop_ir_free(v);
        return e;
    }

    *bhir = v;
    return BH_SUCCESS;
}

/* Destroys a bh_ir struct, created by a previous
 * call to bh_interop_ir_create
 *
 * @bh_ir The BhIR handle
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_ir_destroy(bh_ir* bhir)
{
    if (bhir == NULL)
        return BH_ERROR;

    bh_ir_destroy(bhir);
    return bh_interop_ir_free(bhir);
}


/* Allocates space for a bh_ir struct
 *
 * @bh_ir A reference to storage where the pointer will be stored
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_ir_malloc(bh_ir** bhir)
{
    bh_ir* v = (bh_ir*)malloc(sizeof(bh_ir));
    if (v == NULL)
        return BH_OUT_OF_MEMORY;
    
    *bhir = v;
    return BH_SUCCESS;
}

/* Frees the memory allocated by a previous
 * call to bh_interop_ir_malloc or bh_interop_ir_create
 *
 * @bhir The BhIR handle
 *
 * @return BH_SUCCESS if the operation succeeded, any other value indicates error
 */
bh_error bh_interop_ir_free(bh_ir* bhir)
{
    if (bhir == NULL)
        return BH_ERROR;
    free(bhir);
    return BH_SUCCESS;
}



