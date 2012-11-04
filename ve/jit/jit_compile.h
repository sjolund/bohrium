// jit_compile.h

#ifndef __JIT_COMPILE_H
#define __JIT_COMPILE_H

#include "cphvb.h"
#include <vector>
#include "stdio.h"
#include <string>

using namespace std;

typedef void(*computefunc2)(cphvb_array*,cphvb_array**,cphvb_constant**,cphvb_index,cphvb_index*);
typedef void(*computefunc) (cphvb_array**,cphvb_index,cphvb_constant**,cphvb_index,cphvb_index);
typedef void(*computefunc3) (cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit);

struct jit_comp_kernel{
    computefunc3 function;   // function pointer
    void*       memaddr;    // pointer to memory, for later deallocation
    cphvb_intp  key;        // kernel key, a hash of the mathmatical operations.
};



typedef enum COMPILE_METHOD {
    COMPILE_METHOD_TCC = 1,
    COMPILE_METHOD_GCC = 2
} jit_compile_method;

jit_comp_kernel* jitc_compile_computefunction(string name,string func_text);


//typedef void(*computefunc2)(cphvb_array*,vector<cphvb_array*>*,vector<cphvb_constant*>*,cphvb_index,cphvb_index*);




#endif
