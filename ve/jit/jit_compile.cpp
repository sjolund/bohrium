// jit_compile.cpp
// be the center of attension regarding the compilation of kernels. 
// Must include functionality to compile with different methods and 
// some kind of interface to access these.

// need to keep some state with information on allocated memory for the code being used
// a mapping between expr-key and function pointer, with memory reference, such that it
// it possible to clean up when the hole code is finished.

#include "cphvb.h"

#include "jit_logging.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libtcc.h"
#include "jit_compile.h"

using namespace std;

// TCC (Tiny C Compiler)

cphvb_intp compile_tcc(string func_name,string comput_func_text, jit_comp_kernel* kernel) {
    //printf("s compile_tcc()\n");    
    
    const char* cphvb_headers = "/home/jolu/diku/speciale/cphVB/cphvb-priv/include";
    
    TCCState *s = tcc_new();;        
    void *mem = NULL;
    int size = 0;    
    
    if (!s) {
        fprintf(stderr, "Could not create tcc state\n");
        exit(1);
    }
        
    //tcc_set_lib_path(s,"/usr/local/lib/");
    //printf("state: %p\n",s);
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);        
    
    //printf("compute_func:\n%s\n",comput_func_text.c_str());
    
    tcc_add_include_path(s,cphvb_headers);   
    //printf("Adding headers\n");
    
    if (tcc_compile_string(s, comput_func_text.c_str()) == -1) {
        printf("Failure to compile!\n");
        
        return 1;
    }
    
    
    
    /* get needed size of the code */
    size = tcc_relocate(s, NULL);
    //printf("tcc_relocate size: %d\n",size);
    if (size == -1)
        return 1;
        
    logInfo("test\n");
    /* allocate memory and copy the code into it */
    mem = malloc(size);
    tcc_relocate(s, mem);

    /* get entry symbol */
    kernel->memaddr = &mem; 
    kernel->function = (computefunc) tcc_get_symbol(s, func_name.c_str());            
    kernel->key = 0;
    
    if (!kernel->function) {        
        logInfo(" ---- ret_fun == %p",kernel->function);
        return 1;
    }
        
    logInfo("test\n");
    /* delete the state */
    tcc_delete(s);
    
    /* run the code */
    //func(32);    
    //free(mem);
    logInfo("function pointer == %p\n",kernel->function);
    logInfo("e compile_tcc()\n");
    return 0;
}


jit_comp_kernel* compile(string kernel_func_name,string codetext, jit_compile_method method) {
    
    //jit_comp_kernel* kernel = NULL;
    jit_comp_kernel* kernel = ((jit_comp_kernel*) malloc(sizeof(jit_comp_kernel))) ;
    
    switch(method) {        
        default: 
            printf("compile result: %d\n", compile_tcc(kernel_func_name,codetext,kernel));        
            return kernel;            
    }
}

jit_comp_kernel* jitc_compile_computefunction(string kernel_func_name, string compute_func_text) {
    //printf("........... %s\n",kernel_func_name.c_str());
    return compile(kernel_func_name,compute_func_text,TCC);
}




