// jit_ssa_analyser.cpp


#include "cphvb.h"
#include "jit_ssa_analyser.h"
#include <utility>

using namespace std;

//cphvb_error ssa_lookup(map<cphvb_array*,ssavt*> ssalist, cphvb_array* array, ssavt* out) 
//{    
    //// error handling missing
    
    //std::map<cphvb_array*,ssavt*>::iterator it = ssalist.find(array);
    //if(it == ssalist.end()) {
        //out = NULL;        
    //} else {
        //out = it->second;
    //}
    
    //return CPHVB_SUCCESS;
//}



/**  
 * Lookup the SSA name and exp for the array in the ssalist. If the array
 * exists the ssa_version_table for the base array is returned.
 * If it does not exists out is null
 * */
cphvb_error ssa_lookup(map<cphvb_array*,ssavt*> ssalist, cphvb_array* array, ssavt* out) 
{    
    // error handling missing
    
    std::map<cphvb_array*,ssavt*>::iterator it = ssalist.find(array);
    if(it == ssalist.end()) {
        out = NULL;        
    } else {
        out = it->second;
    }
    
    return CPHVB_SUCCESS;
}



// cphvb_instruction* inst
cphvb_error ssa_add(map<cphvb_array*,ssavt*> ssalist, cphvb_array* key_pointer, ast* expr) {    
    ssavt* vt;
    cphvb_error result;
    // lookup the out_base
    result = ssa_lookup(ssalist, key_pointer, vt);
    if (result != CPHVB_SUCCESS) {
        return result;
    }
        
    if (vt == NULL) {
        std::map<cphvb_intp,ast*> version_table;
        version_table = std::map<cphvb_intp,ast*>();
        version_table[0] = expr;
        
        // no entry exist for the array, create one.
        vt = (ssavt*) malloc(sizeof(ssavt));
        vt->version = 0;
        vt->table = &version_table;        
        //vt->table[0] = expr;
        
        ssalist[key_pointer] = vt;
        
    } else {
        
        vt->version++;
        //vt->table[0][0] = expr;                
        //(*vt->table).insert(std::pair<cphvb_intp,ast*>(vt->version,expr));                
        (*vt->table)[vt->version] = expr;
        //std::map<cphvb_intp,ast*> version_table = *vt->table;
        //version_table.insert(std::pair<cphvb_intp,ast*>(vt->version,expr));                
    }
    

    // assign

    return CPHVB_SUCCESS;    
}
