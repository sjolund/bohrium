// jit_compile.h

#ifndef __JIT_COMPILE_H
#define __JIT_COMPILE_H

#include "cphvb.h"
#include <vector>
#include "stdio.h"
#include <string>


using namespace std;

typedef enum COMPILE_METHOD {
    COMPILE_METHOD_TCC = 1,
    COMPILE_METHOD_GCC = 2,
    COMPILE_METHOD_LLVM = 3
} jit_compile_method;

typedef void(*computefunc3) (cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit);

typedef struct {
    computefunc3    function;   // function pointer
    void*           memaddr;    // pointer to memory, for later deallocation
    cphvb_intp      key;        // kernel key, a hash of the mathmatical operations.
} jit_comp_kernel;

jit_comp_kernel* jitc_compile_computefunction(string name,string func_text,jit_compile_method method);


// OLD
//typedef void(*computefunc2)(cphvb_array*,vector<cphvb_array*>*,vector<cphvb_constant*>*,cphvb_index,cphvb_index*);
typedef void(*computefunc2)(cphvb_array*,cphvb_array**,cphvb_constant**,cphvb_index,cphvb_index*);
typedef void(*computefunc) (cphvb_array**,cphvb_index,cphvb_constant**,cphvb_index,cphvb_index);



#endif
