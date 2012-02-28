/*
 * Copyright 2011 Troels Blum <troels@blum.dk>
 *
 * This file is part of cphVB.
 *
 * cphVB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cphVB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cphVB. If not, see <http://www.gnu.org/licenses/>.
 */

#include <cphvb.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

/* Reduce nDarray info to a base shape
 *
 * Remove dimentions that just indicate orientation in a
 * higher dimention (Py:newaxis)
 *
 * @ndim          Number of dimentions
 * @shape[]       Number of elements in each dimention.
 * @stride[]      Stride in each dimention.
 * @base_ndim     Placeholder for base number of dimentions
 * @base_shape    Placeholder for base number of elements in each dimention.
 * @base_stride   Placeholder for base stride in each dimention.
 */
void cphvb_base_shape(cphvb_intp ndim,
                      const cphvb_index shape[],
                      const cphvb_index stride[],
                      cphvb_intp* base_ndim,
                      cphvb_index* base_shape,
                      cphvb_index* base_stride)
{
    *base_ndim = 0;
    for (int i = 0; i < ndim; ++i)
    {
        // skipping (shape[i] == 1 && stride[i] == 0)
        if (shape[i] != 1 || stride[i] != 0)
        {
            base_shape[*base_ndim] = shape[i];
            base_stride[*base_ndim] = stride[i];
            ++(*base_ndim);
        }
    }
}

/* Is the data accessed continuously, and only once
 *
 * @ndim     Number of dimentions
 * @shape[]  Number of elements in each dimention.
 * @stride[] Stride in each dimention.
 * @return   Truth value indicating continuousity.
 */
bool cphvb_is_continuous(cphvb_intp ndim,
                         const cphvb_index shape[],
                         const cphvb_index stride[])
{
    cphvb_intp my_ndim = 0;
    cphvb_index my_shape[ndim];
    cphvb_index my_stride[ndim];
    cphvb_base_shape(ndim, shape, stride, &my_ndim, my_shape, my_stride);
    for (int i = 0; i < my_ndim - 1; ++i)
    {
        if (my_shape[i+1] != my_stride[i])
            return true;
    }
    if (my_stride[my_ndim-1] != 1)
        return false;

    return true;
}

/* Number of element in a given shape
 *
 * @ndim     Number of dimentions
 * @shape[]  Number of elements in each dimention.
 * @return   Number of element operations
 */
cphvb_index cphvb_nelements(cphvb_intp ndim,
                            const cphvb_index shape[])
{
    cphvb_index res = 1;
    for (int i = 0; i < ndim; ++i)
    {
        res *= shape[i];
    }
    return res;
}

/* Calculate the dimention boundries for shape
 *
 * @ndim      Number of dimentions
 * @shape[]   Number of elements in each dimention.
 * @dimbound  Placeholder for dimbound (return
 */
void cphvb_dimbound(cphvb_intp ndim,
                    const cphvb_index shape[],
                    cphvb_index* dimbound)
{
    dimbound[ndim -1] = shape[ndim -1];
    for (int i = ndim -2 ; i >= 0; --i)
    {
        dimbound[i] = dimbound[i+1] * shape[i];
    }
}


/* Calculate the offset into an array based on element index
 *
 * @ndim     Number of dimentions
 * @shape[]  Number of elements in each dimention.
 * @stride[] Stride in each dimention.
 * @element  Index of element in question
 * @return   Truth value indicating continuousity.
 */
cphvb_index cphvb_calc_offset(cphvb_intp ndim,
                              const cphvb_index shape[],
                              const cphvb_index stride[],
                              const cphvb_index element)
{
    cphvb_index offset = 0;
    cphvb_index dimIndex;
    cphvb_intp i;
    for (i = 0; i < ndim; ++i)
    {
        dimIndex = element % cphvb_nelements(ndim - i, &shape[i]);
        if (i != ndim - 1)
            dimIndex = dimIndex / cphvb_nelements(ndim - (i+1), &shape[i+1]);
        offset += dimIndex * stride[i];
    }
    return offset;
}

/* Find the base array for a given array/view
 *
 * @view   Array/view in question
 * @return The Base array
 */
cphvb_array* cphvb_base_array(cphvb_array* view)
{
    if(view->base == NULL)
    {
        return view;
    }
    else
    {
        assert(view->base->base == NULL);
        return view->base;
    }
}

/* Allocate data memory for the given array if not already allocated.
 * NB: It does NOT initiate the memory.
 * For convenience array is allowed to be NULL.
 *
 * @array  The array in question
 * @return Error code (CPHVB_SUCCESS, CPHVB_OUT_OF_MEMORY)
 */
cphvb_error cphvb_data_malloc(cphvb_array* array)
{
    cphvb_intp nelem, bytes;
    cphvb_array* base;

    if(array == NULL)
        return CPHVB_SUCCESS;

    base = cphvb_base_array(array);

    if(base->data != NULL)
        return CPHVB_SUCCESS;

    nelem = cphvb_nelements(base->ndim, base->shape);
    bytes = nelem * cphvb_type_size(base->type);
    if(bytes <= 0)
        return CPHVB_SUCCESS;

    //Allocate page-size aligned memory.
    //The MAP_PRIVATE and MAP_ANONYMOUS flags is not 100% portable. See:
    //<http://stackoverflow.com/questions/4779188/how-to-use-mmap-to-allocate-a-memory-in-heap>
    base->data = mmap(0, bytes, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if(base->data == MAP_FAILED)
    {
        int errsv = errno;//mmap() sets the errno.
        printf("cphvb_data_malloc() could not mmap a data region. "
               "Returned error code by mmap: %s.\n", strerror(errsv));
        return CPHVB_OUT_OF_MEMORY;
    }

    return CPHVB_SUCCESS;
}

/* Frees data memory for the given array.
 * For convenience array is allowed to be NULL.
 *
 * @array  The array in question
 * @return Error code (CPHVB_SUCCESS, CPHVB_OUT_OF_MEMORY)
 */
cphvb_error cphvb_data_free(cphvb_array* array)
{
    cphvb_intp nelem, bytes;
    cphvb_array* base;

    if(array == NULL)
        return CPHVB_SUCCESS;

    base = cphvb_base_array(array);

    if(base->data == NULL)
        return CPHVB_SUCCESS;

    nelem = cphvb_nelements(base->ndim, base->shape);
    bytes = nelem * cphvb_type_size(base->type);

    if(munmap(base->data, bytes) == -1)
    {
        int errsv = errno;//munmmap() sets the errno.
        printf("cphvb_data_free() could not munmap a data region. "
               "Returned error code by mmap: %s.\n", strerror(errsv));
        return CPHVB_ERROR;
    }
    base->data = NULL;
    return CPHVB_SUCCESS;
}


/* Retrive the operand type of a instruction.
 *
 * @instruction  The instruction in question
 * @operand_no Number of the operand in question
 * @return Error code (CPHVB_SUCCESS, CPHVB_OUT_OF_MEMORY)
 */
cphvb_type cphvb_type_operand(cphvb_instruction *instruction,
                              cphvb_intp operand_no)
{
    cphvb_array *a = instruction->operand[operand_no];
    return a->type;
}


/* Determines whether two arrays conflicts.
 *
 * @a The first array
 * @b The second array
 * @return The boolean answer
 */
int cphvb_array_conflict(cphvb_array *a, cphvb_array *b)
{
    cphvb_intp i;
    if(a == b)
        return 0;
    if(cphvb_base_array(a) != cphvb_base_array(b))
        return 0;

    if(a->ndim != b->ndim)
        return 1;

    if(a->start != b->start)
        return 1;

    for(i=0; i<a->ndim; ++i)
    {
        if(a->shape[i] != b->shape[i])
            return 1;
        if(a->stride[i] != b->stride[i])
            return 1;
    }
    return 0;
}
