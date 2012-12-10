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

typedef struct {
	void* start;
} cphvb_constant_iterator;

typedef struct {
	void* start;
	cphvb_index stride[CPHVB_MAXDIM];
} cphvb_dense_iterator;

struct cphvb_dense_iterator_next {
	inline void operator()(cphvb_dense_iterator* it, cphvb_index dimension) {
		it->start = (void*)(((char*)it->start) + it->stride[dimension]);
	}
};

struct cphvb_noop_iterator_next {
    inline void operator()(void* it, cphvb_index dimension) {
    }
};