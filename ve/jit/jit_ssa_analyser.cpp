// jit_ssa_analyser.cpp


#ifndef _JIT_LOG_LEVEL
#define _JIT_LOG_LEVEL 2 // 0 NO LOGGING, 1 ERROR, 2 WARNING, 3 INFO, 4 DEBUG
#endif

#include "cphvb.h"
#include "jit_ssa_analyser.h"
#include <utility>
#include "jit_ast.h"


using namespace std;

/** 
 * ssa lookup version table. 
 * Lookup array* in ssalist.
 **/
ssavt* ssa_lookup_vt(map<cphvb_array*,ssavt*>* ssalist,cphvb_array* array) {
    std::map<cphvb_array*,ssavt*>::iterator it = ssalist->find(array);
    if(it == ssalist->end()) {
        return NULL;        
    } 
    return it->second;    
}

cphvb_intp ssa_lookup_array_version(map<cphvb_array*,ssavt*>* ssalist, cphvb_array* array) {    
    ssavt* vt = ssa_lookup_vt(ssalist,array);
    if (vt!=NULL) {   
        return vt->version;    
    }
    return -1;
}

ast* ssa_lookup(map<cphvb_array*,ssavt*>* ssalist, cphvb_array* array, cphvb_intp version) {
    ssavt* vt = ssa_lookup_vt(ssalist,array);
    //std::vector<ast*>* vt = ssa_lookup_vt(ssalist,array);

    if (vt == NULL) {
        return NULL;
    }
    
    if(version > (cphvb_intp)vt->table->size()) {
        return NULL;
    }
    
    return vt->table->at(version);
}

ast* ssa_lookup_latest(map<cphvb_array*,ssavt*>* ssalist, cphvb_array* array) {        
    cphvb_intp version = ssa_lookup_array_version(ssalist,array);    
    if (version == -1) {
        //ast_log("> ssa_lookup_latest() = nothing\n",LOG_INFO);
        return NULL;    
    }   
    
    return ssa_lookup(ssalist,array,version);   
}




bool ssa_add(map<cphvb_array*,ssavt*>* ssalist,cphvb_array* array, ast* expr) {    
    ssavt* vt;
    if (_JIT_LOG_LEVEL >= 3) printf("> ssa_add()\n");
    // lookup the out_base
    cphvb_intp version = ssa_lookup_array_version(ssalist, array);
    
    if (_JIT_LOG_LEVEL >= 3) printf("version available = %ld\n",version);
    if (version == -1) {
        // no versions table for the array*. create one.        
        vt = new ssavt;        
        
        vt->table = new std::vector<ast*>(0);
        vt->version = -1;        
        ssalist->insert(std::pair<cphvb_array*,ssavt*>(array,vt));        
        if (_JIT_LOG_LEVEL >= 3) printf("ssavt created: %p\n", vt);    
    }   
    
    vt = ssa_lookup_vt(ssalist,array);
    
    if (_JIT_LOG_LEVEL >= 3) printf("table pointer: %p\n",vt);
    vt->version++;
    if (_JIT_LOG_LEVEL >= 3) printf("version incremented to: %ld\n",vt->version);
    
    vt->table->push_back(expr); 
    if (_JIT_LOG_LEVEL >= 3) printf("expr added\n");   
    return true;        
}


bool ssa_add_base(map<cphvb_array*,ssavt*>* ssalist, cphvb_array* array, ast* expr) {
    ssavt* vt;
    if (_JIT_LOG_LEVEL >= 3) printf("> ssa_add()\n");
    // lookup the out_base
    cphvb_intp version = ssa_lookup_array_version(ssalist, array);
    
    if (_JIT_LOG_LEVEL >= 3) printf("version available = %ld\n",version);
    if (version == -1) {
        // no versions table for the array*. create one.        
        vt = new ssavt;        
        
        vt->table = new std::vector<ast*>(0);
        vt->version = -1;        
        ssalist->insert(std::pair<cphvb_array*,ssavt*>(array,vt));        
        if (_JIT_LOG_LEVEL >= 3) printf("ssavt created: %p\n", vt);    
    }   
    
    // extra lookup, not that importain.
    vt = ssa_lookup_vt(ssalist,array);
    
    if (_JIT_LOG_LEVEL >= 3) printf("table pointer: %p\n",vt);
    vt->version++;
    if (_JIT_LOG_LEVEL >= 3) printf("version incremented to: %ld\n",vt->version);
    
    vt->table->push_back(expr); 
    if (_JIT_LOG_LEVEL >= 3) printf("expr added\n");   
    return true;   
}


void print_ssa_nametable(std::map<cphvb_array*,ssavt*> nametable) {
    
    std::map<cphvb_array*,ssavt*>::iterator it;
    printf("print_nametable size() == %ld\n",nametable.size());
    for ( it=nametable.begin(); it != nametable.end(); it++ ) {
        
        cphvb_array* key = it->first;
        //ssavt* val = it->second;     
        printf("%p\n",key);
        print_ssavt(it->second);
        printf("\n");
    }    
}



void print_ssavt(ssavt* version_table) {    
    printf("SSAVT :: Current Version %ld\n",version_table->version);
    
    if (version_table->version != -1) {        
        std::vector<ast*>* versions = version_table->table;
        for(int i=0; i < (int)versions->size(); i++) {
            printf("%d :: %p\n",i,versions->at(i));                    
        } 
    } else {
        printf("[NO VERSIONS]\n");
    }
    
    
    //~ std::map<cphvb_intp, ast*>::iterator it;
    //~ std::vector<ast*>::iterator it;
    //~ for(it=version_table->table->begin(); it!=version_table->table->end(); it++) {
        //~ printf("%d :: %p\n",i,versions->at(i));        
    //~ }
    printf("-----------\n");
    
}

bool ssa_remove(cphvb_array* array_name) {
    return true;
}



