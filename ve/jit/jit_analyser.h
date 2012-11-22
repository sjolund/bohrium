// cphvb_jit_analyser
//

#ifndef __JIT_ANALYSER_H
#define __JIT_ANALYSER_H

#include "cphvb.h"
#include <vector>
#include <map>
#include "jit_ast.h"
#include <set>

typedef enum { JIT_NO_VAL = 0, JIT_DEPTRAV_VISITED = 1, JIT_DEPTRAV_CUTED = 2} jit_entry_status;

typedef struct {
    cphvb_array*                arrayp;         // The assignment to or target of the expression
    cphvb_intp                  version;        // enables reverse lookup in ssa table with arrayp
    jit_expr*                   expr;           // the expression with pointers to other expressions.
    
    bool                        is_executed;    // indicate if the arrayp contains the result of expr.
    
    cphvb_instruction*          instr;          // the instruction, which is the basis for this entry
    cphvb_intp                  instr_num;      // the number on the instruction_list.
    cphvb_intp                  operand_num;    // the operand number, 1 if right, 2 if left. only available if expr = array_val.

    bool                        dep_trav_visited;    
    
    std::set<cphvb_intp>*       tdon;
    std::set<cphvb_intp>*       tdto;

    cphvb_intp                  freed_at;
    cphvb_intp                  discarded_at;

    // userfunc hack handling!
    // userfunctions are registered in multiple NT entries. One fo each output of the userdefined function.
    // The span and span_index, is the number of NT entries the userfunction spans over and the index in this
    // span the current entry is.
    bool                        is_userfunction;        
    cphvb_index                 span;
    cphvb_index                 span_index;

        
    // needed ?
    std::vector<cphvb_intp>*    used_at;
    jit_entry_status            status;         // set JIT_DEPTRAV_VISIT when expr.execution list is created.       
    
} jit_name_entry;

typedef std::vector<cphvb_intp>                             jit_execute_list; 

typedef std::map<cphvb_array*, std::vector<cphvb_intp>*>    jit_base_dependency_table;
typedef std::vector<jit_name_entry*>                        jit_name_table;
typedef std::map<cphvb_array*, std::vector<cphvb_intp>*>    jit_ssa_map; 

typedef struct {
    jit_ssa_map*                ssamap ;
    jit_name_table*             nametable;
    jit_base_dependency_table*  base_usage_table;
} jit_analyse_state;



// ==============================
// Function definitions
// ==============================

// misc
bool                jita_is_controll(cphvb_instruction* instr);
cphvb_intp          jita_get_prior_element(std::vector<cphvb_intp>* base_dep, cphvb_intp elem);
jit_analyse_state*  jita_make_jit_analyser_state(jit_name_table* nametable, jit_ssa_map* ssamap, jit_base_dependency_table* base_usage_table);

// base dependency
void                        jita_perform_dependecy_analysis(jit_analyse_state* s, cphvb_index offset);
std::vector<cphvb_intp>*    jita_base_usage_table_get_usage(jit_base_dependency_table* dep_table, cphvb_array* base_array);
cphvb_intp                  jita_base_usage_table_add_usage(jit_base_dependency_table* dep_table, cphvb_array* base_array, cphvb_intp name);

// jit_name_table
jit_name_entry*     jita_lookup_name(jit_name_table* nametable, jit_ssa_map* ssamap, cphvb_array* array, cphvb_intp version);
jit_name_entry*     jita_nametable_lookup(jit_name_table* nametable, cphvb_intp index);
jit_name_entry*     jita_nametable_lookup_a(jit_analyse_state* s, cphvb_array* array);
cphvb_intp          jita_insert_name(jit_name_table* nametable,jit_ssa_map* ssamap, cphvb_array* array, jit_expr* expr);

// jit_ssa_map
cphvb_intp          jita_ssamap_version_lookup(jit_ssa_map* ssamap, cphvb_array* array, cphvb_intp version);

// Instruction handlers. Functions to handle different instruction types.
cphvb_intp  jita_handle_free_instruction(jit_analyse_state* s,cphvb_instruction* instr, cphvb_intp instr_num);
cphvb_intp  jita_handle_discard_instruction(jit_analyse_state* s,cphvb_instruction* instr, cphvb_intp instr_num);
cphvb_intp  jita_handle_userfunc_instruction(jit_analyse_state* s,cphvb_instruction* instr, cphvb_intp instr_num);
cphvb_intp  jita_handle_arithmetic_instruction3(jit_analyse_state* s,cphvb_instruction* instr, cphvb_intp instr_num);


#endif


