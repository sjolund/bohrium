// jit_compile.cpp
// be the center of attension regarding the compilation of kernels. 
// Must include functionality to compile with different methods and 
// some kind of interface to access these.

// need to keep some state with information on allocated memory for the code being used
// a mapping between expr-key and function pointer, with memory reference, such that it
// it possible to clean up when the hole code is finished.


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <iostream>
#include <sstream>

#include "libtcc.h"
#include "cphvb.h"
#include "jit_logging.h"
#include "jit_compile.h"

using namespace std;

// TCC (Tiny C Compiler)
cphvb_intp compile_tcc(string func_name,string comput_func_text, jit_comp_kernel* kernel) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"compile_tcc()\n");
    
    const char* cphvb_headers = "/home/jolu/diku/speciale/cphVB/cphvb-priv/include";
    
    TCCState *s = tcc_new();;        
    void *mem = NULL;
    int size = 0;    
    
    if (!s) {
        fprintf(stderr, "Could not create tcc state\n");
        exit(1);
    }        
    //tcc_set_lib_path(s,"/usr/local/lib/");
    
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);                
    tcc_add_include_path(s,cphvb_headers);   
    
    if (tcc_compile_string(s, comput_func_text.c_str()) == -1) {
        printf("Failure to compile!\n");        
        return 1;
    }    
    
    // get needed size of the code
    size = tcc_relocate(s, NULL);
    //printf("tcc_relocate size: %d\n",size);
    if (size == -1)
        return 1;
        
    logcustom(cloglevel,1,"Size of code: %ld\n",size);
    // allocate memory and copy the code into it
    mem = malloc(size);
    tcc_relocate(s, mem);

    /* get entry symbol */
    kernel->memaddr = &mem; 
    kernel->function = (computefunc3) tcc_get_symbol(s, func_name.c_str());            
    kernel->key = 0;

    
    if (kernel->function == NULL) {        
        logInfo(" ---- ret_fun == %p",kernel->function);
        return 1;
    }
    logcustom(cloglevel,1,"Code compiled and function created: %p \n",kernel->function);
    tcc_delete(s); // delete the state
    
    // run the code
    //func(32);    
    //free(mem);    
    return 0;
}


cphvb_intp compile_gcc(string func_name,string comput_func_text, jit_comp_kernel* kernel) {
    string jitkernels_dir = string("/home/jolu/.cphvb/jitkernels");
    string cphvb_include_dir = string("/home/jolu/diku/speciale/cphVB/cphvb-priv/include");

    stringstream ss;
    string gccstring = string("gcc -x -fPIC -c test_kernel.c -I/home/jolu/diku/speciale/cphVB/cphvb-priv/include/ -L/home/jolu/diku/speciale/cphVB/cphvb-priv/core -lcphvb -lrt -ldl -shared -o testkernel.so -");

    // call gcc by system and compile to a .o file

    ss << "echo \"int getFive() { return 5; }\" | " << gccstring;
    
    int res = system(ss.str().c_str());

	//~ my_lib_handle = dlopen("libmylib.so",RTLD_NOW);
	//~ if(my_lib_handle==NULL) {
		//~ /* ERROR HANDLING */
	//~ }
	//~ some_func = (int (*)()) dlsym(my_lib_handle,"some_function");
	//~ if(some_func==NULL) {
		//~ /* ERROR HANDLING */
	//~ }
	//~ printf("Return code is %i\n",(*some_func)());
	//~ return 0;
    
    // use ld to link in the created file




    return 0;
}

jit_comp_kernel* compile(string kernel_func_name,string codetext, jit_compile_method method) {        
    jit_comp_kernel* kernel = ((jit_comp_kernel*) malloc(sizeof(jit_comp_kernel))) ;
    cphvb_intp res;
    switch(method) {            
        case COMPILE_METHOD_TCC:
            res = compile_tcc(kernel_func_name,codetext,kernel);
            //printf("compile result: %d\n", res);        
            return kernel;            
        case COMPILE_METHOD_GCC:
            //res = compile_gcc(kernel_func_name,codetext,kernel);
            return kernel;
        default:
            return NULL;
    }
}

jit_comp_kernel* jitc_compile_computefunction(string kernel_func_name, string compute_func_text) {
    //printf("........... %s\n",kernel_func_name.c_str());
    return compile(kernel_func_name,compute_func_text,COMPILE_METHOD_TCC);
}




