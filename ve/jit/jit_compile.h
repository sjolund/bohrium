// jit_compile.h

#ifndef __JIT_COMPILE_H
#define __JIT_COMPILE_H

#include "cphvb.h"
#include <vector>
#include "stdio.h"
#include <string>

using namespace std;

typedef void(*computefunc2)(cphvb_array*,cphvb_array**,cphvb_constant**,cphvb_index,cphvb_index*);

typedef void(*computefunc) (cphvb_array*,cphvb_array**,cphvb_index,cphvb_constant**,cphvb_index,cphvb_index);

struct jit_comp_kernel{
    computefunc function;   // function pointer
    void*       memaddr;    // pointer to memory, for later deallocation
    cphvb_intp  key;        // kernel key, a hash of the mathmatical operations.
};


typedef enum COMPILE_METHOD {
    TCC = 1
} jit_compile_method;

jit_comp_kernel* jitc_compile_computefunction(string name,string func_text);


//typedef void(*computefunc2)(cphvb_array*,vector<cphvb_array*>*,vector<cphvb_constant*>*,cphvb_index,cphvb_index*);




#endif
