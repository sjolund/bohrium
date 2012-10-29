// jit_kernel_cache
// 

// this should be a map on Hash - kernel, where the Hash is a value 
// computed from the expression of the kernel (or the simple hash of
// the code-string).

// the Hash could be a side effect of the traversal of the 
// jit_expr-structure, where each type of element could be interpretted

#include "jit_kernel_cache.h"
#include "jit_analyser.h"
#include "cphvb.h"
#include <iostream>
#include <map>
#include <set>

#define __JIT_KERNEL_CACHE_DEBUG 0

using namespace std;

/**
 * 
 **/
bool jit_kernel_cache_insert(jit_kernel_cache* kc,cphvb_intp key, jit_compound_kernel* kernel) {
    pair<jit_kernel_cache::iterator,bool> res = kc->insert( pair<cphvb_intp, jit_compound_kernel* >(key,kernel) );
    if(__JIT_KERNEL_CACHE_DEBUG == 1) {
        printf("KCI KernelCache: inserting %ld  -  %s\n",kernel->id, (res.second)? "SUCCESS": "ERROR");
    }  
    return res.second;
}

/**
 * 
 **/
jit_compound_kernel* jit_kernel_cache_lookup(jit_kernel_cache* kc, cphvb_intp key) {    
    jit_kernel_cache::iterator it;
    it = kc->find(key);
    if(__JIT_KERNEL_CACHE_DEBUG == 1) {
        printf("KCL KernelCache: lookup %ld  -  %s\n",key, (it != kc->end())? "SUCCESS": "ERROR");
    }    
    if (it == kc->end()) {
        return NULL;
    }    
    return &(*it->second);
}








