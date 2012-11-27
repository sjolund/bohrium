// jit_kernel_cache
// 

// this should be a map on Hash - kernel, where the Hash is a value 
// computed from the expression of the kernel (or the simple hash of
// the code-string).

// the Hash could be a side effect of the traversal of the 
// jit_expr-structure, where each type of element could be interpretted

#include "jit_kernel_cache.h"
#include "jit_kernel.h"

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



cphvb_index jit_expr_kernel_cache_hits = 0;
cphvb_index jit_expr_kernel_cache_miss = 0;



bool jit_expression_kernel_cache_insert(jit_expression_kernel_cache* ekc, cphvb_intp key, jit_execute_kernel* kernel) {
    pair<jit_expression_kernel_cache::iterator,bool> res = ekc->insert( pair<cphvb_intp, jit_execute_kernel* >(key,kernel) );
    if(__JIT_KERNEL_CACHE_DEBUG == 1) {
        printf("EKCI KernelCache: inserting %ld  -  %s\n",kernel->kernel->id, (res.second)? "SUCCESS": "ERROR");
    }  
    return res.second;
    
    return 0;
}
jit_execute_kernel* jit_expression_kernel_cache_lookup(jit_expression_kernel_cache* ekc, cphvb_intp key ) {
    jit_expression_kernel_cache::iterator it;
    it = ekc->find(key);
    if(__JIT_KERNEL_CACHE_DEBUG == 1) {
        printf("EKCL KernelCache: lookup %ld  -  %s\n",key, (it != ekc->end())? "SUCCESS": "ERROR");
    }    
    if (it == ekc->end()) {
        jit_expr_kernel_cache_miss++;
        return NULL;
    }
    jit_expr_kernel_cache_hits++;
    return &(*it->second);
}

string jit_expression_kernel_cache_string_stats() {
    stringstream ss;
    ss << "Hits: "<< jit_expr_kernel_cache_hits <<" Miss:"<<jit_expr_kernel_cache_miss;
    return ss.str();
}






















