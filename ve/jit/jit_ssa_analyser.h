// jit ssa analyser
//

#ifndef __JIT_SSA_ANALYSER_H
#define __JIT_SSA_ANALYSER_H

#include "cphvb.h"
#include "jit_ast.h"
#include <map>


typedef struct SSA_Version_Table {
    cphvb_intp  version;
    //std::map<cphvb_intp, cphvb_intp>* table;
    std::map<cphvb_intp, ast*>* table;
} ssavt;


//cphvb_error ssa_lookup(map<cphvb_intp, *ssavt> ssalist, cphvb_array* array, ssavt* out);


//cphvb_error ssa_add(map<cphvb_intp, cphvb_intp> ssalist, cphvb_array* key_pointer, ast* expr);



cphvb_error ssa_lookup(std::map<cphvb_array*,ssavt*>* ssalist, cphvb_array* array, ssavt* out);

cphvb_error ssa_add(std::map<cphvb_array*,ssavt*>* ssalist, cphvb_array* array, ast* expr);


#endif
