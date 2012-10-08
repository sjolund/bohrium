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
    cphvb_array*                arrayp;
    cphvb_intp                  version;        // enables reverse lookup in ssa table with arrayp
    jit_expr*                   expr;           // the expression with pointers to other expressions.
    
    bool                        is_executed;    // indicate if the arrayp contains the result of expr.
    
    cphvb_instruction*          instr;          // the instruction, which is the basis for this entry
    cphvb_intp                  instr_num;      // the number on the instruction_list.
    cphvb_intp                  operand_num;    // the operand number, 1 if right, 2 if left. only available if expr = array_val.

    bool                        dep_trav_visited;
    
    
    std::set<cphvb_intp>*       tdon;
    std::set<cphvb_intp>*       tdto;

    cphvb_intp                  free_pos;
    cphvb_intp                  discarded_pos;

    // needed ?
    std::vector<cphvb_intp>*    used_at;
    jit_entry_status            status;         // set JIT_DEPTRAV_VISIT when expr.execution list is created.       
    
} jit_name_entry;


typedef struct {
    cphvb_array*                arrayp;
    cphvb_intp                  version;        // enables reverse lookup in ssa table with arrayp
    jit_expr*                   expr;           // the expression with pointers to other expressions.
    jit_entry_status            status;
    bool                        is_executed;    // indicate if the arrayp contains the result of expr.
    std::vector<cphvb_intp>*    DON;            // nametable entries which this expr is dependend on. Max is 2. more then this, is dependent on the dependent once. child of child. 
    std::vector<cphvb_intp>*    DTO;            // nametable entries which it dependent on this expression. Has no max, since the result from the expression can be used by many.
} jit_execute_expr_entry;


typedef struct jit_dependency_graph_node {
    jit_name_entry*                          name_table_entry;
    std::vector<jit_name_entry*>*     dep_ons;
    std::vector<jit_name_entry*>*     dep_tos;    
} jit_dependency_graph_node ;

typedef std::map<cphvb_intp,jit_dependency_graph_node*> jit_dependency_graph;

std::vector<jit_name_entry*>* jita_depgraph_get_dto(jit_dependency_graph* graph, cphvb_intp name);
std::vector<jit_name_entry*>* jita_depgraph_get_don(jit_dependency_graph* graph, cphvb_intp name);
jit_dependency_graph_node* jita_depgraph_get_node(jit_dependency_graph* graph, cphvb_intp name);
jit_dependency_graph_node* jita_depgraph_add_node(jit_dependency_graph* graph, jit_name_entry* name_table_entry);
cphvb_intp jita_depgraph_node_add_don(jit_dependency_graph_node* n,jit_name_entry* onnode);
cphvb_intp jita_depgraph_node_add_dto(jit_dependency_graph_node* n,jit_name_entry* tonode);

typedef std::map<cphvb_array*, std::vector<cphvb_intp>*>    jit_base_dependency_table;
typedef std::vector<jit_name_entry*>                        jit_name_table;
typedef std::map<cphvb_array*, std::vector<cphvb_intp>*>    jit_ssa_map; 
typedef std::vector<cphvb_intp>                             jit_execute_list; 

typedef struct {
    jit_ssa_map*                ssamap ;
    jit_name_table*             nametable;
    jit_base_dependency_table*  base_usage_table;
    jit_dependency_graph*       dep_graph;        
} jit_analyse_state;

jit_name_entry* jita_lookup_name(jit_name_table* nametable, jit_ssa_map* ssamap, cphvb_array* array, cphvb_intp version);

// base array dependency table. (usage_list)     
std::vector<cphvb_intp>*    jita_base_usage_table_get_usage(jit_base_dependency_table* dep_table, cphvb_array* base_array);
cphvb_intp                  jita_base_usage_table_add_usage(jit_base_dependency_table* dep_table, cphvb_array* base_array, cphvb_intp name);

// jit_name_table
cphvb_intp          jita_insert_name(jit_name_table* nametable,jit_ssa_map* ssamap, cphvb_array* array, jit_expr* expr);
cphvb_intp          jita_handle_arithmetic_instruction(jit_name_table* nametable, jit_ssa_map* ssamap, cphvb_instruction* instr);
jit_name_entry*     jita_nametable_lookup(jit_name_table* nametable, cphvb_intp index);
jit_name_entry*     jita_nametable_lookup_a(jit_analyse_state* s, cphvb_array* array);

// array* key with value being a vector of names, where the index is the version.
// jit_ssa_map
cphvb_intp jita_ssamap_version_lookup(jit_ssa_map* ssamap, cphvb_array* array, cphvb_intp version);


// other
jit_analyse_state* jita_make_jit_analyser_state(jit_name_table* nametable, jit_ssa_map* ssamap, jit_base_dependency_table* base_usage_table);
cphvb_intp jita_handle_controll_instruction(jit_name_table* nametable, jit_ssa_map* ssamap, jit_execute_list* exelist, cphvb_instruction* instr);
cphvb_intp jita_handle_arithmetic_instruction2(jit_analyse_state* s,cphvb_instruction* instr, cphvb_intp instr_num);
bool jita_is_controll(cphvb_instruction* instr);



// testing stuff
void jita_run_tests();
void _print_used_at(std::vector<cphvb_intp>* vec);

#endif

