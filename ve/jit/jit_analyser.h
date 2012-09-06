// cphvb_jit_analyser
//

#ifndef __JIT_ANALYSER_H
#define __JIT_ANALYSER_H

#include "cphvb.h"
#include <vector>
#include <map>
#include "jit_ast.h"

typedef enum { JIT_DISCARDRD = 1, JIT_EXECUTE = 2} jit_entry_status;

typedef struct {
    cphvb_array*                arrayp;
    cphvb_intp                  version;
    jit_expr*                   expr;
    jit_entry_status            status;
    std::vector<cphvb_intp>*    used_at;
} jit_name_entry;

typedef std::vector<jit_name_entry*> jit_name_table;
typedef std::map<cphvb_array*, std::vector<cphvb_intp>*> jit_ssa_map; // array* key with value being a vector of names, where the index is the version.
typedef std::vector<cphvb_intp> jit_execute_list; 

cphvb_intp jita_insert_name(jit_name_table* nametable,jit_ssa_map* ssamap, cphvb_array* array, jit_expr* expr);
cphvb_intp jita_handle_arithmetic_instruction(jit_name_table* nametable, jit_ssa_map* ssamap, cphvb_instruction* instr);

cphvb_intp jita_handle_controll_instruction(jit_name_table* nametable, jit_ssa_map* ssamap, jit_execute_list* exelist, cphvb_instruction* instr);
cphvb_intp jita_ssamap_version_lookup(jit_ssa_map* ssamap, cphvb_array* array, cphvb_intp version);

bool jita_is_controll(cphvb_instruction* instr);

jit_name_entry* jita_nametable_lookup(jit_name_table* nametable, cphvb_intp index);

// testing stuff
void jita_run_tests();
void _print_used_at(std::vector<cphvb_intp>* vec);

#endif

