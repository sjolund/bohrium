// jit_codegenerator.h

#ifndef __JIT_CODEGENERATOR_H
#define __JIT_CODEGENERATOR_H
#include <iostream>
#include <sstream>
#include "cphvb.h"
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <string>
#include "jit_ast.h"
#include "jit_analyser.h"


void jitcg_print_cphvb_array(cphvb_array* array, cphvb_intp limit, std::stringstream* ss);
void jitcg_expr_codetext(cphvb_array* return_array, jit_expr* expr, jit_ssa_map* ssamap, const char* kernel_name);


//cphvb_error traverse_first_kernel(cphvb_array* oa, vector<cphvb_array*>* as, vector<cphvb_constant*>* cs, cphvb_index skip, cphvb_index limit )
void test_computation(cphvb_array* output_array, jit_expr* expr,jit_ssa_map* ssamap,jit_name_table* nametable);
#endif
