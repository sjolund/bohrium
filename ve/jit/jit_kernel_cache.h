// jit_kernel_cache.h
#ifndef __JIT_KERNEL_CACHE_H
#define __JIT_KERNEL_CACHE_H

#include "cphvb.h"
#include "jit_compile.h"
#include "jit_kernel.h"
#include <iostream>
#include <map>

using namespace std;

typedef map<cphvb_intp, jit_compound_kernel*> jit_kernel_cache;

typedef map<cphvb_intp, jit_execute_kernel*> jit_expression_kernel_cache;

jit_compound_kernel*    jit_kernel_cache_lookup(jit_kernel_cache* kc, cphvb_intp key);
bool                    jit_kernel_cache_insert(jit_kernel_cache* kc, cphvb_intp key, jit_compound_kernel* kernel);


jit_execute_kernel*     jit_expression_kernel_cache_lookup(jit_expression_kernel_cache* ekc, cphvb_intp key );
bool                    jit_expression_kernel_cache_insert(jit_expression_kernel_cache* ekc, cphvb_intp key, jit_execute_kernel* kernel);

string jit_expression_kernel_cache_string_stats();

#endif
