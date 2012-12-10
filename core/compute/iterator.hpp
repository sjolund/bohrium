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

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it_aa_1d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i;
	cphvb_index limit = itstate->shape[0];
	
	for(i = 0; i < limit; i++) {
		op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
		nextA((IteratortypeA*)itstate->iterator[0], 0);
		nextB((IteratortypeB*)itstate->iterator[1], 0);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it_aaa_1d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i;
	cphvb_index limit = itstate->shape[0];

	//printf("In aaa 1d, shape: %lld\n", limit);

	//printf("In aaa 1d, it-pointers: %lld, %lld, %lld\n", (cphvb_int64)itstate->iterator[0], (cphvb_int64)itstate->iterator[1], (cphvb_int64)itstate->iterator[2]);

	//printf("In aaa 1d, data-pointers: %lld, %lld, %lld\n", (cphvb_int64)((IteratortypeA*)itstate->iterator[0])->start, (cphvb_int64)((IteratortypeB*)itstate->iterator[1])->start, (cphvb_int64)((IteratortypeC*)itstate->iterator[2])->start);

	for(i = 0; i < limit; i++) {
		op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
		nextA((IteratortypeA*)itstate->iterator[0], 0);
		nextB((IteratortypeB*)itstate->iterator[1], 0);
		nextC((IteratortypeC*)itstate->iterator[2], 0);
	}

	//printf("Leaving aaa 1d, shape: %lld\n", limit);
};

#define TRAVERSE_IT_AAA_ND(DIMNO, THISF, PREVF) \
template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC> \
void THISF( cphvb_itstate* itstate ) \
{ \
	NextfuncA nextA; \
	NextfuncB nextB; \
	NextfuncC nextC; \
\
	cphvb_index i; \
	cphvb_index limit = itstate->shape[ DIMNO ]; \
\
	for(i = 0; i < limit; i++) { \
		PREVF<T0, T1, T2, Instr, NextfuncA, IteratortypeA, NextfuncB, IteratortypeB, NextfuncC, IteratortypeC>(itstate); \
		nextA((IteratortypeA*)itstate->iterator[0], DIMNO); \
		nextB((IteratortypeB*)itstate->iterator[1], DIMNO); \
		nextC((IteratortypeC*)itstate->iterator[2], DIMNO); \
	} \
};

#define TRAVERSE_IT_AA_ND(DIMNO, THISF, PREVF) \
template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB> \
void THISF( cphvb_itstate* itstate ) \
{ \
	NextfuncA nextA; \
	NextfuncB nextB; \
\
	cphvb_index i; \
	cphvb_index limit = itstate->shape[ DIMNO ]; \
\
	for(i = 0; i < limit; i++) { \
		PREVF<T0, T1, Instr, NextfuncA, IteratortypeA, NextfuncB, IteratortypeB>(itstate); \
		nextA((IteratortypeA*)itstate->iterator[0], DIMNO); \
		nextB((IteratortypeB*)itstate->iterator[1], DIMNO); \
	} \
};
TRAVERSE_IT_AAA_ND(1, traverse_it_aaa_2d, traverse_it_aaa_1d)
TRAVERSE_IT_AAA_ND(2, traverse_it_aaa_3d, traverse_it_aaa_2d)
TRAVERSE_IT_AAA_ND(3, traverse_it_aaa_4d, traverse_it_aaa_3d)
TRAVERSE_IT_AAA_ND(4, traverse_it_aaa_5d, traverse_it_aaa_4d)
TRAVERSE_IT_AAA_ND(5, traverse_it_aaa_6d, traverse_it_aaa_5d)
TRAVERSE_IT_AAA_ND(6, traverse_it_aaa_7d, traverse_it_aaa_6d)
TRAVERSE_IT_AAA_ND(7, traverse_it_aaa_8d, traverse_it_aaa_7d)
TRAVERSE_IT_AAA_ND(8, traverse_it_aaa_9d, traverse_it_aaa_8d)
TRAVERSE_IT_AAA_ND(9, traverse_it_aaa_10d, traverse_it_aaa_9d)
TRAVERSE_IT_AAA_ND(10, traverse_it_aaa_11d, traverse_it_aaa_10d)
TRAVERSE_IT_AAA_ND(11, traverse_it_aaa_12d, traverse_it_aaa_11d)
TRAVERSE_IT_AAA_ND(12, traverse_it_aaa_13d, traverse_it_aaa_12d)
TRAVERSE_IT_AAA_ND(13, traverse_it_aaa_14d, traverse_it_aaa_13d)
TRAVERSE_IT_AAA_ND(14, traverse_it_aaa_15d, traverse_it_aaa_14d)
TRAVERSE_IT_AAA_ND(15, traverse_it_aaa_16d, traverse_it_aaa_15d)

TRAVERSE_IT_AA_ND(1, traverse_it_aa_2d, traverse_it_aa_1d)
TRAVERSE_IT_AA_ND(2, traverse_it_aa_3d, traverse_it_aa_2d)
TRAVERSE_IT_AA_ND(3, traverse_it_aa_4d, traverse_it_aa_3d)
TRAVERSE_IT_AA_ND(4, traverse_it_aa_5d, traverse_it_aa_4d)
TRAVERSE_IT_AA_ND(5, traverse_it_aa_6d, traverse_it_aa_5d)
TRAVERSE_IT_AA_ND(6, traverse_it_aa_7d, traverse_it_aa_6d)
TRAVERSE_IT_AA_ND(7, traverse_it_aa_8d, traverse_it_aa_7d)
TRAVERSE_IT_AA_ND(8, traverse_it_aa_9d, traverse_it_aa_8d)
TRAVERSE_IT_AA_ND(9, traverse_it_aa_10d, traverse_it_aa_9d)
TRAVERSE_IT_AA_ND(10, traverse_it_aa_11d, traverse_it_aa_10d)
TRAVERSE_IT_AA_ND(11, traverse_it_aa_12d, traverse_it_aa_11d)
TRAVERSE_IT_AA_ND(12, traverse_it_aa_13d, traverse_it_aa_12d)
TRAVERSE_IT_AA_ND(13, traverse_it_aa_14d, traverse_it_aa_13d)
TRAVERSE_IT_AA_ND(14, traverse_it_aa_15d, traverse_it_aa_14d)
TRAVERSE_IT_AA_ND(15, traverse_it_aa_16d, traverse_it_aa_15d)
