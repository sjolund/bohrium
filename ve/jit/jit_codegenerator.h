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
#include "jit_compile.h"

using namespace std;

// required state for code generation of a expression
typedef struct {
    cphvb_intp                  sync_expr_name;
    cphvb_intp                  hash;
    string                      kernel_name;    
    
    string                      source_function;            // function source
    string                      source_math;                // math operation source
    jit_comp_kernel*            kernel;                     // pointer to the kernel. set after compilation
    
    cphvb_array**               array_inputs;
    cphvb_intp                  array_inputs_len;
    cphvb_constant**            constant_inputs;
    cphvb_intp                  constant_inputs_len;      
} jitcg_state;


void jitcg_print_cphvb_array(cphvb_array* array, cphvb_intp limit, std::stringstream* ss);
void jitcg_expr_codetext(cphvb_array* return_array, jit_expr* expr, jit_ssa_map* ssamap, const char* kernel_name);
//void jitcg_create_kernel_code(jitcg_state* cgs, jit_analyse_state* s, jit_expr* expr);

//cphvb_error traverse_first_kernel(cphvb_array* oa, vector<cphvb_array*>* as, vector<cphvb_constant*>* cs, cphvb_index skip, cphvb_index limit )
void test_computation(cphvb_array* output_array, jit_expr* expr,jit_ssa_map* ssamap,jit_name_table* nametable);
const char* cphvb_type_typetext(cphvb_type type);
void jitcg_codetext_opcode_stream(cphvb_opcode opcode, stringstream* oss);
string etrav(jit_expr* e);
string build_computestring_etrav(jit_expr* e);
string build_expression_string(cphvb_opcode c, string s1, string s2);

string create_kernel_function_travers(string name,string computation_string,map<cphvb_array*,cphvb_intp>* outputarrays,map<cphvb_array*,cphvb_intp>* arrays,map<cphvb_constant*,cphvb_intp>* constants);
string create_kernel_function_travers_nocast(string name,string computation_string,map<cphvb_array*,cphvb_intp>* outputarrays,map<cphvb_array*,cphvb_intp>* arrays,map<cphvb_constant*,cphvb_intp>* constants);


string jitcg_nameing_array_offset(string prefix,cphvb_index i);
string jitcg_nameing_array_datapointer(string prefix,cphvb_index i);
string jitcg_nameing_array(cphvb_index i);
string jitcg_nameing_constant(cphvb_index i);

string jitcg_build_constant_compute_component(cphvb_index index);
string jitcg_build_array_compute_component(cphvb_index index);

            
void kernel_func_4265622974_0(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit);
void kernel_func_1899990464_0(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit);
// C
void kernel_func_4265622974_0(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit);

// D
void kernel_func_1625152366_0(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit);

// E
void kernel_func_3286243741_0(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit);

#endif

