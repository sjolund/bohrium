// jit_mcache.h

// this is more of a wrapper on the cphvb_mcache then and implementation.
// access to allocation and free based on instructions is not valid in the
// jit approach where arrays are used as the base. 

#ifndef __JIT_MCACHE_H
#define __JIT_MCACHE_H

#include "cphvb.h"
#include "cphvb_mcache.h"

cphvb_error jit_mcache_free( cphvb_array* array );  
cphvb_error jit_mcache_malloc( cphvb_array* array );


#endif

