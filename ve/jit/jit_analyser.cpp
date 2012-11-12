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
 * Lookup a name in the nametable. The name coresponds to the index. 
 * @return A pointer to a jit_name_entry object associated with th ename 
 *      or NULL if the index is out of range. 
 **/
jit_name_entry* jita_nametable_lookup(jit_name_table* nametable, cphvb_intp index) {
    logInfo("jita_nametable_lookup()\n");
    if( (index != -1) && ((int)nametable->size() > index-1)) {    
        return nametable->at(index);
    }       
    
    // TODO: handle error (out of bound exception)    
    return NULL;
}    

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

jit_name_entry* jita_nametable_lookup_by_array(jit_name_table* nametable, jit_ssa_map* ssamap, cphvb_array* array, cphvb_intp version) { }
 

/**
 * lookup_name()
 * 
 * look up the jit_name_entry in the nametable for the array with the specified version.
 * @version The array version to get. If version == -1, return the highest version.
 */
jit_name_entry* jita_lookup_name(jit_name_table* nametable, jit_ssa_map* ssamap, cphvb_array* array, cphvb_intp version) {    
    logDebug("jita_lookup_name()\n");
    cphvb_intp name = jita_ssamap_version_lookup(ssamap,array,version);
    if (name == -1) { // no name
        return NULL;     
    }     
    // lookup in the nametable
    return jita_nametable_lookup(nametable, name);
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

cphvb_intp _jita_update_used_at(jit_name_table* nametable,jit_ssa_map* ssamap,cphvb_intp used_at_name, cphvb_intp name) {    
    logDebug("_jita_update_used_at()\n");
    logDebug("input: used_at_name = %ld \n name: %ld\n",used_at_name,name);    
    
    // validate existense of used_at_array
    jit_name_entry* used_at_entry = jita_nametable_lookup(nametable,used_at_name);        
    if (used_at_entry == NULL) {
        logDebug(" used_at_name %ld , NOT FOUND\n",used_at_name);         
        return -1;
    }    
    
    // add name, to the used_at_vector
    used_at_entry->used_at->push_back(name);
        logDebug("entry = %p\n",used_at_entry);
                   
    return 1;
}

cphvb_intp _jita_update_used_at(jit_name_table* nametable,jit_ssa_map* ssamap,jit_name_entry* used_at_entry, cphvb_intp name) {
    logDebug("_jita_update_used_at\n");
    logDebug("input: used_at_name = %p \n name: %ld\n",used_at_entry,name);    
    
    // validate existense of used_at_array
    //jit_name_entry* used_at_entry = jita_nametable_lookup(nametable,used_at_name);        
    if (used_at_entry == NULL) {
        logDebug(" used_at_name %p , NOT FOUND\n",used_at_entry);         
        return -1;
    }    
    
    // add name, to the used_at_vector
    used_at_entry->used_at->push_back(name);
    logDebug("entry = %p\n",used_at_entry);
                   
    return 1;
}

jit_analyse_state* jita_make_jit_analyser_state(jit_name_table* nametable, jit_ssa_map* ssamap, jit_base_dependency_table* base_usage_table) {
   jit_analyse_state* s = (jit_analyse_state*) malloc(sizeof(jit_analyse_state));
   s->ssamap = ssamap;
   s->nametable = nametable;
   s->base_usage_table = base_usage_table;
   return s;
}


/// find the element lower or prior to the input element in the base_dep 
cphvb_intp jita_get_prior_element(vector<cphvb_intp>* base_dep, cphvb_intp elem) {
    if (base_dep == NULL)
        return -2;
    
    vector<cphvb_intp>::iterator it;
    cphvb_intp prior = -1;
    int i;
    
    for(i=0; i<base_dep->size();i++) {
        if(base_dep->at(i) < elem) {
            prior = base_dep->at(i);                                    
        } else {
            break;
        }            
    }
    return prior;    
}



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
        if (basea->data != NULL) {                
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
                    logcustom(cloglevel,1,"UED2 parent->tdon: %p, entr->tdto: %p\n",parent->tdon,entr->tdto);

                // nametable entries dependency                
                parent->tdon->insert(dep_on_name);                
                entr->tdto->insert(parent->expr->name);                
            }        
        }
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
    for(int j=start; j < s->nametable->size(); j++,ntcount++) {
        logcustom(cloglevel,1,"JPDA NT: I %d name%ld\n",j,ntcount);
        
        // iterator points to name_entry.
        entr = s->nametable->at(j);
        
        if(entr->is_userfunction) {
            logcustom(cloglevel,1,"JPDA userfunc\n");
            
            // determin dependencies for the input expressions.
            for(int i=0;i<entr->expr->userfunction_inputs->size();i++) {                
                logcustom(cloglevel,1,"JPDA in: updating expr deps: %d . total:%d\n",i,entr->expr->userfunction_inputs->size());
                update_expr_dependencies2(s,entr,entr->expr->userfunction_inputs->at(i));
            }
            
            jit_name_entry* tentr;
            for(int i=0;i < entr->span;i++) {
                tentr = jita_nametable_lookup(s->nametable,j+i);      
                basea = cphvb_base_array(tentr->arrayp);
                logcustom(cloglevel,1,"JPDA I %d basea: %p \n",i,basea);
                if (basea->data != NULL) {                   
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
                }                
            }
            j+= (entr->span-1);

            
        } else {
            logcustom(cloglevel,1,"JPDA %d = not userfunctions\n",entr->expr->name);        
            name = entr->expr->name;                                
                
            if(is_un_op(entr->expr) || is_bin_op(entr->expr)) {  // left expr dependency
                logcustom(cloglevel,1,"un ");
                update_expr_dependencies2(s,entr,entr->expr->op.expression.left);                        
            }
            
            if(is_bin_op(entr->expr)) {  // right expr dependency
                logcustom(cloglevel,1,"bin ");
                update_expr_dependencies2(s,entr,entr->expr->op.expression.right);            
            }

            // determin dependencies for target array.
            basea = cphvb_base_array(entr->arrayp);

            if (basea->data != NULL) {
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
                
                if (dep_on_name > -1) {
                    // the entry depends on dep_on_name.
                    // add dep_on_name to DependON list for the entry.
                    
                    entr->tdon->insert(dep_on_name);                
                    jita_nametable_lookup(s->nametable,dep_on_name)->tdto->insert(entr->expr->name);                                
                }                            
            }
        }        
    } // end for loop
}


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
    
    //if (array->base != NULL || array->data != NULL ) {            
    if (cphvb_base_array(array)->data != NULL) {        
        // add only usage if the basearray had not been added before!                                  
        if (jita_base_usage_table_get_usage(s->base_usage_table,cphvb_base_array(array)) == NULL) {
            logcustom(cloglevel,1,"NAC adding %ld to base: %p",expr_name,cphvb_base_array(array));
            jita_base_usage_table_add_usage(s->base_usage_table,cphvb_base_array(array),expr_name);
        }
    }        
    
    return entr;            
}

void jita_handle_userfunc_instruction(jit_analyse_state* s,cphvb_instruction* instr, cphvb_intp instr_num) {
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



    vector<cphvb_array*>* outputs = new vector<cphvb_array*>();
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
        if (cphvb_base_array(userfunc->operand[i])->data != NULL) {
            logcustom(cloglevel,2,"JHUI O Adding %ld to base_usage %p\n",name,cphvb_base_array(userfunc->operand[i]));
            jita_base_usage_table_add_usage(s->base_usage_table,cphvb_base_array(userfunc->operand[i]),name);
        }
    }  

    
    
    
    // output
    //~ vector<cphvb_array*>* outputs = new vector<cphvb_array*>();
    //~ for(int i=0;i<userfunc->nout ;i++) {
        //~ entr = jita_nametable_lookup_a(s, userfunc->operand[i]);
        //~ 
        //~ if (entr == NULL) {
            //~ jit_name_entry* entr = new jit_name_entry();
            //~ entr->arrayp = userfunc->operand[i];
            //~ entr->tdto = new set<cphvb_intp>();
            //~ entr->tdon = new set<cphvb_intp>();
            //~ entr->instr = instr;
            //~ entr->instr_num = instr_num;
            //~ entr->operand_num = i;
//~ 
            //~ logcustom(cloglevel,1,"OUT ++ %d %p\n",i,userfunc->operand[i]);                                                
            //~ 
        //~ }
        //~ logcustom(cloglevel,1,"OUT in %ld ,on%ld \n",entr->instr_num,entr->operand_num);
        //~ logcustom(cloglevel,1,"OUT inserting into outputs %p at %d of %d\n", userfunc->operand[i],i,outputs->size());
        //~ outputs->push_back(userfunc->operand[i]);
        //~ 
    //~ }
        
    //~ cphvb_intp res = jita_nametable_insert_userfunc(s,outputs,in_exprs,instr);
    
    //jit_pprint_nametable(s->nametable);    
}

/**
 * with base_array_usage_list / dependency_table
 **/ 
void jita_handle_free_instruction(jit_analyse_state* s,cphvb_instruction* instr, cphvb_intp instr_num) {
    bool cloglevel[] = {0};
    // lookup instruction
    //cphvb_array* base = cphvb_base_array(instr->operand[0]);
    jit_name_entry* e = jita_nametable_lookup_a(s,instr->operand[0]);
    if (e == NULL) {
        // trying to free a un-registered instruction
        return;
    }
    //printf("%p %p %ld %p\n",s,instr,instr_num,e);
    
    // set last nametable entry as the point in "analyse time" where the free happened.
    // For temp array this will then be the same as the array name. 
    e->freed_at = s->nametable->size();
    logcustom(cloglevel,0,"freed registered: %ld: %ld\n",e->expr->name, s->nametable->size());
}

/**
 * with base_array_usage_list / dependency_table
 **/ 
void jita_handle_discard_instruction(jit_analyse_state* s,cphvb_instruction* instr, cphvb_intp instr_num) {
    bool cloglevel[] = {0};
    // lookup instruction
    //cphvb_array* base = cphvb_base_array(instr->operand[0]);
    jit_name_entry* e = jita_nametable_lookup_a(s,instr->operand[0]);
    if (e == NULL) {
        // trying to discard a un-registered instruction
        return;
    }
    //printf("%p %p %ld %p\n",s,instr->operand[0],instr_num,e);
    
    // set last nametable entry as the point in "analyse time" where the free happened.
    // For temp array this will then be the same as the array name. 
    e->discarded_at = s->nametable->size();
    logcustom(cloglevel,0,"discarded registered: %ld: %ld\n",e->expr->name, s->nametable->size());
}



/**
 * with base_array_usage_list / dependency_table
 **/ 
//cphvb_intp jita_handle_arithmetic_instruction2(jit_name_table* nametable, jit_ssa_map* ssamap, jit_base_dependency_table* basedep_table, cphvb_instruction* instr) {
cphvb_intp jita_handle_arithmetic_instruction2(jit_analyse_state* s, cphvb_instruction* instr, cphvb_intp instr_num) {
    bool cloglevel[] = {0,0,0};
    logcustom(cloglevel,0,"s jita_handle_arithmetic_instruction()\n");    
    logDebug("opcode: %s\n", cphvb_opcode_text(instr->opcode));
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
    cphvb_intp name_first = -1;
    cphvb_intp name_second = -1;    
    
    //jit_name_entry* first_entry = jita_lookup_name(nametable,ssamap, instr->operand[1],-1);        
     
    jit_name_entry* first_entry = NULL;
    if (!cphvb_is_constant( instr->operand[1])) {
        
        first_entry = jita_nametable_lookup_a(s, instr->operand[1]);    
    }
    
    //printf("-- First %p\n",first_entry);
    if (first_entry == NULL) {        
        first = new jit_expr();            
        operand_to_exp(instr,1,first);
                           
        logDebug("first created\n");
        
        if (!cphvb_is_constant(instr->operand[1])) {
                    
            // this step can be reached by newly created arrays.
            name_first = jita_insert_name(s->nametable,s->ssamap,instr->operand[1],first);
            e = jita_nametable_lookup(s->nametable,name_first);
            e->instr = instr;
            e->instr_num = instr_num;
            e->operand_num = 1;           
            e->tdto = new set<cphvb_intp>();
            e->tdon = new set<cphvb_intp>();
            e->is_userfunction = false;      
            if (instr->operand[1]->base != NULL || instr->operand[1]->data != NULL ) {                
                // add only usage if the basearray had not been added before!                                  
                if (jita_base_usage_table_get_usage(s->base_usage_table,cphvb_base_array(instr->operand[1])) == NULL) {
                    logcustom(cloglevel,2,"JHAI L Adding %ld to base_usage %p\n",name_first,cphvb_base_array(instr->operand[1]));
                    jita_base_usage_table_add_usage(s->base_usage_table,cphvb_base_array(instr->operand[1]),name_first);
                }
            } 
        }                            
    } else {
        first = first_entry->expr;                
        logDebug("first looked up.\n");
    }
    
             
    instr_tag = un_op;
    expr_depth = first->depth;                        
    
    // binary
    jit_name_entry* second_entry = NULL;    
    if (cphvb_operands(instr->opcode) == 3) {                    
        //second_entry = jita_lookup_name(nametable,ssamap, instr->operand[2],-1);
        //cphvb_pprint_instr(instr);
        if (instr->operand[2] != NULL)
            second_entry = jita_nametable_lookup_a(s, instr->operand[2]);   

        //printf("-- Second %p\n",second_entry);
        if (second_entry == NULL) {
            second = new jit_expr();
            operand_to_exp(instr,2,second);
            logDebug("second created\n");
            if(!cphvb_is_constant(instr->operand[2])) {  
                // this step is reached only once for each array! if it exists, it is caught in jita_lookup_name
                name_second = jita_insert_name(s->nametable,s->ssamap,instr->operand[2],second);
                e = jita_nametable_lookup(s->nametable,name_second);
                e->instr = instr;
                e->instr_num = instr_num;
                e->operand_num = 2;
                e->tdto = new set<cphvb_intp>();
                e->tdon = new set<cphvb_intp>();
                e->is_userfunction = false;                  
                if (instr->operand[2]->base != NULL || instr->operand[2]->data != NULL ) {
                    if (jita_base_usage_table_get_usage(s->base_usage_table,cphvb_base_array(instr->operand[2])) == NULL) {
                        logcustom(cloglevel,2,"JHAI R Adding %ld to base_usage %p\n",name_second,cphvb_base_array(instr->operand[2]));
                        jita_base_usage_table_add_usage(s->base_usage_table,cphvb_base_array(instr->operand[2]),name_second);
                    }
                    
                }              
            }          
        } else {
            second = second_entry->expr;
            logDebug("second looked up.\n");            
        }                    
        instr_tag = bin_op;
        expr_depth = std::max(first->depth,second->depth);
    }
    
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
    
    e= jita_nametable_lookup(s->nametable,name);
    e->instr = instr;
    e->instr_num = instr_num;
    e->operand_num = 0;
    e->tdto = new set<cphvb_intp>();
    e->tdon = new set<cphvb_intp>();
    e->is_userfunction = false;
    e->freed_at = -1;
    e->discarded_at = -1;
    logDebug("expr inserted as %ld:\n",name);     

    // add operand0 assignment to the base_array_usages_list.
    if (cphvb_base_array(instr->operand[0])->data != NULL) {
        logcustom(cloglevel,2,"JHAI O Adding %ld to base_usage %p\n",name,cphvb_base_array(instr->operand[0]));
        jita_base_usage_table_add_usage(s->base_usage_table,cphvb_base_array(instr->operand[0]),name);
    }    
    
    logDebug("e jita_handle_arithmetic_instruction()\n");        
    
    return 1;
}

//~ cphvb_intp jita_handle_arithmetic_instruction(jit_name_table* nametable, jit_ssa_map* ssamap, cphvb_instruction* instr) {
    //~ logInfo("s jita_handle_arithmetic_instruction()\n");
    //~ logDebug("opcode: %s\n", cphvb_opcode_text(instr->opcode));
    //~ // get the first and second operands. Look them up, to see if they 
    //~ // are already registered. If not register them, else reference the 
    //~ // registered one and add "used_at".    
    //~ 
    //~ jit_expr* first = NULL;        
    //~ jit_expr* second = NULL;    
    //~ jit_expr* expr = new jit_expr();
    //~ 
    //~ jit_expr_tag instr_tag;
    //~ cphvb_intp expr_depth = 1;
            //~ 
    //~ cphvb_intp name_first = -1;
    //~ cphvb_intp name_second = -1;
         //~ 
    //~ jit_name_entry* first_entry = jita_lookup_name(nametable,ssamap, instr->operand[1],-1);    
    //~ if (first_entry == NULL) {        
        //~ first = new jit_expr();            
        //~ operand_to_exp(instr,1,first);                      
        //~ logDebug("first created\n");     
        //~ if (!cphvb_is_constant(instr->operand[1])) {
            //~ name_first = jita_insert_name(nametable,ssamap,instr->operand[1],first);
        //~ }
                        //~ 
    //~ } else {
        //~ first = first_entry->expr;
        //~ logDebug("first looked up.\n");
    //~ }            
    //~ instr_tag = un_op;
    //~ expr_depth = first->depth;                        
    //~ 
    //~ // binary
    //~ jit_name_entry* second_entry = NULL;    
    //~ if (cphvb_operands(instr->opcode) == 3) {        
        //~ second_entry = jita_lookup_name(nametable,ssamap, instr->operand[2],-1);
        //~ if (second_entry == NULL) {                
            //~ second = new jit_expr();            
            //~ operand_to_exp(instr,2,second);             
            //~ logDebug("second created\n");
            //~ if(!cphvb_is_constant(instr->operand[2])) {                  
                //~ name_second = jita_insert_name(nametable,ssamap,instr->operand[2],second);
            //~ }            
        //~ } else {
            //~ second = second_entry->expr;
            //~ logDebug("second looked up.\n");
            //~ 
        //~ }                    
        //~ instr_tag = bin_op;
        //~ expr_depth = std::max(first->depth,second->depth);
    //~ }
    //~ 
    //~ // create expr. 
    //~ expr->tag = instr_tag;
    //~ expr->op.expression.opcode = instr->opcode;          
    //~ expr->op.expression.left = first;
    //~ expr->op.expression.right = second;
    //~ expr->depth = expr_depth + 1;    
    //~ logDebug("expr created: %p\n",expr);     
    //~ 
    //~ // insert the new expression    
    //~ cphvb_intp name = jita_insert_name(nametable, ssamap, instr->operand[0], expr);    
    //~ logDebug("expr inserted as %ld:\n",name);     
    //~ // update first and second expression entries with used_at, 
    //~ // if not constant.    
    //~ 
    //~ 
    //~ // update used_at entries
    //~ if(!cphvb_is_constant(instr->operand[1])) {
        //~ if (first_entry == NULL) {
            //~ first_entry = jita_nametable_lookup(nametable,name_first);
        //~ }
        //~ logDebug("first_entry->used_at->push_back(%ld);\n",name); 
        //~ logDebug("%p \n",first_entry); 
        //~ 
        //~ first_entry->used_at->push_back(name);
    //~ }
    //~ 
    //~ if(second != NULL && !cphvb_is_constant(instr->operand[2])) {
        //~ if (second_entry == NULL) {
            //~ second_entry = jita_nametable_lookup(nametable,name_second);
        //~ }        
        //~ logDebug("second_entry->used_at->push_back(%ld);\n",name); 
        //~ second_entry->used_at->push_back(name);                
    //~ }
    //~ logDebug("e jita_handle_arithmetic_instruction()\n");
    //~ return 1;
//~ }


//~ cphvb_intp jita_nametable_insert_userfunc(jit_analyse_state* s,
                        //~ vector<cphvb_array*>* outputs,
                        //~ vector<jit_expr*>* inputs,
                        //~ cphvb_instruction* instr);
//~ 
    //~ bool cloglevel[] = {1,1};
    //~ logcustom(cloglevel,0,"JNIU jita_nametable_insert_userfunc()\n");    
    //~ // output arrays must be registered in the ssa. They will point to the same nametable entry.
    //~ // 
    //~ 
    //~ jit_name_entry* ne = new jit_name_entry();
    //~ ne->is_userfunction = true;
    //~ 
    //~ ne->userfunc_in = inputs;
    //~ ne->userfunc_out = outputs;
    //~ ne->userfunc_out_versions = new vector<cphvb_intp>();
        //~ 
    //~ ne->instr = instr;
    //~ 
    //~ ne->tdon = new set<cphvb_intp>();
    //~ ne->tdto = new set<cphvb_intp>();
//~ 
    //~ ne->expr = new jit_expr();
    //~ ne->expr->tag = userfunc;
    //~ ne->expr->depth = 0;
    //~ ne->expr->op.userfunc = instr->userfunc;
    //~ ne->expr->userfunction_inputs = inputs
    //~ 
    //~ ne->expr->name = _jita_nametable_insert(s->nametable,ne);
    //~ // foreach output array, insert the name reference in the ssamap.
    //~ for(int i=0;i<outputs->size();i++) {
         //~ ne->userfunc_out_versions->push_back(jita_ssamap_version_lookup(s->ssamap,outputs->at(i),-1)+1);
        //~ _jita_ssamap_insert(s->ssamap,outputs->at(i),ne->expr->name);        
    //~ }    
    //~ return 0;
//~ }



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


// jit_dependency_graph() methods
// ===================================

jit_dependency_graph_node* jita_depgraph_get_node(jit_dependency_graph* graph, cphvb_intp name) {
    jit_dependency_graph::iterator it = graph->find(name);
    if (it==graph->end()) {
        return NULL;
    }    
    return it->second;
}

/**
 * @return 0 on success. 1 on failure.
 */
jit_dependency_graph_node* jita_depgraph_add_node(jit_dependency_graph* graph, jit_name_entry* name_table_entry) {    
    jit_dependency_graph_node* n = (jit_dependency_graph_node*)malloc(sizeof(jit_dependency_graph_node));
    n->name_table_entry = name_table_entry;    
    n->dep_ons = new vector<jit_name_entry*>();
    n->dep_tos = new vector<jit_name_entry*>();
    bool sta = graph->insert(pair<cphvb_intp,jit_dependency_graph_node*>(name_table_entry->expr->name,n)).second;
    logDebug("insert status: %d,%d,%p,%p\n",sta,name_table_entry->expr->name,n,graph);
    if(sta==false) {
        logDebug("element already exists\n");
        
        return jita_depgraph_get_node(graph,name_table_entry->expr->name);        
    }
    
    return n;    
}

vector<jit_name_entry*>* jita_depgraph_get_dto(jit_dependency_graph* graph, cphvb_intp name) {
     jit_dependency_graph_node* n = jita_depgraph_get_node(graph, name);
     if (n != NULL) {
         return n->dep_tos;
     }
     return NULL;
}

vector<jit_name_entry*>* jita_depgraph_get_don(jit_dependency_graph* graph, cphvb_intp name) {
     jit_dependency_graph_node* n = jita_depgraph_get_node(graph, name);
     if (n != NULL) {
         return n->dep_ons;
     }
     return NULL;
}

cphvb_intp jita_depgraph_node_add_don(jit_dependency_graph_node* n,jit_name_entry* onnode) {
    n->dep_ons->push_back(onnode);
    return 0;
}

cphvb_intp jita_depgraph_node_add_dto(jit_dependency_graph_node* n,jit_name_entry* tonode) {
    n->dep_tos->push_back(tonode);
    return 0;
}

// jit_base_dependency_table() methods
// ===================================

vector<cphvb_intp>* jita_base_usage_table_get_usage(jit_base_dependency_table* dep_table, cphvb_array* base_array) {
    logDebug("jita_base_usage_table_get_usage(%p,%p)\n",dep_table,base_array);
    jit_base_dependency_table::iterator it = dep_table->find(base_array);    
    //jit_pprint_base_dependency_table(dep_table);
    
    it = dep_table->find(base_array);
    if (it==dep_table->end()) { 
        logDebug("Deptable does not contain basearray\n");
        return NULL;         
    }
    return it->second;
}


cphvb_intp jita_base_usage_table_add_usage(jit_base_dependency_table* dep_table, cphvb_array* base_array, cphvb_intp name) {
    bool cloglevel[] = {0};
    jit_base_dependency_table::iterator it = dep_table->find(base_array);
    it = dep_table->find(base_array);
    if (it==dep_table->end()) {
        (*dep_table)[base_array] = new vector<cphvb_intp>(0);                
    }
    logcustom(cloglevel,0,"JBUTAU adding %ld to %p\n",name,base_array);
    (*dep_table)[base_array]->push_back(name);
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
    bool equal = true;
    int i;
    
    
    // do compares on dimension, start, shape and stride
    if (a->ndim != b->ndim) {
        return 10;
    }
     
    if (a->start != b->start) {
        return 11;                
    }
    
    for(i=0;i < a->ndim;i++) {
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
 * Lookup an array deeply, by searching nametable for 
 **/
cphvb_intp jita_lookup_name_deep(jit_base_dependency_table* dep_table,jit_name_table* nametable, jit_ssa_map* ssamap, cphvb_array* array) {    
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"JLND jita_lookup_name_deep(%p)\n",array);
    
    cphvb_array* base = cphvb_base_array(array);
    // validate input   
    if(base->data == NULL) {
        logcustom(cloglevel,1,"JLND base->data == NULL\n");    
        return -1;
    }
    
    // get the base_usagelist for the array's base.    
    std::vector<cphvb_intp>* base_deps = jita_base_usage_table_get_usage(dep_table,base);
    if(base_deps == NULL) {
        logcustom(cloglevel,1,"JLND base_deps == NULL\n");    
        return -1;
    }
    
    //printf("..... %p \n", base_deps);
    //jit_pprint_base_dependency_list(base_deps);
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

          
    //~ for(it=base_deps->rbegin(); it!=base_deps->rend(); it++) {
        //~ logcustom(cloglevel,1,"JLND cc %p %d\n",nametable,*it);
        //~ entr = jita_nametable_lookup(nametable,*it);
        //~ if (entr != NULL) {
            //~ int res = array_view_deep_compare(entr->arrayp,array);  
            //~ //printf("compare %p == %p = %d \n",entr->arrayp,array,res);                      
            //~ if (res == 0) {                
                //~ return *it;
            //~ }                    
        //~ }
    //~ }
    return -1;
}

cphvb_intp jita_lookup_name_deep_s(jit_analyse_state* s, cphvb_array* array) {
    return jita_lookup_name_deep(s->base_usage_table,s->nametable, s->ssamap,array);
}

jit_name_entry* jita_nametable_lookup_a(jit_analyse_state* s, cphvb_array* array) {    
    logDebug("jita_nametable_lookup_a(%p)\n",array);
    
    cphvb_intp name = jita_lookup_name_a(s,array);
    
    //printf("jita_nametable_lookup_a >> name : %ld for %p\n",name,array);
    //jit_pprint_nametable(s->nametable);
    jit_name_entry* e = jita_nametable_lookup(s->nametable,name);    
    return e;
}

cphvb_intp jita_lookup_name_a(jit_analyse_state* s, cphvb_array* array) {
    bool cloglevel[] = {0};
    logcustom(cloglevel,0,"jita_lookup_name_a()\n");    
    //printf("%p b> \n",array);
    cphvb_intp name = jita_ssamap_version_lookup(s->ssamap,array,-1);    
    logcustom(cloglevel,0,"JLNA %ld - base array%p\n",name, array->base);
    if ((name == -1) && (cphvb_base_array(array)->data != NULL)) {                            
        name = jita_lookup_name_deep_s(s,array);
        //logDebug("deep %ld\n",name);
        
    }    
    return name;
}

// ===================================








// Test methods
void _print_used_at(std::vector<cphvb_intp>* vec) {
    printf(" used_at [");
    int size = (int)vec->size();
    for(int i = 0; i<size; i++) {
        printf("%ld, ",vec->at(i));
    }
    printf("]\n\n");
}

jit_expr* _test_make_expr_const(cphvb_intp id, cphvb_constant* constant) {    
    jit_expr* expr = new jit_expr();
    expr->tag = const_val;
    expr->op.constant = constant;
    expr->depth = 0;
    expr->id = id;    
    return expr;    
}
jit_expr* _test_make_expr_array(cphvb_intp id, cphvb_array* array) {    
    jit_expr* expr = new jit_expr();
    expr->tag = array_val;
    expr->op.array = array;
    expr->depth = 0;
    expr->id = id;
    //printf("%p\n",array);
    return expr;    
}

bool _test_jita_insert_name_arr() {
    int loglevel = 1;    
    jit_ssa_map* ssamap = new jit_ssa_map();
    jit_name_table* nametable = new jit_name_table();
    
    cphvb_array* inpA = new cphvb_array();    
    jit_expr* exprA = _test_make_expr_array(5,inpA);        
    
    if (loglevel > 0) printf("expr %p\narray %p\n",exprA,inpA);        
    if (loglevel > 0) printf("Tesing jita_insert_name()\n");
    jita_insert_name(nametable,ssamap,inpA,exprA);    
    
    
    
    if (loglevel > 0) printf("* validate ssamap entry\n");
    if (loglevel > 0) printf("arr* = %p\n",inpA);
    
    
    jit_ssa_map::iterator it = ssamap->find(inpA);
    if (it == ssamap->end()) {
        return false;
    }
    std::vector<cphvb_intp>* version_vector = it->second;
    if (loglevel > 0) printf("version vector* = %p\n",version_vector);
    if (loglevel > 0) printf("version vector* size = %ld\n",version_vector->size());
    if (loglevel > 0) printf("version vector[0]* = %ld\n",version_vector->at(0));        
    cphvb_intp name = version_vector->at(0);
        
    if (loglevel > 0) printf("* validate nametable entry entry\n");
    if (loglevel > 0) printf("name_table lookup = %ld\n",name);
    
    jit_name_entry* entry = nametable->at(name);
    if (loglevel > 0) printf("name_table_entry* = %p\n",entry);
    if (loglevel > 0) printf("name_table_entry* ->version = %ld\n",entry->version);
    
    if (loglevel > 0) printf("name_table_entry* ->arrayp = %p\n",entry->arrayp);
    if (loglevel > 0) printf("name_table_entry* ->expr = %p\n",entry->expr);
    if (loglevel > 0) printf("name_table_entry* ->used_at = %p\n",entry->used_at);
    //printf("name_table_entry* ->used_at->size = %d\n",entry->used_at->size());
                   
    return false;
}

bool _test_jita_insert_name_complex() {
    int loglevel = 4;
    int INFO = 4;
    
    jit_ssa_map* ssamap = new jit_ssa_map();
    jit_name_table* nametable = new jit_name_table();
    
    // create array
    // output, inputA, inputB    
    
    cphvb_array* inpA = new cphvb_array();
        
    jit_expr* exprA = _test_make_expr_array(9,inpA);            
    cphvb_intp nameA = jita_insert_name(nametable,ssamap,inpA,exprA);        
    if (loglevel >= INFO) printf("A (left):\narr* = %p\nexpr* = %p\nname = %ld\n",inpA,exprA,nameA);    
    
    cphvb_array* inpB = new cphvb_array();
    
    jit_expr* exprB = _test_make_expr_array(2,inpB);
    cphvb_intp nameB = jita_insert_name(nametable,ssamap,inpB,exprB);    
    if (loglevel >= INFO) printf("B (right):\narr* = %p\nexpr* = %p\nname = %ld\n",inpB,exprB,nameB);
                
    cphvb_array* out = new cphvb_array();    
    
    
    jit_expr* exprComb = new jit_expr();
    exprComb->tag = bin_op;    
    exprComb->depth = 1;
    exprComb->id = 1;
    exprComb->op.expression.opcode = CPHVB_ADD;
    exprComb->op.expression.left = exprA;
    exprComb->op.expression.right = exprB;
        
    cphvb_intp nameOut = jita_insert_name(nametable,ssamap,out,exprComb);  
    if (loglevel >= INFO) printf("Out :\narr* %p\nexpr* = %p\nname = %ld\n",out,exprComb,nameOut);
    
    
    printf("nametable.size()=%ld\n",nametable->size() );
    printf("nametable 0 %p\n",jita_nametable_lookup(nametable,0));
    printf("nametable 1 %p\n",jita_nametable_lookup(nametable,1));
    printf("nametable 2 %p\n",jita_nametable_lookup(nametable,2));
        
    _jita_update_used_at(nametable,ssamap,nameA,nameOut);
    _jita_update_used_at(nametable,ssamap,nameB,nameOut);
    
    printf("%p -> ",inpA);
    _print_used_at(jita_nametable_lookup(nametable,nameA)->used_at);    
    printf("%p -> ",inpB);
    _print_used_at(jita_nametable_lookup(nametable,nameB)->used_at);
   
    
    return true;
}


void jita_run_tests() {
    int loglevel = 1;    
    if (loglevel > 0) printf("jita_run_test() - starting \n");
    
    
    if (loglevel > 0) printf("\n** create array\n");    
    cphvb_array* arr = new cphvb_array();    
    
    printf("arr* = %p\n", arr);
    
    
    if (loglevel > 0) printf("\n-- make expr()\n");    
    jit_expr* expr = _test_make_expr_array(5,arr);        
    printf("expr* = %p\n",expr);
    printf("expr* tag = %d\n",expr->tag);
    printf("expr* id = %ld\n",expr->id);
    printf("expr* depth = %ld\n",expr->depth);
    printf("expr* op.array = %p\n",expr->op.array);
    
    if (loglevel > 0) printf("-- make expr() DONE\n");    
    
    if (loglevel > 0) printf("\n-- jita_test_insert_name_arr() \n");        
    _test_jita_insert_name_arr();    
    if (loglevel > 0) printf("-- jita_test_insert_name_arr() DONE\n");        
    
    if (loglevel > 0) printf("\n-- jita_test_insert_name_complex() \n");        
    _test_jita_insert_name_complex();
    if (loglevel > 0) printf("-- jita_test_insert_name_complex() DONE\n");        
    
    if (loglevel > 0) printf("jita_run_test() - done \n");
}

