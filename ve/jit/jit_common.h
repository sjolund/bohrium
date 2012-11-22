// jit_common.h


#ifndef __JIT_COMMON_H
#define __JIT_COMMON_H

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <set>
#include <vector>
#include <map>
#include "cphvb.h"
#include "cphvb_pprint.h"
#include "jit_analyser.h"
#include "jit_codegenerator.h"
#include "jit_kernel.h" 
#include "MurmurHash3.h"

cphvb_intp instructionlist_hash(cphvb_instruction* instruction_list, cphvb_intp instruction_count);

void cphvb_pprint_instr_list_small( cphvb_instruction* instruction_list, cphvb_intp instruction_count, const char* txt );
void jit_pprint_expr(jit_expr* expr);
void jit_pprint_ssamap(jit_ssa_map* ssamap);
void jit_pprint_base_dependency_table(jit_base_dependency_table* base);

void jit_pprint_base_dependency_list(std::vector<cphvb_intp>* ep_list);
void jit_pprint_list_of_expr(std::vector<jit_expr*>* expr_list);
string jit_pprint_nametable(jit_name_table* nt);
void jit_pprint_set(std::set<cphvb_intp>* set);
void jit_pprint_nametable_dependencies(jit_name_table* nt);
void jit_pprint_nametable_entry(jit_name_entry* entry);

//~ void jit_pprint_cg_state(jitcg_state* cgs);
//~ void jit_pprint_il_map(jit_io_instruction_list_map* il_map);

void jit_pprint_execute_kernel(jit_execute_kernel* exekernel);
void jit_pprint_il_map2(jit_io_instruction_list_map_lists* il_map);

string jit_pprint_compile_method(jit_compile_method method);
string jit_pprint_functiontext_creator(jitcg_functiontext_creator functext); 

void jit_pprint_cphvb_array(cphvb_array* a0, cphvb_intp limit);
void jit_pprint_cphvb_array_stream(cphvb_array* a0, cphvb_intp limit, stringstream* ss);
string jit_pprint_true_false(bool stm);
cphvb_intp expr_hash(jit_expr* expr);
timespec diff(timespec start, timespec end);
int timeval_subtract (timeval* result, timeval* x, timeval* y);


#endif
