// jit_analyser.cpp

#include "jit_analyser.h"
#include "jit_ast.h"
#include <vector>
#include "jit_logging.h"
#include "jit_common.h"

#ifndef _LOG_LEVEL
#define _LOG_LEVEL 0 // NONE
#endif

using namespace std;
//~ cphvb_intp jita_nametable_insert_userfunc(jit_analyse_state* s,vector<cphvb_array*>* outputs,vector<jit_expr*>* inputs,cphvb_instruction* instr);
jit_name_entry* jita_nametable_lookup_a(jit_analyse_state* s, cphvb_array* array);
cphvb_intp jita_lookup_name_a(jit_analyse_state* s, cphvb_array* array);
jit_analyse_state* jita_make_jit_analyser_state(jit_name_table* nametable, jit_ssa_map* ssamap, jit_base_dependency_table* base_usage_table);

/**
 * the opcode of is SYNC, DISCARD, FREE, 
 **/
bool jita_is_controll(cphvb_instruction* instr) {    
    return (instr->opcode == CPHVB_SYNC || 
            instr->opcode == CPHVB_DISCARD || 
            instr->opcode == CPHVB_FREE || 
            instr->opcode == CPHVB_NONE);
}


/**
 * Create and analyse-state, to hold the datastructures used to build and create the nametable, base dependency table 
 **/
jit_analyse_state* jita_make_jit_analyser_state(jit_name_table* nametable, jit_ssa_map* ssamap, jit_base_dependency_table* base_usage_table) {
   jit_analyse_state* s = (jit_analyse_state*) malloc(sizeof(jit_analyse_state));
   s->ssamap = ssamap;
   s->nametable = nametable;
   s->base_usage_table = base_usage_table;
   return s;
}

/**
 * @return 0 if equal. 
 **/
int array_view_deep_compare(cphvb_array* a, cphvb_array* b ) {
    if (a == b) {
        return 0;    
    }
        
    if ( a->base == NULL || b->base == NULL) {
        return 1;
    }
    
    // both are views on a base array?
    if (a->base != b->base) {
        return 2;
    }
    
    // a and b are views on the same basearray.    
    
    // do compares on dimension, start, shape and stride
    if (a->ndim != b->ndim) {
        return 10;
    }
     
    if (a->start != b->start) {
        return 11;                
    }
    
    for(int i=0;i < a->ndim;i++) {
        if (a->shape[i] != b->shape[i]) {
            return 12;
        }
        
        if (a->stride[i] != b->stride[i]) {
            return 13;
        }
    }    
    return 0;    
}

/**
 * Find the element lower or prior to the input element in the base_dep
 **/
cphvb_intp jita_get_prior_element(vector<cphvb_intp>* vectorinput, cphvb_intp elem) {
    if (vectorinput == NULL) 
        return -2;
            
    vector<cphvb_intp>::iterator it;
    cphvb_intp prior = -1;    
    
    for(uint i=0; i<vectorinput->size();i++) {
        if(vectorinput->at(i) < elem) {
            prior = vectorinput->at(i);                                    
        } else {
            break;
        }            
    }
    return prior;    
}

// ==============================
// Static Single Assignment Map
// ==============================

/**
 * @return a version-vector with mappings to the nametable.
 **/
vector<cphvb_intp>* jita_ssamap_lookup(jit_ssa_map* ssamap,cphvb_array* array) {
    logInfo("jita_ssamap_lookup()\n");
    jit_ssa_map::iterator it = ssamap->find(array);
    if (it != ssamap->end()) {
        return it->second;
    }
    return NULL;
}


/**
 * Lookup an array in the ssamap table and returns its name in the nametable.
 * @version the array version to lookup. If version == -1, return the highest version.
 * @return name of the array in the nametable. -1 if no entry in the ssamap. 
 **/
cphvb_intp jita_ssamap_version_lookup(jit_ssa_map* ssamap, cphvb_array* array, cphvb_intp version) {
    logInfo("jita_ssamap_lookup()\n");    
    jit_ssa_map::iterator it = ssamap->find(array);
    if (it != ssamap->end()) {        
        std::vector<cphvb_intp>* versions = it->second;
        
        // return the latest version name
        if(version == -1) {
            return versions->back();
        }
        
        // return the specified version name
        if ( (long)versions->size() > (version-1)) {
            return versions->at(version);
        }
    }   
    
    // no entry      
    return -1;
}

/**
 * @return the version of the just inserted 
 */
cphvb_intp _jita_ssamap_insert(jit_ssa_map* ssamap,cphvb_array* array, cphvb_intp name) {    
    logDebug("_jita_ssamap_insert()\n");        
    cphvb_intp next_version  = jita_ssamap_version_lookup(ssamap,array,-1) + 1;
        
    logDebug("array* %p next version = %ld\n",array,next_version);    
    std::vector<cphvb_intp>* versions_vector = jita_ssamap_lookup(ssamap,array);
    if (next_version == 0) {
        // create the entry in the ssamap        
        versions_vector = new std::vector<cphvb_intp>();    
        ssamap->insert(std::pair<cphvb_array*,std::vector<cphvb_intp>*>(array,versions_vector));            
    } 
    
    versions_vector->push_back(name);
    return next_version;
}


// ==============================
// Nametable
// ==============================

/**
 * Lookup a name in the nametable. The name coresponds to the index. 
 * @return A pointer to a jit_name_entry object associated with th ename 
 *      or NULL if the index is out of range. 
 **/
jit_name_entry* jita_nametable_lookup(jit_name_table* nametable, cphvb_intp index) {
    logInfo("jita_nametable_lookup()\n");
    if( (index != -1) && ((int)nametable->size() > index-1)) {    
        return nametable->at(index);
    }           
    return NULL;
}


/**
 * Insert a jit_name_entry into the nametable. It is appended to the 
 * vector with its index as "name".
 * @return name
 */
cphvb_intp _jita_nametable_insert(jit_name_table* nametable,jit_name_entry* entry) {
    nametable->push_back(entry);
    return ( nametable->size()-1 );    
}

/**
 * insert a assignment with array* = expression into the nametable.
 * @return name in the nametable. -1 if error.
 */
cphvb_intp jita_insert_name(jit_name_table* nametable, jit_ssa_map* ssamap, cphvb_array* array, jit_expr* expr) {        
    logInfo("jita_insert_name()\n");
    logDebug(">>> arrayp* %p\n",array);
    logDebug(">>> expr*   %p\n",expr);
        
    // the ssamap, is a mapping from array* to nametable. We thus start
    // by creating the name_table entry.
    
    // get the highest version of the array
    cphvb_intp next_version = jita_ssamap_version_lookup(ssamap,array,-1)+1;
    
    // create the name entry for the name table.
    jit_name_entry* name_entry = new jit_name_entry();
    name_entry->version = next_version;    
    name_entry->expr = expr;
    name_entry->arrayp = array;      
    //name_entry->used_at = new std::vector<cphvb_intp>();

    logDebug(">>> - entry %p\n",name_entry);
    
    // insert into nametable    
    cphvb_intp name = _jita_nametable_insert(nametable,name_entry);        
    logDebug(">>> - name %ld\n",name);
    
    // insert into ssamap
    //cphvb_intp added_version = _jita_ssamap_insert(ssamap,array,name);    
    _jita_ssamap_insert(ssamap,array,name);    
    // set the name given to the assignment, to the expr.
    expr->name = name;
    
    return name;
}


/**
 * For a view, get the last view which is a view on the exact same data.
 * 
 * Views for base arrays are used as temp arrays but a new view for the base are generated
 * each time. The register renameing method does not catch this and a deep compare is need to
 * ensure correctly created expressions.
 **/
cphvb_intp jita_lookup_name_deep(jit_base_dependency_table* dep_table,jit_name_table* nametable, jit_ssa_map* ssamap, cphvb_array* array) {    
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"JLND jita_lookup_name_deep(%p)\n",array);
    
    cphvb_array* base = cphvb_base_array(array);
    // validate input   
    if(base->data == NULL) {
        return -1;
        logcustom(cloglevel,1,"JLND base->data == NULL\n");    
    }
    
    // get the base_usagelist for the array's base.    
    std::vector<cphvb_intp>* base_deps = jita_base_usage_table_get_usage(dep_table,base);
    if(base_deps == NULL) {
        logcustom(cloglevel,1,"JLND base_deps == NULL\n");    
        return -1;
    }
        
    // go through the baselist, and lookup the expressions changeing the base, to look for
    // equal view to array.
    jit_name_entry* entr;
    std::vector<cphvb_intp>::reverse_iterator it;

    for(int i=base_deps->size()-1; i>=0;i--) {    
        entr = jita_nametable_lookup(nametable,base_deps->at(i));
        logcustom(cloglevel,1,"JLND cc %p %d NTentry: %p\n",nametable,base_deps->at(i),entr);
        if (entr != NULL) {
            int res = array_view_deep_compare(entr->arrayp,array);
            
            //printf("compare %p == %p = %d \n",entr->arrayp,array,res);                      
            if (res == 0) {
                logcustom(cloglevel,1,"JLND returning %d\n",nametable,base_deps->at(i));           
                return base_deps->at(i);
            }                    
        }
    }
    return -1;
}


/**
 * 
 **/
cphvb_intp jita_lookup_name_deep_s(jit_analyse_state* s, cphvb_array* array) {
    return jita_lookup_name_deep(s->base_usage_table,s->nametable, s->ssamap,array);
}


/**
 * Inteligent lookup for an array pointer in the nametable.
 * If the arraypointer is not found in the nametable a deep
 * lookup is performed if the array has a base array.
 **/
cphvb_intp jita_lookup_name_a(jit_analyse_state* s, cphvb_array* array) {
    bool cloglevel[] = {0};
    logcustom(cloglevel,0,"jita_lookup_name_a()\n");    
    //printf("%p b> \n",array);
    cphvb_intp name = jita_ssamap_version_lookup(s->ssamap,array,-1);    
    logcustom(cloglevel,0,"JLNA %ld - base array%p\n",name, array->base);
    //if ((name == -1) && (cphvb_base_array(array)->data != NULL)) {
    if ((name == -1) && (cphvb_base_array(array) != array)) {                            
        name = jita_lookup_name_deep_s(s,array);        
    }    
    return name;
}


/**
 * 
 **/
jit_name_entry* jita_nametable_lookup_a(jit_analyse_state* s, cphvb_array* array) {    
    logDebug("jita_nametable_lookup_a(%p)\n",array);
    
    cphvb_intp name = jita_lookup_name_a(s,array);
    
    //printf("jita_nametable_lookup_a >> name : %ld for %p\n",name,array);
    //jit_pprint_nametable(s->nametable);
    jit_name_entry* e = jita_nametable_lookup(s->nametable,name);    
    return e;
}


// ==============================
// Base Dependency 
// ==============================

/**
 * Get a list of usages of a basearray. the list contains the nametable names where the basearray have been writen to.
 **/
vector<cphvb_intp>* jita_base_usage_table_get_usage(jit_base_dependency_table* dep_table, cphvb_array* base_array) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"JBUTGU %p\n",base_array);    
    jit_base_dependency_table::iterator it = dep_table->find(base_array);    
    if (it==dep_table->end()) { 
        logDebug("Deptable does not contain basearray\n");
        return NULL;         
    }
    return it->second;
}


/**
 * Add the use of an basearray by a nametable entry, name.
 **/
cphvb_intp jita_base_usage_table_add_usage(jit_base_dependency_table* dep_table, cphvb_array* base_array, cphvb_intp name) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"JBUTAU %ld to %p\n",name,base_array);
    jit_base_dependency_table::iterator it = dep_table->find(base_array);
    it = dep_table->find(base_array);
    if (it==dep_table->end()) {
        (*dep_table)[base_array] = new vector<cphvb_intp>(0);                
    }
    logcustom(cloglevel,1,"JBUTAU adding %ld to %p\n",name,base_array);
    (*dep_table)[base_array]->push_back(name);
    return 0;
}


/**
 * Base array dependencies analysis, visit functions.
 **/
cphvb_intp update_expr_dependencies2(jit_analyse_state* s, jit_name_entry* parent, jit_expr* expr) {
    bool cloglevel[] = {0,0,0};
    logcustom(cloglevel,0,"update_expr_dependencies2(state,%ld,%ld)\n",parent->expr->name,expr->name);
    cphvb_array* basea = NULL;
    vector<cphvb_intp>* base_dep;
        
    if (!is_constant(expr)) {
        jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);        
        logcustom(cloglevel,1,"UED2 ! %p %p\n",entr->arrayp->base, entr->arrayp->data);
        
        basea = cphvb_base_array(entr->arrayp);
        logcustom(cloglevel,1,"UED2 -basea: %p\n",basea);
        
        // Child must write to a BoundedArray, before parent can be dependent on it        
        //if (basea->data != NULL) {                
            base_dep = jita_base_usage_table_get_usage(s->base_usage_table,basea);

            // get the element which write to the same basearray last.
            cphvb_intp dep_on_name = jita_get_prior_element(base_dep,parent->expr->name);

            logcustom(cloglevel,1,"UED2 base: basedeps:%d depON: %ld\n",base_dep->size(),dep_on_name);
            if (cloglevel[2]) {
                logcustom(cloglevel,2,"dependencylist for basea: %p",basea);
                jit_pprint_base_dependency_list(base_dep);
            }
            
            // if parent did not write to the base array
            // if not self and a sanity check
            if (dep_on_name != -1 && dep_on_name != parent->expr->name) {
               // logcustom(cloglevel,1,"UED2 parent->tdon: %p, entr->tdto: %p\n",parent->tdon,entr->tdto);
                logcustom(cloglevel,1,"UED2 --- (%ld,%ld) (%ld,%ld)\n",parent->expr->name,dep_on_name,entr->expr->name,parent->expr->name);
                // nametable entries dependency
                parent->tdon->insert(dep_on_name);
                jita_nametable_lookup(s->nametable,dep_on_name)->tdto->insert(parent->expr->name);
                //entr->tdto->insert(parent->expr->name);
            }
        //}        
    }
    return 1;
}

/**
 * Perform dependency analysis on the analyse_state. The analysis is done 
 * by going thruw the nametable and dertermining the depency for each
 * element. If offset is set it is used as the starting entry for the
 * analysis.
 **/
void jita_perform_dependecy_analysis(jit_analyse_state* s, cphvb_index offset) {    
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"JPDA jita_perform_dependecy_analysis()\n");
    
    vector<cphvb_intp>* base_dep;    

    jit_name_entry* entr;            
    cphvb_array* basea;
    cphvb_intp name;
    cphvb_intp dep_on_name;    
    
    
    int start = offset;    
    cphvb_index ntcount = 0;

    //logcustom(cloglevel,1,"nametable size %d \n",s->nametable->size());
    for(uint j=start; j < s->nametable->size(); j++,ntcount++) {
        logcustom(cloglevel,1,"JPDA NT: I %d name%ld\n",j,ntcount);
        
        // iterator points to name_entry.
        entr = s->nametable->at(j);
        
        if(entr->is_userfunction) {
            logcustom(cloglevel,1,"JPDA userfunc\n");
            
            // determin dependencies for the input expressions.
            for(uint i=0;i<entr->expr->userfunction_inputs->size();i++) {                
                logcustom(cloglevel,1,"JPDA in: updating expr deps: %d . total:%d\n",i,entr->expr->userfunction_inputs->size());
                update_expr_dependencies2(s,entr,entr->expr->userfunction_inputs->at(i));
            }
            
            jit_name_entry* tentr;
            for(int i=0;i < entr->span;i++) {
                tentr = jita_nametable_lookup(s->nametable,j+i);      
                basea = cphvb_base_array(tentr->arrayp);
                logcustom(cloglevel,1,"JPDA I %d basea: %p \n",i,basea);
                //if (basea->data != NULL) {                   
                    base_dep = jita_base_usage_table_get_usage(s->base_usage_table,basea);            
                    dep_on_name = jita_get_prior_element(base_dep,j+i);            
                    logcustom(cloglevel,1,"base_dep.size() %d. dep_on: %d name:%d\n",base_dep->size(),dep_on_name,name);
                    
                    if (dep_on_name > -1) {
                        // the entry depends on dep_on_name.
                        // add dep_on_name to DependON list for the entry.
                        logcustom(cloglevel,1,"JPDA adding entr: %d tdon: %d\n",j+i, dep_on_name);
                        tentr->tdon->insert(dep_on_name);                
                        jita_nametable_lookup(s->nametable,dep_on_name)->tdto->insert(tentr->expr->name);                                
                    }                            
                //}                
            }
            j+= (entr->span-1);

            
        } else {
            logcustom(cloglevel,1,"JPDA %d = not userfunctions\n",entr->expr->name);        
            name = entr->expr->name;                                

            // dependency check the left expression    
            if(is_un_op(entr->expr) || is_bin_op(entr->expr)) {  // left expr dependency
                logcustom(cloglevel,1,"JPDA un ");
                update_expr_dependencies2(s,entr,entr->expr->op.expression.left);                        
            }
            
            // dependency check the right expression
            if(is_bin_op(entr->expr)) {  // right expr dependency
                logcustom(cloglevel,1,"JPDA bin ");
                update_expr_dependencies2(s,entr,entr->expr->op.expression.right);            
            }

            // determin dependencies for target array.
            basea = cphvb_base_array(entr->arrayp);

            //if (basea->data != NULL) {
                // The entr affects the basearray by changeing it.
                // This has been recorded in the 'instruction handler', who build
                // the initial analyse_state.

                // this also means that this is dependent on previous changes.
                // It is unknown how this entry, (with the view and expression), changes
                // the basea. It could be writing a single value, a row or everything is changed.
                // This we do not know. (Might be able to figure this out later).
                
                // Add lookup usage_list of the basea.
                
                base_dep = jita_base_usage_table_get_usage(s->base_usage_table,basea);
                dep_on_name = jita_get_prior_element(base_dep,name);
                //printf("base_dep.size() %d. dep_on: %d name:%d\n",base_dep->size(),dep_on_name,name);
                logcustom(cloglevel,1,"JPDA %p %p,%ld\n",basea,base_dep,dep_on_name);
                if (dep_on_name > -1) {
                    // the entry depends on dep_on_name.
                    // add dep_on_name to DependON list for the entry.
                    logcustom(cloglevel,1,"JPDA --- (%ld,%ld) (%ld,%ld)\n",entr->expr->name,dep_on_name,jita_nametable_lookup(s->nametable,dep_on_name)->expr->name,entr->expr->name);
                    entr->tdon->insert(dep_on_name);                    
                    jita_nametable_lookup(s->nametable,dep_on_name)->tdto->insert(entr->expr->name);                                
                }                            
            //}
        }        
    } // end for loop
}



// ==============================
// Instruction Handler functions
// ==============================

/**
 * requires that the array is not already registered.
 *  jita_nametable_lookup_a(s, array) == NULL.
 **/
jit_name_entry* new_array_creation(jit_analyse_state* s, cphvb_array* array) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"NAC new_array_creation(,%p)\n",array);
    jit_name_entry* entr;   
    //logcustom(cloglevel,1,"NAC entr: %p\n",entr);

    jit_expr* expr = cphvb_array_to_jit_expr(array);
    logcustom(cloglevel,1,"NAC new expr: %p\n",expr);
    cphvb_intp expr_name = jita_insert_name(s->nametable,s->ssamap,array,expr);
    entr = jita_nametable_lookup(s->nametable,expr_name);                    
    entr->tdto = new set<cphvb_intp>();
    entr->tdon = new set<cphvb_intp>();
    
    // add only usage if the basearray had not been added before!                                  
    if (jita_base_usage_table_get_usage(s->base_usage_table,cphvb_base_array(array)) == NULL) {
        logcustom(cloglevel,1,"NAC adding %ld to base: %p\n",expr_name,cphvb_base_array(array));
        jita_base_usage_table_add_usage(s->base_usage_table,cphvb_base_array(array),expr_name);
    }
    
    return entr;            
}



/**
 * Handle the free instruction by simply registering it in the nametable entry for the array.
 **/ 
cphvb_intp jita_handle_free_instruction(jit_analyse_state* s,cphvb_instruction* instr, cphvb_intp instr_num) {
    bool cloglevel[] = {0};
    // lookup instruction
    //cphvb_array* base = cphvb_base_array(instr->operand[0]);
    jit_name_entry* e = jita_nametable_lookup_a(s,instr->operand[0]);
    if (e == NULL) {
        // trying to free a un-registered instruction
        return 1;
    }    
    
    // set last nametable entry as the point in "analyse time" where the free happened.
    // For temp array this will then be the same as the array name. 
    e->freed_at = s->nametable->size();
    logcustom(cloglevel,0,"freed registered: %ld: %ld\n",e->expr->name, s->nametable->size());
    return 0;
}


/**
 * Handle the discard instruction by simply registering it in the nametable entry for the array.
 **/ 
cphvb_intp jita_handle_discard_instruction(jit_analyse_state* s,cphvb_instruction* instr, cphvb_intp instr_num) {
    bool cloglevel[] = {0};
    // lookup instruction
    //cphvb_array* base = cphvb_base_array(instr->operand[0]);
    jit_name_entry* e = jita_nametable_lookup_a(s,instr->operand[0]);
    if (e == NULL) {
        // trying to discard a un-registered instruction
        return 1;
    }
    //printf("%p %p %ld %p\n",s,instr->operand[0],instr_num,e);
    
    // set last nametable entry as the point in "analyse time" where the free happened.
    // For temp array this will then be the same as the array name. 
    e->discarded_at = s->nametable->size();
    logcustom(cloglevel,0,"discarded registered: %ld: %ld\n",e->expr->name, s->nametable->size());
    return 0;
}


/**
 * Handling a userdefined function
 **/ 
cphvb_intp jita_handle_userfunc_instruction(jit_analyse_state* s,cphvb_instruction* instr, cphvb_intp instr_num) {
    /* // handle the userdefined function as a dependency hotspot tree node.
    // Thus a must-execute where all inputs must be pre-executed. 

    // add to nametable.
    // set nametable-entry to *userfunc*
    // add all input as DON
    // add all output as DTO

    // futher
    // ignore in the dependecy analysis
    // add to execution list + with input, in the executionlist creation. */
    bool cloglevel[] = {0,0,0};
    logcustom(cloglevel,0,"jita_handle_userfunc_instruction(,%d)\n",instr_num);
    //cphvb_pprint_instr(instr);
    cphvb_userfunc* userfunc = instr->userfunc;
    
    // lookup all input
    jit_name_entry* entr;
    vector<jit_expr*>* in_exprs = new vector<jit_expr*>(userfunc->nin);

    // create userdefined-function expression.
    // expr name will point to the last NT entry for the userfunction
    jit_expr* expr = new jit_expr();
    expr->tag = expr_type_userfunc;
    expr->op.userfunc = userfunc;
    expr->userfunction_inputs = in_exprs;
    expr->depth = 0;
    expr->parent = NULL;

    
    // input
    for(int i=userfunc->nout; i < userfunc->nin + userfunc->nout; i++) {
        entr = jita_nametable_lookup_a(s, userfunc->operand[i]);
        if (entr == NULL) {
            logcustom(cloglevel,1,"IN ++ %d %p\n",i,userfunc->operand[i]);
            entr = new_array_creation(s,userfunc->operand[i]);            
            entr->instr = instr;
            entr->instr_num = instr_num;
            entr->operand_num = i;            
            entr->expr->parent = expr;
        }
        logcustom(cloglevel,1,"IN in %ld ,on %ld \n",entr->instr_num,entr->operand_num);
        logcustom(cloglevel,1,"IN inserting into in_exprs %p at %d of %d\n", userfunc->operand[i],i,userfunc->nout-i);
        
        in_exprs->at(userfunc->nout-i) = entr->expr;        
    }
    
    for(int i=0;i<userfunc->nout ;i++) {
        entr = jita_nametable_lookup_a(s, userfunc->operand[i]);
        
        cphvb_intp name = jita_insert_name(s->nametable, s->ssamap, userfunc->operand[i], expr);                
        jit_name_entry* e = jita_nametable_lookup(s->nametable,name);
        e->instr = instr;
        e->instr_num = instr_num;
        e->operand_num = i;
        e->span = userfunc->nout;
        e->span_index = i;        
        e->tdto = new set<cphvb_intp>();
        e->tdon = new set<cphvb_intp>();
        e->is_userfunction = true;
        logcustom(cloglevel,2,"JHUI :\n");     

        // add operand0 assignment to the base_array_usages_list.
        //if (cphvb_base_array(userfunc->operand[i])->data != NULL) {
        logcustom(cloglevel,2,"JHUI O Adding %ld to base_usage %p\n",name,cphvb_base_array(userfunc->operand[i]));
        jita_base_usage_table_add_usage(s->base_usage_table,cphvb_base_array(userfunc->operand[i]),name);
        //}
    }  
    //jit_pprint_nametable(s->nametable);
    return 0;
}


/**
 * Handling of an mathmatical instruction. Creating and inserting it into the nametable,
 * performing register renameing and creating and AST representation the expression.
 **/ 
cphvb_intp jita_handle_arithmetic_instruction3(jit_analyse_state* s, cphvb_instruction* instr, cphvb_intp instr_num) {
    bool cloglevel[] = {0,0,0};
    logcustom(cloglevel,0,"s jita_handle_arithmetic_instruction3()\n");    
    logcustom(cloglevel,1,"JHAI3 %p, %ld\n",instr,instr_num);    
    // get the first and second operands. Look them up, to see if they 
    // are already registered. If not register them, else reference the 
    // registered one and add "used_at".
    
    jit_base_dependency_table::iterator it;
    jit_expr* first = NULL;        
    jit_expr* second = NULL;    
    jit_expr* expr = new jit_expr();
    jit_name_entry* e;
    
    jit_expr_tag instr_tag;
    cphvb_intp expr_depth = 1;                
    
    //jit_name_entry* first_entry = jita_lookup_name(nametable,ssamap, instr->operand[1],-1);        

    // first or left operand 
    jit_name_entry* first_entry = NULL;
    if (instr->operand[1] == NULL) {
        first = cphvb_constant_to_jit_expr(&instr->constant);
        //operand_to_exp(instr,1,first);            
    } else {
        first_entry = jita_nametable_lookup_a(s, instr->operand[1]);         
        if (first_entry == NULL) {                                
            first_entry = new_array_creation(s,instr->operand[1]);            
            first_entry->instr = instr;
            first_entry->instr_num = instr_num;
            first_entry->operand_num = 1;            
            first_entry->expr->parent = expr;                                    
        } 
        first = first_entry->expr;                        
    }
                
    instr_tag = un_op;
    expr_depth = first->depth;                        
    
    // second or right operand if the instruction is binary.
    jit_name_entry* second_entry = NULL;    
    if (cphvb_operands(instr->opcode) == 3) {                    
        //second_entry = jita_lookup_name(nametable,ssamap, instr->operand[2],-1);
        //cphvb_pprint_instr(instr);
        if (instr->operand[2] == NULL) {
            second = cphvb_constant_to_jit_expr(&instr->constant);                
        } else {
            second_entry = jita_nametable_lookup_a(s, instr->operand[2]);
            if (second_entry == NULL) {    
                second_entry = new_array_creation(s,instr->operand[2]);            
                second_entry->instr = instr;
                second_entry->instr_num = instr_num;
                second_entry->operand_num = 2;            
                second_entry->expr->parent = expr;                                            
            }            
            second = second_entry->expr;
        }
                            
        instr_tag = bin_op;
        expr_depth = std::max(first->depth,second->depth);
    }
    logcustom(cloglevel,1,"JHAI3 operand 1 and 2 created/extracted.\n");

    // create expr. 
    expr->tag = instr_tag;
    expr->op.expression.opcode = instr->opcode;          
    expr->op.expression.left = first;
    expr->op.expression.right = second;
    expr->depth = expr_depth + 1;
    expr->parent = NULL;

    // setting parent of child nodes
    first->parent = expr;
    if (second != NULL) {
        second->parent = expr;
    }
        
    // insert the new expression    
    cphvb_intp name = jita_insert_name(s->nametable, s->ssamap, instr->operand[0], expr);
    
    e = jita_nametable_lookup(s->nametable,name);
    logcustom(cloglevel,1,"JHAI3, %p %ld\n",e,name);
    e->instr = instr;
    e->instr_num = instr_num;
    e->operand_num = 0;
    e->tdto = new set<cphvb_intp>();
    e->tdon = new set<cphvb_intp>();
    e->is_userfunction = false;
    e->freed_at = -1;
    e->discarded_at = -1;
        
    logcustom(cloglevel,2,"JHAI3 O Adding %ld to base_usage %p\n",name,cphvb_base_array(instr->operand[0]));
    jita_base_usage_table_add_usage(s->base_usage_table,cphvb_base_array(instr->operand[0]),name);        
    
    return 1;
}
