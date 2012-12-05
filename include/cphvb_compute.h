/*
This file is part of cphVB and copyright (c) 2012 the cphVB team:
http://cphvb.bitbucket.org

cphVB is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 
of the License, or (at your option) any later version.

cphVB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the 
GNU Lesser General Public License along with cphVB. 

If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __CPHVB_COMPUTE_H
#define __CPHVB_COMPUTE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cphvb_tstate_naive cphvb_tstate_naive;
struct cphvb_tstate_naive {
    cphvb_index coord[CPHVB_MAXDIM];
    cphvb_index cur_e;
};
void cphvb_tstate_reset_naive( cphvb_tstate_naive *state );

typedef struct cphvb_tstate cphvb_tstate;
struct cphvb_tstate {
    cphvb_index ndim;
    cphvb_index noperands;
    cphvb_index shape[CPHVB_MAXDIM];
    void* start[CPHVB_MAX_NO_OPERANDS];
    cphvb_index stride[CPHVB_MAX_NO_OPERANDS][CPHVB_MAXDIM];
};
void cphvb_tstate_reset( cphvb_tstate *state, cphvb_instruction* instr );

typedef struct cphvb_dense_iterator cphvb_dense_iterator;
struct cphvb_dense_iterator {

	// Counters for keeping track of positions in anything above 3D
	cphvb_index counters[CPHVB_MAXDIM];

	// The number of elements in each dimension
	cphvb_index shape[CPHVB_MAXDIM];

	// The stride (in bytes) for moving in each dimension
	cphvb_index stride[CPHVB_MAX_NO_OPERANDS][CPHVB_MAXDIM];
		
	// The pointer offsets
    void* start[CPHVB_MAX_NO_OPERANDS];

	// A 4-element lookup table for fast calculation of offsets
	// when moving in the inner 3 dimensions
	cphvb_index stride_lookup[4][CPHVB_MAX_NO_OPERANDS];

	// The number of dimensions - 3
	cphvb_index pstart;
	
	// The counter for inner elements, moving from 0 to shapelimit0
	cphvb_index inner_ops;

	// The number of elements in the inner 3 dimensions
	cphvb_index shapelimit0;
	
	// The number of elements in the inner 2 dimensions
	cphvb_index shapelimit1;
		
	// The number of elements in the innermost dimension
	cphvb_index shapelimit2;
};

typedef struct cphvb_constant_iterator cphvb_constant_iterator;
struct cphvb_constant_iterator {
    void* start[1];
};

cphvb_error cphvb_dense_iterator_reset(cphvb_dense_iterator* it, cphvb_instruction* instr);
cphvb_error cphvb_constant_iterator_reset(cphvb_constant_iterator* it, cphvb_instruction* instr);

typedef cphvb_error (*cphvb_computeloop)( cphvb_instruction*, cphvb_tstate* );
typedef cphvb_error (*cphvb_computeloop_naive)( cphvb_instruction*, cphvb_tstate_naive*, cphvb_index );
typedef cphvb_error (*cphvb_computeloop_iterator)( cphvb_index, void** );

cphvb_computeloop_naive cphvb_compute_get_naive( cphvb_instruction *instr );
cphvb_error cphvb_compute_apply_naive( cphvb_instruction *instr );
cphvb_error cphvb_compute_reduce_naive(cphvb_userfunc *arg, void* ve_arg);

cphvb_error cphvb_compute_iterator_apply( cphvb_instruction *instr );
cphvb_computeloop_iterator cphvb_compute_iterator_get (cphvb_instruction *instr);

cphvb_computeloop cphvb_compute_get( cphvb_instruction *instr );
cphvb_error cphvb_compute_apply( cphvb_instruction *instr );
cphvb_error cphvb_compute_reduce(cphvb_userfunc *arg, void* ve_arg);
cphvb_error cphvb_compute_aggregate(cphvb_userfunc *arg, void* ve_arg);

cphvb_error cphvb_compute_random(cphvb_userfunc *arg, void* ve_arg);
cphvb_error cphvb_compute_matmul(cphvb_userfunc *arg, void* ve_arg);
cphvb_error cphvb_compute_nselect(cphvb_userfunc *arg, void* ve_arg);

#ifdef __cplusplus
}
#endif

#endif
