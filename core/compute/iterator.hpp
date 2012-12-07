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
 
#include <cphvb.h>
#include <assert.h>
#include <cphvb_compute.h>

#define BYTE char

#define ADD_VOID_POINTER(pointer, value) ((pointer) = (void*)(((BYTE*)pointer) + (value)))


struct cphvb_dense_iterator_next_general_1op {
    inline void operator()(cphvb_dense_iterator* it) {
    	it->inner_ops++;

    	cphvb_index lookup = ((!(it->inner_ops % it->shapelimit1)) << 1) | (!(it->inner_ops % it->shapelimit2));
    	cphvb_index* tb = it->stride_lookup[lookup];
    	ADD_VOID_POINTER(it->start[0], tb[0]);
    	
    	if (it->inner_ops == it->shapelimit0) {
			cphvb_index p = it->pstart;
			it->inner_ops = 0;

			ADD_VOID_POINTER(it->start[0], it->stride[0][p]);

			while(--it->counters[p] == 0 && p > 0)
			{
				it->counters[p] = it->shape[p];
				p--;
				
		    	ADD_VOID_POINTER(it->start[0], it->stride[0][p]);
			}
    	}
    }
};

struct cphvb_dense_iterator_next_general_2op {
    inline void operator()(cphvb_dense_iterator* it) {
    	cphvb_index iops = ++it->inner_ops;

    	cphvb_index lookup = ((!(iops % it->shapelimit1)) << 1) | (!(iops % it->shapelimit2));
    	cphvb_index* tb = it->stride_lookup[lookup];

    	ADD_VOID_POINTER(it->start[0], tb[0]);
    	ADD_VOID_POINTER(it->start[1], tb[1]);
    	
    	if (it->inner_ops == it->shapelimit0) {
			cphvb_index p = it->pstart;
			it->inner_ops = 0;
	
			ADD_VOID_POINTER(it->start[0], it->stride[0][p]);
			ADD_VOID_POINTER(it->start[1], it->stride[1][p]);
	
			while(--it->counters[p] == 0 && p > 0)
			{
				it->counters[p] = it->shape[p];
				p--;
				
		    	ADD_VOID_POINTER(it->start[0], it->stride[0][p]);
		    	ADD_VOID_POINTER(it->start[1], it->stride[1][p]);
			}
    	}
    }
};

struct cphvb_dense_iterator_next_general_3op {
    inline void operator()(cphvb_dense_iterator* it) {
    	it->inner_ops++;

    	cphvb_index lookup = ((!(it->inner_ops % it->shapelimit1)) << 1) | (!(it->inner_ops % it->shapelimit2));
    	cphvb_index* tb = it->stride_lookup[lookup];
    	ADD_VOID_POINTER(it->start[0], tb[0]);
    	ADD_VOID_POINTER(it->start[1], tb[1]);
    	ADD_VOID_POINTER(it->start[2], tb[2]);
    	
    	if (it->inner_ops == it->shapelimit0) {
			cphvb_index p = it->pstart;
			it->inner_ops = 0;

			ADD_VOID_POINTER(it->start[0], it->stride[0][p]);
			ADD_VOID_POINTER(it->start[1], it->stride[1][p]);
			ADD_VOID_POINTER(it->start[2], it->stride[2][p]);

			while(--it->counters[p] == 0 && p > 0)
			{
				it->counters[p] = it->shape[p];
				p--;
				
		    	ADD_VOID_POINTER(it->start[0], it->stride[0][p]);
		    	ADD_VOID_POINTER(it->start[1], it->stride[1][p]);
		    	ADD_VOID_POINTER(it->start[2], it->stride[2][p]);
			}
    	}
    }
};

struct cphvb_constant_iterator_next {
    inline void operator()(cphvb_constant_iterator* it) {
    }
};

template <typename T0, typename T1, typename Instr, typename Nextfunc, typename Iteratortype>
cphvb_error traverse_it_aa( cphvb_index operations, void** arg ) {
	Instr op;
	Nextfunc next;
	
	Iteratortype* it = (Iteratortype*)arg[0];
	
	cphvb_index i;

	for(i = 0; i < operations; i++) {
		op((T0*)it->start[0], (T1*)it->start[1]);
		next(it);
	}
	
	return CPHVB_SUCCESS;
}

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
cphvb_error traverse_it_ab( cphvb_index operations, void** arg ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	IteratortypeA* itA = (IteratortypeA*)arg[0];
	IteratortypeB* itB = (IteratortypeB*)arg[1];

	cphvb_index i;

	for(i = 0; i < operations; i++) {
		op((T0*)itA->start[0], (T1*)itB->start[0]);
		nextA(itA);
		nextB(itB);
	}

	return CPHVB_SUCCESS;
}

template <typename T0, typename T1, typename T2, typename Instr, typename Nextfunc, typename Iteratortype>
cphvb_error traverse_it_aaa( cphvb_index operations, void** arg ) {
	Instr op;
	Nextfunc next;
	
	Iteratortype* it = (Iteratortype*)arg[0];

	cphvb_index i;

	for(i = 0; i < operations; i++) {
		op((T0*)it->start[0], (T1*)it->start[1], (T2*)it->start[2]);
		next(it);
	}

	return CPHVB_SUCCESS;
}

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
cphvb_error traverse_it_baa( cphvb_index operations, void** arg ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	IteratortypeA* itA = (IteratortypeA*)arg[0];
	IteratortypeB* itB = (IteratortypeB*)arg[1];

	cphvb_index i;

	for(i = 0; i < operations; i++) {
		op((T0*)itB->start[0], (T1*)itA->start[0], (T2*)itA->start[1]);
		nextA(itA);
		nextB(itB);
	}

	return CPHVB_SUCCESS;
}

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
cphvb_error traverse_it_aab( cphvb_index operations, void** arg ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	IteratortypeA* itA = (IteratortypeA*)arg[0];
	IteratortypeB* itB = (IteratortypeB*)arg[1];

	cphvb_index i;

	for(i = 0; i < operations; i++) {
		op((T0*)itA->start[0], (T1*)itA->start[1], (T2*)itB->start[0]);
		nextA(itA);
		nextB(itB);
	}

	return CPHVB_SUCCESS;
}

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
cphvb_error traverse_it_aba( cphvb_index operations, void** arg ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	IteratortypeA* itA = (IteratortypeA*)arg[0];
	IteratortypeB* itB = (IteratortypeB*)arg[1];

	cphvb_index i;

	for(i = 0; i < operations; i++) {
		op((T0*)itA->start[0], (T1*)itB->start[0], (T2*)itA->start[1]);
		nextA(itA);
		nextB(itB);
	}

	return CPHVB_SUCCESS;
}

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
cphvb_error traverse_it_abc( cphvb_index operations, void** arg ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	IteratortypeA* itA = (IteratortypeA*)arg[0];
	IteratortypeB* itB = (IteratortypeB*)arg[1];
	IteratortypeB* itC = (IteratortypeC*)arg[2];

	cphvb_index i;

	for(i = 0; i < operations; i++) {
		op((T0*)itA->start[0], (T1*)itB->start[0], (T2*)itC->start[0]);
		nextA(itA);
		nextB(itB);
		nextC(itC);
	}

	return CPHVB_SUCCESS;
}
