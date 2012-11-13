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
#include <fstream>
#include <sstream>
#include <algorithm>

#include "libtcc.h"
#include "cphvb.h"
#include "jit_logging.h"
#include "jit_compile.h"
#include "jit_common.h"


#ifndef JIT_KERNEL_DIR
#define JIT_KERNEL_DIR "/home/jolu/.cphvb/jitkernels"
#endif    

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

/*==  ==*/



/**
 * Escape quotes
 **/
string escape_text_quotes(string text) {
    stringstream ss;
    //text = string("two \" plings \"");
    string::iterator it;
    for(it=text.begin();it!=text.end();it++) {
        if (*it == '\"') {
            ss << "\\" << *it;
        } else if (*it == '\n') {
            ss << "\\n";
        } else if (*it == '\t') {
            ss << "\\t";
        } else {
            ss << *it;
        }
                   
    }
    //printf("%s",ss.str().c_str());
    return ss.str();
}

/**
 * nameing function for kernel files.
 **/
string jit_nameing_kernel_file(string func_name) {
    return func_name;
}

/**
 * Write the function text to an file based on the nameing function
 * and the global JIT_KERNEL_DIR varible.
 **/
cphvb_intp jit_write_function_to_file(string func_name, string func_text) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"write_function_to_file(%s,txt)\n",func_name.c_str());
    string filepath = string(JIT_KERNEL_DIR)+"/"+func_name+".c";
    logcustom(cloglevel,1,"writing file to %s" , filepath.c_str());
    ofstream file;    
    file.open(filepath.c_str(), ios::app);
    if (file != NULL) {
        file << func_text;
    } else {
        return 1;
    }
    file.flush();
    file.close();
    return 0;
}


/**
 * Remove the kernel file for the function name.
 **/
cphvb_intp remove_kernel_files(string func_name) {
    bool cloglevel[] = {0};
    string filepath = string(JIT_KERNEL_DIR)+"/"+func_name;
    logcustom(cloglevel,0,"CGCC Removing kernelfile %s\n", filepath.c_str());    
    remove( (filepath + ".so").c_str() );
    remove( (filepath + ".c").c_str() );
    return 0;
}

/**
 * Compiles the kernel string to an share library file, which are linked into the running code.
 * The funcion pointer in the kernel is updated with the new functions. The func_name must
 * be the exact name of a function in the compute_func_text
 *
 * No c-file version not working. needed to? Must create a valid string for the console for gcc to load.
 **/
cphvb_intp compile_gcc(string func_name,string compute_func_text, jit_comp_kernel* kernel, bool reset_kernel_dir) {
    bool cloglevel[] = {0,0,0};
    bool create_c_file = true;
    logcustom(cloglevel,0,"CGCC compile_gcc(%s, removekernel=%s\n",func_name.c_str(),jit_pprint_true_false(reset_kernel_dir).c_str());
    
    
    string jitkernels_dir       = string(JIT_KERNEL_DIR);
    string cphvb_include_dir    = string("/home/jolu/diku/speciale/cphVB/cphvb-priv/include");
    string cphvb_core_dir       = string("/home/jolu/diku/speciale/cphVB/cphvb-priv/core");    
    string funcname = jit_nameing_kernel_file(func_name);
    string funclibname = funcname+string(".so");
    string funclibpath = jitkernels_dir + "/" + funclibname;
    // chech if a .so file with the func_name already exists. 
    string escaped = escape_text_quotes(compute_func_text);
    logcustom(cloglevel,2,"CGGC initial dlopen: %s\n",funclibpath.c_str());            
    void* lib_handler = dlopen( funclibpath.c_str(),RTLD_LAZY);
    logcustom(cloglevel,2,"dlopen error: %s\n",dlerror());
    
    if (lib_handler == NULL) {        
        logcustom(cloglevel,1,"CGCC creating new %s\n",funclibname.c_str());
        stringstream ss;
                
        if (create_c_file) {
            if (jit_write_function_to_file(func_name,compute_func_text) == 0) {
                ss << "gcc -march=native -xc -fPIC -O2 " << jitkernels_dir << "/" << funcname  << ".c -I" << cphvb_include_dir << " -L" << cphvb_core_dir;
                ss << " -lcphvb -lrt -ldl -shared -o " << funclibpath;
            }            
            
        } else {
            ss << "gcc -march=native -xc -fPIC -O2 -I" << cphvb_include_dir << " -L" << cphvb_core_dir;
            ss << " -lrt -lcphvb -ldl -shared -o " << funclibpath << " -";    
        }
              
        string gccstring = ss.str();
        //string gccstring = string("gcc -x -fPIC -O2 -c test_kernel.c -I/home/jolu/diku/speciale/cphVB/cphvb-priv/include/ -L/home/jolu/diku/speciale/cphVB/cphvb-priv/core -lcphvb -lrt -ldl -shared -o testkernel.so -");

        logcustom(cloglevel,2,"CGCC Compile arg string: %s\n",gccstring.c_str());
        ss.clear();

    
        if (!create_c_file) {
            //compute_func_text = ("int doFive() {\n\tprintf(\"test\n\");\n}\n");
            ss << "echo \"" << escape_text_quotes(compute_func_text).c_str() << "\" | " << gccstring;
        }
        
        //printf("\n\n%s\n\n",ss.str().c_str());
        // call gcc by system and compile to a .o file            
        int res = system(ss.str().c_str());        
        logcustom(cloglevel,0,"CGCC Compiler results: %d\n",res);        
        ss.clear();

        logcustom(cloglevel,2,"CGGC created. dlopen: %s\n",funclibpath.c_str());
        fstream getfile (funclibpath.c_str());        
        getfile.close();        
        logcustom(cloglevel,1," %s \n",getfile.is_open()?"open":"closed");
        
        lib_handler = dlopen( funclibpath.c_str(),RTLD_LAZY);
        logcustom(cloglevel,1,"dlopen error: %s\n",dlerror());
    }
    
	
	if(lib_handler==NULL) {
        logcustom(cloglevel,1,"CGCC Failed to open newly created %s file\n", funclibname.c_str());
        return 1;		
	}
    
	computefunc3 func = (computefunc3) dlsym(lib_handler,funcname.c_str());
	if(func==NULL) {
        logcustom(cloglevel,1,"CGCC Failed to get functionpointer from kernelfile %s\n", funcname.c_str());
		return 2;
	}

    kernel->function = func;

    if (reset_kernel_dir) {    
        return remove_kernel_files(funcname);
    }    	
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
            res = compile_gcc(kernel_func_name,codetext,kernel,true);
            return kernel;
        case COMPILE_METHOD_LLVM:
            //res = compile_gcc(kernel_func_name,codetext,kernel,false);
            //return kernel;
            printf("LLVM Compiler option not implemented.\n");
            return NULL;          
        default:
            return NULL;
    }
}

jit_comp_kernel* jitc_compile_computefunction(string kernel_func_name, string compute_func_text,jit_compile_method method) {
    //printf("........... %s\n",kernel_func_name.c_str());
    //return compile(kernel_func_name,compute_func_text,COMPILE_METHOD_TCC);
    return compile(kernel_func_name,compute_func_text,method);
}




