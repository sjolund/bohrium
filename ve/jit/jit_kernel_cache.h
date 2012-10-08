// jit_kernel_cache.h



#ifndef __JIT_KERNEL_CACHE_H
#define __JIT_KERNEL_CACHE_H

#include "cphvb.h"
#include "jit_compile.h"
#include <iostream>
#include <stdlib.h>
#include <map>
#include <vector>
#include "jit_kernel.h"

using namespace std;

typedef map<size_t,jit_comp_kernel*> jit_kernel_map;

typedef map<cphvb_intp, vector<jit_kernel*>* > jit_kernel_cache;
typedef map<cphvb_intp, vector<jit_kernel*>*, bool > jit_kernel_cache_retit;

#endif
