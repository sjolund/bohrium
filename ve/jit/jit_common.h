// jit_common.h

#ifndef __JIT_COMMON_H
#define __JIT_COMMON_H

#include <stdio.h>
#include <string.h>
#include <cphvb.h>
#include <cphvb_pprint.h>
#include "jit_analyser.h"
#include "jit_codegenerator.h"
#include <string>
#include <set>

void cphvb_pprint_instr_list_small( cphvb_instruction* instruction_list, cphvb_intp instruction_count, const char* txt );
void jit_pprint_expr(jit_expr* expr);
void jit_pprint_ssamap(jit_ssa_map* ssamap);
void jit_pprint_base_dependency_table(jit_base_dependency_table* base);
void jit_pprint_dependency_graph(jit_dependency_graph* graph);
void jit_pprint_base_dependency_list(std::vector<cphvb_intp>* ep_list);
void jit_pprint_list_of_expr(std::vector<jit_expr*>* expr_list);
std::string jit_pprint_nametable(jit_name_table* nt);
void jit_pprint_set(std::set<cphvb_intp>* set);
void jit_pprint_nametable_dependencies(jit_name_table* nt);
void jit_pprint_nametable_entry(jit_name_entry* entry);

void jit_pprint_cg_state(jitcg_state* cgs);
cphvb_intp instuctionlist_hash(cphvb_instruction* instruction_list, cphvb_intp instruction_count);
#endif
