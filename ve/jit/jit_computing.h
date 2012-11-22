// jit_computing.h

#ifndef __JIT_COMPUTING_H
#define __JIT_COMPUTING_H
#include "cphvb.h"


typedef cphvb_error(*instruction_compute_function) (cphvb_instruction *instr );

// a struct to hold the computation function. The list of registered userfunction
// and the compute function for regular instructions.
typedef struct {
    std::map<cphvb_index,cphvb_userfunc_impl>*  userfunctions;
    instruction_compute_function                instr_compute;
    
} jit_compute_functions;

#endif
