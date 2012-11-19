// jit ssa analyser
//


#ifndef __JIT_SSA_ANALYSER_H
#define __JIT_SSA_ANALYSER_H

#include "cphvb.h"
#include "jit_ast.h"
#include <map>
#include <vector>


typedef struct SSA_Version_Table1 {
    cphvb_intp  version;
    //std::map<cphvb_intp, cphvb_intp>* table;
    std::map<cphvb_intp, ast*>* table;
} ssavt1;

typedef struct SSA_Version_Table {
    cphvb_intp  version;
    //std::map<cphvb_intp, cphvb_intp>* table;
    std::vector<ast*>* table;
} ssavt;

typedef std::map<cphvb_array*,ssavt*> ssalist_t;

// a array* -> vector[version][ list of <array,version>  ]
typedef std::map<cphvb_array*,std::vector<std::vector<std::pair<cphvb_array*, cphvb_intp>* >* >* > ssa_used_at_t;




ssavt* ssa_lookup_vt(std::map<cphvb_array*,ssavt*>* ssalist,cphvb_array* array);
cphvb_intp ssa_lookup_array_version(std::map<cphvb_array*,ssavt*>* ssalist, cphvb_array* array);
ast* ssa_lookup(std::map<cphvb_array*,ssavt*>* ssalist, cphvb_array* array,cphvb_intp version);
ast* ssa_lookup_latest(std::map<cphvb_array*,ssavt*>* ssalist, cphvb_array* array) ;
bool ssa_add(std::map<cphvb_array*,ssavt*>* ssalist,cphvb_array* array, ast* expr);

//void ast_handle_instruction_ssa(std::list<ast*>* expression_list, std::map<cphvb_array*,ssavt*>* nametable, cphvb_instruction* instr);
//void ast_handle_instruction_ssa_ua(ssa_used_at_t ssauaTable, std::list<ast*>* expression_list, std::map<cphvb_array*,ssavt*>* nametable, cphvb_instruction* instr);

void print_ssavt(ssavt* version_table); 

#endif
