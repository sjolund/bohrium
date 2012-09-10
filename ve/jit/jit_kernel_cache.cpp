// jit_kernel_cache
// 

// this should be a map on Hash - kernel, where the Hash is a value 
// computed from the expression of the kernel (or the simple hash of
// the code-string).

// the Hash could be a side effect of the traversal of the 
// jit_expr-structure, where each type of element could be interpretted

#include "jit_kernel_cache.h"
#include "StringHasher.hpp"
#include <iostream>
#include <map>

using namespace std;

size_t make_key(std::string str) {
    return string_hasher(str);
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







