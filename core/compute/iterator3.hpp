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

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_1d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;
	cphvb_index limit0 = itstate->shape[0];

	for(i0 = 0; i0 < limit0; i0++) {
		op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
		nextA((IteratortypeA*)itstate->iterator[0], 0);
		nextB((IteratortypeB*)itstate->iterator[1], 0);
		nextC((IteratortypeC*)itstate->iterator[2], 0);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_2d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];

	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_3d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];

	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	nextC((IteratortypeC*)itstate->iterator[2], 2);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_4d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];

	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	nextC((IteratortypeC*)itstate->iterator[2], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	nextC((IteratortypeC*)itstate->iterator[2], 3);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_5d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];

	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	nextC((IteratortypeC*)itstate->iterator[2], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	nextC((IteratortypeC*)itstate->iterator[2], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	nextC((IteratortypeC*)itstate->iterator[2], 4);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_6d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];

	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	nextC((IteratortypeC*)itstate->iterator[2], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	nextC((IteratortypeC*)itstate->iterator[2], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	nextC((IteratortypeC*)itstate->iterator[2], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	nextC((IteratortypeC*)itstate->iterator[2], 5);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_7d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];

	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	nextC((IteratortypeC*)itstate->iterator[2], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	nextC((IteratortypeC*)itstate->iterator[2], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	nextC((IteratortypeC*)itstate->iterator[2], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	nextC((IteratortypeC*)itstate->iterator[2], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	nextC((IteratortypeC*)itstate->iterator[2], 6);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_8d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];

	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	nextC((IteratortypeC*)itstate->iterator[2], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	nextC((IteratortypeC*)itstate->iterator[2], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	nextC((IteratortypeC*)itstate->iterator[2], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	nextC((IteratortypeC*)itstate->iterator[2], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	nextC((IteratortypeC*)itstate->iterator[2], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	nextC((IteratortypeC*)itstate->iterator[2], 7);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_9d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];

	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	nextC((IteratortypeC*)itstate->iterator[2], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	nextC((IteratortypeC*)itstate->iterator[2], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	nextC((IteratortypeC*)itstate->iterator[2], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	nextC((IteratortypeC*)itstate->iterator[2], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	nextC((IteratortypeC*)itstate->iterator[2], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	nextC((IteratortypeC*)itstate->iterator[2], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	nextC((IteratortypeC*)itstate->iterator[2], 8);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_10d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;	cphvb_index i9;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];	cphvb_index limit9 = itstate->shape[9];

	for(i9 = 0; i9 < limit9; i9++) {
	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	nextC((IteratortypeC*)itstate->iterator[2], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	nextC((IteratortypeC*)itstate->iterator[2], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	nextC((IteratortypeC*)itstate->iterator[2], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	nextC((IteratortypeC*)itstate->iterator[2], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	nextC((IteratortypeC*)itstate->iterator[2], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	nextC((IteratortypeC*)itstate->iterator[2], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	nextC((IteratortypeC*)itstate->iterator[2], 8);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 9);
	nextB((IteratortypeB*)itstate->iterator[1], 9);
	nextC((IteratortypeC*)itstate->iterator[2], 9);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_11d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;	cphvb_index i9;	cphvb_index i10;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];	cphvb_index limit9 = itstate->shape[9];	cphvb_index limit10 = itstate->shape[10];

	for(i10 = 0; i10 < limit10; i10++) {
	for(i9 = 0; i9 < limit9; i9++) {
	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	nextC((IteratortypeC*)itstate->iterator[2], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	nextC((IteratortypeC*)itstate->iterator[2], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	nextC((IteratortypeC*)itstate->iterator[2], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	nextC((IteratortypeC*)itstate->iterator[2], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	nextC((IteratortypeC*)itstate->iterator[2], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	nextC((IteratortypeC*)itstate->iterator[2], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	nextC((IteratortypeC*)itstate->iterator[2], 8);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 9);
	nextB((IteratortypeB*)itstate->iterator[1], 9);
	nextC((IteratortypeC*)itstate->iterator[2], 9);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 10);
	nextB((IteratortypeB*)itstate->iterator[1], 10);
	nextC((IteratortypeC*)itstate->iterator[2], 10);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_12d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;	cphvb_index i9;	cphvb_index i10; cphvb_index i11;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];	cphvb_index limit9 = itstate->shape[9];	cphvb_index limit10 = itstate->shape[10]; cphvb_index limit11 = itstate->shape[11];

	for(i11 = 0; i11 < limit11; i11++) {
	for(i10 = 0; i10 < limit10; i10++) {
	for(i9 = 0; i9 < limit9; i9++) {
	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	nextC((IteratortypeC*)itstate->iterator[2], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	nextC((IteratortypeC*)itstate->iterator[2], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	nextC((IteratortypeC*)itstate->iterator[2], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	nextC((IteratortypeC*)itstate->iterator[2], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	nextC((IteratortypeC*)itstate->iterator[2], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	nextC((IteratortypeC*)itstate->iterator[2], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	nextC((IteratortypeC*)itstate->iterator[2], 8);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 9);
	nextB((IteratortypeB*)itstate->iterator[1], 9);
	nextC((IteratortypeC*)itstate->iterator[2], 9);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 10);
	nextB((IteratortypeB*)itstate->iterator[1], 10);
	nextC((IteratortypeC*)itstate->iterator[2], 10);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 11);
	nextB((IteratortypeB*)itstate->iterator[1], 11);
	nextC((IteratortypeC*)itstate->iterator[2], 11);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_13d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;	cphvb_index i9;	cphvb_index i10; cphvb_index i11; cphvb_index i12;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];	cphvb_index limit9 = itstate->shape[9];	cphvb_index limit10 = itstate->shape[10]; cphvb_index limit11 = itstate->shape[11]; cphvb_index limit12 = itstate->shape[12];

	for(i12 = 0; i12 < limit12; i12++) {
	for(i11 = 0; i11 < limit11; i11++) {
	for(i10 = 0; i10 < limit10; i10++) {
	for(i9 = 0; i9 < limit9; i9++) {
	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	nextC((IteratortypeC*)itstate->iterator[2], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	nextC((IteratortypeC*)itstate->iterator[2], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	nextC((IteratortypeC*)itstate->iterator[2], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	nextC((IteratortypeC*)itstate->iterator[2], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	nextC((IteratortypeC*)itstate->iterator[2], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	nextC((IteratortypeC*)itstate->iterator[2], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	nextC((IteratortypeC*)itstate->iterator[2], 8);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 9);
	nextB((IteratortypeB*)itstate->iterator[1], 9);
	nextC((IteratortypeC*)itstate->iterator[2], 9);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 10);
	nextB((IteratortypeB*)itstate->iterator[1], 10);
	nextC((IteratortypeC*)itstate->iterator[2], 10);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 11);
	nextB((IteratortypeB*)itstate->iterator[1], 11);
	nextC((IteratortypeC*)itstate->iterator[2], 11);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 12);
	nextB((IteratortypeB*)itstate->iterator[1], 12);
	nextC((IteratortypeC*)itstate->iterator[2], 12);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_14d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;	cphvb_index i9;	cphvb_index i10; cphvb_index i11; cphvb_index i12; cphvb_index i13;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];	cphvb_index limit9 = itstate->shape[9];	cphvb_index limit10 = itstate->shape[10]; cphvb_index limit11 = itstate->shape[11]; cphvb_index limit12 = itstate->shape[12]; cphvb_index limit13 = itstate->shape[13];

	for(i13 = 0; i13 < limit13; i13++) {
	for(i12 = 0; i12 < limit12; i12++) {
	for(i11 = 0; i11 < limit11; i11++) {
	for(i10 = 0; i10 < limit10; i10++) {
	for(i9 = 0; i9 < limit9; i9++) {
	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	nextC((IteratortypeC*)itstate->iterator[2], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	nextC((IteratortypeC*)itstate->iterator[2], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	nextC((IteratortypeC*)itstate->iterator[2], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	nextC((IteratortypeC*)itstate->iterator[2], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	nextC((IteratortypeC*)itstate->iterator[2], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	nextC((IteratortypeC*)itstate->iterator[2], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	nextC((IteratortypeC*)itstate->iterator[2], 8);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 9);
	nextB((IteratortypeB*)itstate->iterator[1], 9);
	nextC((IteratortypeC*)itstate->iterator[2], 9);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 10);
	nextB((IteratortypeB*)itstate->iterator[1], 10);
	nextC((IteratortypeC*)itstate->iterator[2], 10);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 11);
	nextB((IteratortypeB*)itstate->iterator[1], 11);
	nextC((IteratortypeC*)itstate->iterator[2], 11);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 12);
	nextB((IteratortypeB*)itstate->iterator[1], 12);
	nextC((IteratortypeC*)itstate->iterator[2], 12);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 13);
	nextB((IteratortypeB*)itstate->iterator[1], 13);
	nextC((IteratortypeC*)itstate->iterator[2], 13);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_15d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;	cphvb_index i9;	cphvb_index i10; cphvb_index i11; cphvb_index i12; cphvb_index i13;	cphvb_index i14;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];	cphvb_index limit9 = itstate->shape[9];	cphvb_index limit10 = itstate->shape[10]; cphvb_index limit11 = itstate->shape[11]; cphvb_index limit12 = itstate->shape[12]; cphvb_index limit13 = itstate->shape[13]; cphvb_index limit14 = itstate->shape[14];

	for(i14 = 0; i14 < limit14; i14++) {
	for(i13 = 0; i13 < limit13; i13++) {
	for(i12 = 0; i12 < limit12; i12++) {
	for(i11 = 0; i11 < limit11; i11++) {
	for(i10 = 0; i10 < limit10; i10++) {
	for(i9 = 0; i9 < limit9; i9++) {
	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	nextC((IteratortypeC*)itstate->iterator[2], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	nextC((IteratortypeC*)itstate->iterator[2], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	nextC((IteratortypeC*)itstate->iterator[2], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	nextC((IteratortypeC*)itstate->iterator[2], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	nextC((IteratortypeC*)itstate->iterator[2], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	nextC((IteratortypeC*)itstate->iterator[2], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	nextC((IteratortypeC*)itstate->iterator[2], 8);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 9);
	nextB((IteratortypeB*)itstate->iterator[1], 9);
	nextC((IteratortypeC*)itstate->iterator[2], 9);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 10);
	nextB((IteratortypeB*)itstate->iterator[1], 10);
	nextC((IteratortypeC*)itstate->iterator[2], 10);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 11);
	nextB((IteratortypeB*)itstate->iterator[1], 11);
	nextC((IteratortypeC*)itstate->iterator[2], 11);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 12);
	nextB((IteratortypeB*)itstate->iterator[1], 12);
	nextC((IteratortypeC*)itstate->iterator[2], 12);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 13);
	nextB((IteratortypeB*)itstate->iterator[1], 13);
	nextC((IteratortypeC*)itstate->iterator[2], 13);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 14);
	nextB((IteratortypeB*)itstate->iterator[1], 14);
	nextC((IteratortypeC*)itstate->iterator[2], 14);
	}
};

template <typename T0, typename T1, typename T2, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB, typename NextfuncC, typename IteratortypeC>
void traverse_it3_aaa_16d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	NextfuncC nextC;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;	cphvb_index i9;	cphvb_index i10; cphvb_index i11; cphvb_index i12; cphvb_index i13;	cphvb_index i14;	cphvb_index i15;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];	cphvb_index limit9 = itstate->shape[9];	cphvb_index limit10 = itstate->shape[10]; cphvb_index limit11 = itstate->shape[11]; cphvb_index limit12 = itstate->shape[12]; cphvb_index limit13 = itstate->shape[13]; cphvb_index limit14 = itstate->shape[14]; cphvb_index limit15 = itstate->shape[15];

	for(i15 = 0; i15 < limit15; i15++) {
	for(i14 = 0; i14 < limit14; i14++) {
	for(i13 = 0; i13 < limit13; i13++) {
	for(i12 = 0; i12 < limit12; i12++) {
	for(i11 = 0; i11 < limit11; i11++) {
	for(i10 = 0; i10 < limit10; i10++) {
	for(i9 = 0; i9 < limit9; i9++) {
	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start, (T2*)((IteratortypeC*)itstate->iterator[2])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
			nextC((IteratortypeC*)itstate->iterator[2], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
		nextC((IteratortypeC*)itstate->iterator[2], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	nextC((IteratortypeC*)itstate->iterator[2], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	nextC((IteratortypeC*)itstate->iterator[2], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	nextC((IteratortypeC*)itstate->iterator[2], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	nextC((IteratortypeC*)itstate->iterator[2], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	nextC((IteratortypeC*)itstate->iterator[2], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	nextC((IteratortypeC*)itstate->iterator[2], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	nextC((IteratortypeC*)itstate->iterator[2], 8);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 9);
	nextB((IteratortypeB*)itstate->iterator[1], 9);
	nextC((IteratortypeC*)itstate->iterator[2], 9);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 10);
	nextB((IteratortypeB*)itstate->iterator[1], 10);
	nextC((IteratortypeC*)itstate->iterator[2], 10);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 11);
	nextB((IteratortypeB*)itstate->iterator[1], 11);
	nextC((IteratortypeC*)itstate->iterator[2], 11);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 12);
	nextB((IteratortypeB*)itstate->iterator[1], 12);
	nextC((IteratortypeC*)itstate->iterator[2], 12);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 13);
	nextB((IteratortypeB*)itstate->iterator[1], 13);
	nextC((IteratortypeC*)itstate->iterator[2], 13);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 14);
	nextB((IteratortypeB*)itstate->iterator[1], 14);
	nextC((IteratortypeC*)itstate->iterator[2], 14);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 15);
	nextB((IteratortypeB*)itstate->iterator[1], 15);
	nextC((IteratortypeC*)itstate->iterator[2], 15);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_1d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;

	cphvb_index limit0 = itstate->shape[0];

	for(i0 = 0; i0 < limit0; i0++) {
		op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
		nextA((IteratortypeA*)itstate->iterator[0], 0);
		nextB((IteratortypeB*)itstate->iterator[1], 0);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_2d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];

	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_3d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	

	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_4d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];

	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_5d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];

	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_6d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];

	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_7d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];

	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_8d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];
	
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_9d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];

	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_10d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;	cphvb_index i9;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];	cphvb_index limit9 = itstate->shape[9];

	for(i9 = 0; i9 < limit9; i9++) {
	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 9);
	nextB((IteratortypeB*)itstate->iterator[1], 9);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_11d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;	cphvb_index i9;	cphvb_index i10;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];	cphvb_index limit9 = itstate->shape[9];	cphvb_index limit10 = itstate->shape[10];

	for(i10 = 0; i10 < limit10; i10++) {
	for(i9 = 0; i9 < limit9; i9++) {
	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 9);
	nextB((IteratortypeB*)itstate->iterator[1], 9);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 10);
	nextB((IteratortypeB*)itstate->iterator[1], 10);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_12d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;	cphvb_index i9;	cphvb_index i10; cphvb_index i11;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];	cphvb_index limit9 = itstate->shape[9];	cphvb_index limit10 = itstate->shape[10]; cphvb_index limit11 = itstate->shape[11]; 

	for(i11 = 0; i11 < limit11; i11++) {
	for(i10 = 0; i10 < limit10; i10++) {
	for(i9 = 0; i9 < limit9; i9++) {
	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 9);
	nextB((IteratortypeB*)itstate->iterator[1], 9);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 10);
	nextB((IteratortypeB*)itstate->iterator[1], 10);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 11);
	nextB((IteratortypeB*)itstate->iterator[1], 11);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_13d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;	cphvb_index i9;	cphvb_index i10; cphvb_index i11; cphvb_index i12;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];	cphvb_index limit9 = itstate->shape[9];	cphvb_index limit10 = itstate->shape[10]; cphvb_index limit11 = itstate->shape[11]; cphvb_index limit12 = itstate->shape[12]; 


	for(i12 = 0; i12 < limit12; i12++) {
	for(i11 = 0; i11 < limit11; i11++) {
	for(i10 = 0; i10 < limit10; i10++) {
	for(i9 = 0; i9 < limit9; i9++) {
	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 9);
	nextB((IteratortypeB*)itstate->iterator[1], 9);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 10);
	nextB((IteratortypeB*)itstate->iterator[1], 10);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 11);
	nextB((IteratortypeB*)itstate->iterator[1], 11);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 12);
	nextB((IteratortypeB*)itstate->iterator[1], 12);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_14d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;	cphvb_index i9;	cphvb_index i10; cphvb_index i11; cphvb_index i12; cphvb_index i13;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];	cphvb_index limit9 = itstate->shape[9];	cphvb_index limit10 = itstate->shape[10]; cphvb_index limit11 = itstate->shape[11]; cphvb_index limit12 = itstate->shape[12]; cphvb_index limit13 = itstate->shape[13];

	for(i13 = 0; i13 < limit13; i13++) {
	for(i12 = 0; i12 < limit12; i12++) {
	for(i11 = 0; i11 < limit11; i11++) {
	for(i10 = 0; i10 < limit10; i10++) {
	for(i9 = 0; i9 < limit9; i9++) {
	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 9);
	nextB((IteratortypeB*)itstate->iterator[1], 9);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 10);
	nextB((IteratortypeB*)itstate->iterator[1], 10);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 11);
	nextB((IteratortypeB*)itstate->iterator[1], 11);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 12);
	nextB((IteratortypeB*)itstate->iterator[1], 12);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 13);
	nextB((IteratortypeB*)itstate->iterator[1], 13);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_15d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;	cphvb_index i9;	cphvb_index i10; cphvb_index i11; cphvb_index i12; cphvb_index i13;	cphvb_index i14;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];	cphvb_index limit9 = itstate->shape[9];	cphvb_index limit10 = itstate->shape[10]; cphvb_index limit11 = itstate->shape[11]; cphvb_index limit12 = itstate->shape[12]; cphvb_index limit13 = itstate->shape[13]; cphvb_index limit14 = itstate->shape[14];

	for(i14 = 0; i14 < limit14; i14++) {
	for(i13 = 0; i13 < limit13; i13++) {
	for(i12 = 0; i12 < limit12; i12++) {
	for(i11 = 0; i11 < limit11; i11++) {
	for(i10 = 0; i10 < limit10; i10++) {
	for(i9 = 0; i9 < limit9; i9++) {
	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 9);
	nextB((IteratortypeB*)itstate->iterator[1], 9);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 10);
	nextB((IteratortypeB*)itstate->iterator[1], 10);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 11);
	nextB((IteratortypeB*)itstate->iterator[1], 11);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 12);
	nextB((IteratortypeB*)itstate->iterator[1], 12);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 13);
	nextB((IteratortypeB*)itstate->iterator[1], 13);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 14);
	nextB((IteratortypeB*)itstate->iterator[1], 14);
	}
};

template <typename T0, typename T1, typename Instr, typename NextfuncA, typename IteratortypeA, typename NextfuncB, typename IteratortypeB>
void traverse_it3_aa_16d( cphvb_itstate* itstate ) {
	Instr op;
	NextfuncA nextA;
	NextfuncB nextB;
	
	cphvb_index i0;	cphvb_index i1;	cphvb_index i2;	cphvb_index i3;	cphvb_index i4;	cphvb_index i5;	cphvb_index i6;	cphvb_index i7;	cphvb_index i8;	cphvb_index i9;	cphvb_index i10; cphvb_index i11; cphvb_index i12; cphvb_index i13;	cphvb_index i14;	cphvb_index i15;

	cphvb_index limit0 = itstate->shape[0];	cphvb_index limit1 = itstate->shape[1];	cphvb_index limit2 = itstate->shape[2];	cphvb_index limit3 = itstate->shape[3];	cphvb_index limit4 = itstate->shape[4];	cphvb_index limit5 = itstate->shape[5];	cphvb_index limit6 = itstate->shape[6];	cphvb_index limit7 = itstate->shape[7];	cphvb_index limit8 = itstate->shape[8];	cphvb_index limit9 = itstate->shape[9];	cphvb_index limit10 = itstate->shape[10]; cphvb_index limit11 = itstate->shape[11]; cphvb_index limit12 = itstate->shape[12]; cphvb_index limit13 = itstate->shape[13]; cphvb_index limit14 = itstate->shape[14]; cphvb_index limit15 = itstate->shape[15];

	for(i15 = 0; i15 < limit15; i15++) {
	for(i14 = 0; i14 < limit14; i14++) {
	for(i13 = 0; i13 < limit13; i13++) {
	for(i12 = 0; i12 < limit12; i12++) {
	for(i11 = 0; i11 < limit11; i11++) {
	for(i10 = 0; i10 < limit10; i10++) {
	for(i9 = 0; i9 < limit9; i9++) {
	for(i8 = 0; i8 < limit8; i8++) {
	for(i7 = 0; i7 < limit7; i7++) {
	for(i6 = 0; i6 < limit6; i6++) {
	for(i5 = 0; i5 < limit5; i5++) {
	for(i4 = 0; i4 < limit4; i4++) {
	for(i3 = 0; i3 < limit3; i3++) {
	for(i2 = 0; i2 < limit2; i2++) {
	for(i1 = 0; i1 < limit1; i1++) {
		for(i0 = 0; i0 < limit0; i0++) {
			op((T0*)((IteratortypeA*)itstate->iterator[0])->start, (T1*)((IteratortypeB*)itstate->iterator[1])->start);
			nextA((IteratortypeA*)itstate->iterator[0], 0);
			nextB((IteratortypeB*)itstate->iterator[1], 0);
		}
		nextA((IteratortypeA*)itstate->iterator[0], 1);
		nextB((IteratortypeB*)itstate->iterator[1], 1);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 2);
	nextB((IteratortypeB*)itstate->iterator[1], 2);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 3);
	nextB((IteratortypeB*)itstate->iterator[1], 3);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 4);
	nextB((IteratortypeB*)itstate->iterator[1], 4);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 5);
	nextB((IteratortypeB*)itstate->iterator[1], 5);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 6);
	nextB((IteratortypeB*)itstate->iterator[1], 6);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 7);
	nextB((IteratortypeB*)itstate->iterator[1], 7);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 8);
	nextB((IteratortypeB*)itstate->iterator[1], 8);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 9);
	nextB((IteratortypeB*)itstate->iterator[1], 9);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 10);
	nextB((IteratortypeB*)itstate->iterator[1], 10);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 11);
	nextB((IteratortypeB*)itstate->iterator[1], 11);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 12);
	nextB((IteratortypeB*)itstate->iterator[1], 12);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 13);
	nextB((IteratortypeB*)itstate->iterator[1], 13);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 14);
	nextB((IteratortypeB*)itstate->iterator[1], 14);
	}
	nextA((IteratortypeA*)itstate->iterator[0], 15);
	nextB((IteratortypeB*)itstate->iterator[1], 15);
	}
};
