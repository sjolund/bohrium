// jit_kernel_cache
// 

// this should be a map on Hash - kernel, where the Hash is a value 
// computed from the expression of the kernel (or the simple hash of
// the code-string).

// the Hash could be a side effect of the traversal of the 
// jit_expr-structure, where each type of element could be interpretted

#include "jit_kernel_cache.h"
#include "StringHasher.hpp"
#include "cphvb.h"
#include <iostream>
#include <map>
#include <set>

using namespace std;

size_t make_key(std::string str) {
    return string_hasher(str);
}


void jit_kernel_cache_insert(jit_kernel_cache* kc,cphvb_intp key, jit_kernel* kernel){
    jit_kernel_cache::iterator it;    
    it = kc->find(key);
    if (it == kc->end()) {
        vector<jit_kernel*>* v = new vector<jit_kernel*>(1);
        kc->insert( pair<cphvb_intp, vector<jit_kernel*>* >(key,v) );
        (*v)[0] = kernel;
    } else {
        (*it).second->push_back(kernel);
    }
}

/**
 * takes and key, and returns a list of values for this key. 
 * */
vector<jit_kernel*>* jit_kernel_cache_lookup(jit_kernel_cache* kc, cphvb_intp key) {
    jit_kernel_cache_retit ret;
    map<cphvb_intp, vector<jit_kernel*>*, bool>::iterator res;
    res = kc->find(key);
    if (res->second == false) {
        return NULL;
    }
    return res->second;    
}



// get_a_cache()

bool put_in_cache(jit_kernel_map map, size_t key, jit_comp_kernel* kernel) {
    
    if (map.find(key) != map.end()) {
        map.insert(pair<size_t,jit_comp_kernel*>(key,kernel));
        return true;
    }
    return false;
}

// get_from_cache()







