// jit_analyser.cpp

#include "jit_analyser.h"
#include "jit_ast.h"
#include <vector>

using namespace std;

/**
 * the opcode of is SYNC, DISCARD, FREE, 
 **/
bool jita_is_controll(cphvb_instruction* instr) {    
    return (instr->opcode == CPHVB_SYNC || instr->opcode == CPHVB_DISCARD || instr->opcode == CPHVB_DESTROY || instr->opcode == CPHVB_NONE);
}

jit_name_entry* jita_nametable_lookup(jit_name_table* nametable, cphvb_intp index) {        
    if( (int)nametable->size() > index-1) {    
        return nametable->at(index);
    }         
    // handle error (out of bound exception)    
    return NULL;
}


/**
 * @return a version-vector with mappings to the nametable.
 **/
vector<cphvb_intp>* jita_ssamap_lookup(jit_ssa_map* ssamap,cphvb_array* array) {
    jit_ssa_map::iterator it = ssamap->find(array);
    if (it != ssamap->end()) {
        return it->second;
    }
    return NULL;
}

/**
 * ssamap_lookup()
 * @version the array version to lookup. If version == -1, return the highest version.
 * @return name of the array in the nametable. -1 if no entry in the ssamap. 
 **/
cphvb_intp jita_ssamap_version_lookup(jit_ssa_map* ssamap, cphvb_array* array, cphvb_intp version) {
    jit_ssa_map::iterator it = ssamap->find(array);
    if (it == ssamap->end()) {
        // no entry
        return -1;
    } 
    
    std::vector<cphvb_intp>* versions = it->second;
    
    // return the latest version name
    if(version == -1) {
        return versions->back();
    }
    
    // return the specified version name
    if (versions->size() > version-1) {
        return versions->at(version);
    }
}

/**
 * lookup_name()
 * 
 * look up the jit_name_entry in the nametable for the array with the specified version.
 * @version The array version to get. If version == -1, return the highest version.
 */
jit_name_entry* jita_lookup_name(jit_name_table* nametable, jit_ssa_map* ssamap, cphvb_array* array, cphvb_intp version) {    
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
    int loglevel = 4;
    int DEBUG = 4, INFO = 3;
    if (loglevel >= INFO) printf("_jita_ssamap_insert\n");
    
    cphvb_intp next_version  = jita_ssamap_version_lookup(ssamap,array,-1) + 1;
    if (loglevel >= INFO) printf("array* %p next version = %d\n",array,next_version);
    
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
    int loglevel = 4;
    int DEBUG = 4, INFO = 3;
        if (loglevel >= INFO) printf("_jita_update_used_at\n");
        if (loglevel >= DEBUG) printf("input: used_at_name = %d \n name: %d\n",used_at_name,name);    
    
    // validate existense of used_at_array
    jit_name_entry* used_at_entry = jita_nametable_lookup(nametable,used_at_name);        
    if (used_at_entry == NULL) {
        if (loglevel >= DEBUG) printf(" used_at_name %d , NOT FOUND\n",used_at_name);         
        return -1;
    }    
    
    // add name, to the used_at_vector
    used_at_entry->used_at->push_back(name);
        if (loglevel >= DEBUG) printf("entry = %p\n",used_at_entry);
                   
    return 1;
}

cphvb_intp _jita_update_used_at(jit_name_table* nametable,jit_ssa_map* ssamap,jit_name_entry* used_at_entry, cphvb_intp name) {
    int loglevel = 4;
    int DEBUG = 4, INFO = 3;
        if (loglevel >= INFO) printf("_jita_update_used_at\n");
        if (loglevel >= DEBUG) printf("input: used_at_name = %p \n name: %d\n",used_at_entry,name);    
    
    // validate existense of used_at_array
    //jit_name_entry* used_at_entry = jita_nametable_lookup(nametable,used_at_name);        
    if (used_at_entry == NULL) {
        if (loglevel >= DEBUG) printf(" used_at_name %p , NOT FOUND\n",used_at_entry);         
        return -1;
    }    
    
    // add name, to the used_at_vector
    used_at_entry->used_at->push_back(name);
        if (loglevel >= DEBUG) printf("entry = %p\n",used_at_entry);
                   
    return 1;
}

//~ cphvb_intp _jita_update_used_at(jit_name_table* nametable,jit_ssa_map* ssamap,cphvb_array* used_at_array, cphvb_intp name) {
    //~ int loglevel = 4;
    //~ int DEBUG = 4, INFO = 3;
        //~ if (loglevel >= INFO) printf("_jita_update_used_at\n");
        //~ if (loglevel >= DEBUG) printf("input: used_at = %p \n name: %d\n",used_at_array,name);    
    //~ 
    //~ // validate existense of used_at_array
    //~ cphvb_intp used_at_name = jita_ssamap_version_lookup(ssamap,used_at_array,-1);    
        //~ if (loglevel >= DEBUG) printf(" used_at_name = %d\n",used_at_name);            
    //~ if(used_at_name == -1) {
        //~ // used_at_array not found in ssamap!
        //~ return -1;
    //~ }
    //~ 
    //~ // lookup used_at_arrat's name_entry's used_at vector
    //~ jit_name_entry* used_at_entry = jita_nametable_lookup(nametable,used_at_name);    
        //~ if (loglevel >= DEBUG) printf("entry = %p\n",used_at_entry);
    //~ // add name, to the used_at_vector
    //~ used_at_entry->used_at->push_back(name);
                //~ 
    //~ return 1;
//~ }

/**
 * When an discard is call on a array, it is no longer needed. This can be because it
 * is a temp array, it falls out of scope either from a return of the function or because
 * it was used in a If-,for- or similar statement.
 * 
 **/
cphvb_intp jita_handle_discard(jit_name_table* nametable, jit_ssa_map* ssamap, jit_execute_list* exelist, cphvb_array* array) {
    int loglevel = 4;
    int DEBUG = 4, INFO = 3;    
    if (loglevel >= INFO) printf("jita_handle_discard()\n");
    
    bool triggered_by_finishing_block = false;
    
    // With SSA, all variables are only assigned once. If a variable is 
    // not used by another when discarded, it it not possible to know if
    // it is a return value or just not used at all. It must be computed.
         
    cphvb_intp name = jita_ssamap_version_lookup(ssamap,array,-1);
    jit_name_entry* e = jita_nametable_lookup(nametable,name);
    if( ((int)e->used_at->size()) > 0) {
        e->status = JIT_DISCARDRD;
        if (loglevel >= DEBUG) printf("Marked as DISCARDED: A:%p V:%d  N:%d, used_at-size =%d \n",e->arrayp, e->version ,name,(int)e->used_at->size() );
    } else {
        e->status = JIT_EXECUTE;        
        exelist->push_back(name);
        if (loglevel >= DEBUG) printf("Added to execute: %d\n",name);
    }
        
    
    // if discard happens on a array which is not used anywhere it must
    // be computed.
    
    
    // if discard happens on a array which is used in other expressions
    // they are substituted in. (they already are) and it is marked as 
    // "discarded" in jit_name_entry->status;

    
}

cphvb_intp jita_handle_controll_instruction(jit_name_table* nametable, jit_ssa_map* ssamap, jit_execute_list* exelist, cphvb_instruction* instr) {
    int loglevel = 4;
    int DEBUG = 4, INFO = 3;
    if (loglevel >= INFO) printf("jita_handle_controll_instruction()\n");
    switch(instr->opcode) {
        case CPHVB_DISCARD:              
            jita_handle_discard(nametable, ssamap, exelist , instr->operand[0]);                        
            break;        
            
        case CPHVB_SYNC:
            break;
            
        case CPHVB_NONE:
            break;
    }

}

cphvb_intp jita_handle_arithmetic_instruction(jit_name_table* nametable, jit_ssa_map* ssamap, cphvb_instruction* instr) {
    int loglevel = 4;
    int DEBUG = 4, INFO = 3;
    if (loglevel >= INFO) printf("jita_handle_arithmetic_instruction()\n");
    if (loglevel >= DEBUG) printf("opcode: %s\n", cphvb_opcode_text(instr->opcode));
    // get the first and second operands. Look them up, to see if they 
    // are already registered. If not register them, else reference the 
    // registered one and add "used_at".    
    
    jit_expr* first;        
    jit_expr* second;    
    jit_expr* expr = new jit_expr();
    
    jit_expr_tag instr_tag;
    cphvb_intp expr_depth = 1;
            
    cphvb_intp name_first;
    cphvb_intp name_second;
            
         
    jit_name_entry* first_entry = jita_lookup_name(nametable,ssamap, instr->operand[1],-1);    
    if (first_entry == NULL) {        
        first = new jit_expr();            
        operand_to_exp(instr,1,first);                      
        if (loglevel >= DEBUG) printf("first created\n");     
        if (!cphvb_is_constant(instr->operand[1])) {        
            name_first = jita_insert_name(nametable,ssamap,instr->operand[1],first);
        }        
                        
    } else {
        first = first_entry->expr;
        if (loglevel >= DEBUG) printf("first looked up.\n");
    }            
    instr_tag = un_op;
    expr_depth = first->depth + 1;                        
    
    // binary
    jit_name_entry* second_entry;
    if (cphvb_operands(instr->opcode) == 3) {        
        second_entry = jita_lookup_name(nametable,ssamap, instr->operand[2],-1);
        if (second_entry == NULL) {                
            second = new jit_expr();            
            operand_to_exp(instr,2,second);             
            if (loglevel >= DEBUG) printf("second created\n");
            if(!cphvb_is_constant(instr->operand[2])) {                  
                name_second = jita_insert_name(nametable,ssamap,instr->operand[2],second);
            }
            
        } else {
            second = second_entry->expr;
            if (loglevel >= DEBUG) printf("second looked up.\n");
            
        }                    
        instr_tag = bin_op;
        expr_depth = std::max(first->depth,second->depth) + 1;
    }
    
    // create expr. 
    expr->tag = instr_tag;
    expr->op.expression.opcode = instr->opcode;          
    expr->op.expression.left = first;
    expr->op.expression.left = second;
    expr->depth = expr_depth;    
    if (loglevel >= DEBUG) printf("expr created: %p\n",expr);     
    
    // insert the new expression    
    cphvb_intp name = jita_insert_name(nametable, ssamap, instr->operand[0], expr);    
    if (loglevel >= DEBUG) printf("expr inserted as %d:\n",name);     
    // update first and second expression entries with used_at, 
    // if not constant.    
    
    if(!cphvb_is_constant(instr->operand[1])) {
        if (first_entry == NULL) {
            first_entry = jita_nametable_lookup(nametable,name_first);
        }
        if (loglevel >= DEBUG) printf("first_entry->used_at->push_back(%d);\n",name); 
        if (loglevel >= DEBUG) printf("%p \n",first_entry); 
        first_entry->used_at->push_back(name);
        
        //_jita_update_used_at(nametable,ssamap,first_entry,name);
    }
    
    if(second != NULL && !cphvb_is_constant(instr->operand[2])) {
        if (second_entry == NULL) {
            second_entry = jita_nametable_lookup(nametable,name_second);
        }
        
        if (loglevel >= DEBUG) printf("second_entry->used_at->push_back(%d);\n",name); 
        second_entry->used_at->push_back(name);
            
        //_jita_update_used_at(nametable,ssamap,second_entry,name);
    }
    
    return 1;
}

/**
 * insert a assignment with array* = expression into the nametable.
 * @return name in the nametable. -1 if error.
 */
cphvb_intp jita_insert_name(jit_name_table* nametable, jit_ssa_map* ssamap, cphvb_array* array, jit_expr* expr) {    
    int loglevel = 4;
    int DEBUG = 4, INFO = 3;
    if (loglevel >= INFO) printf("jita_insert_name()\n",array);
    if (loglevel >= DEBUG) printf(">>> arrayp* %p\n",array);
    if (loglevel >= DEBUG) printf(">>> expr*   %p\n",expr);
        
    // the ssamap, is a mapping from array* to nametable. We thus start
    // by creating the name_table entry.
    
    // get the highest version of the array
    cphvb_intp next_version = jita_ssamap_version_lookup(ssamap,array,-1)+1;
    
    // create the name entry for the name table.
    jit_name_entry* name_entry = new jit_name_entry();
    name_entry->version = next_version;    
    name_entry->expr = expr;
    name_entry->arrayp = array;    
    name_entry->used_at = new std::vector<cphvb_intp>();

    if (loglevel >= DEBUG) printf(">>> - entry %p\n",name_entry);
    
    // insert into nametable    
    cphvb_intp name = _jita_nametable_insert(nametable,name_entry);        
    if (loglevel >= DEBUG) printf(">>> - name %d\n",name);
    
    // insert into ssamap
    cphvb_intp added_version = _jita_ssamap_insert(ssamap,array,name);    
    
    return name;
}


// Test methods
void _print_used_at(std::vector<cphvb_intp>* vec) {
    printf(" used_at [");
    int size = (int)vec->size();
    for(int i = 0; i<size; i++) {
        printf("%d, ",vec->at(i));
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
    inpA->owner = 2002;    
    jit_expr* exprA = _test_make_expr_array(5,inpA);        
    
    if (loglevel > 0) printf("expr %p\narray %p\n",exprA,inpA);
        
    if (loglevel > 0) printf("Tesing jita_insert_name()\n");
    jita_insert_name(nametable,ssamap,inpA,exprA);    
    cphvb_intp name;
    
    if (loglevel > 0) printf("* validate ssamap entry\n");
    if (loglevel > 0) printf("arr* = %p\n",inpA);
    jit_ssa_map::iterator it = ssamap->find(inpA);
    if (it != ssamap->end()) {
        std::vector<cphvb_intp>* version_vector = it->second;
        if (loglevel > 0) printf("version vector* = %p\n",version_vector);
        if (loglevel > 0) printf("version vector* size = %d\n",version_vector->size());
        if (loglevel > 0) printf("version vector[0]* = %d\n",version_vector->at(0));        
        name = version_vector->at(0);
    }
    
    if (loglevel > 0) printf("* validate nametable entry entry\n");
    if (loglevel > 0) printf("name_table lookup = %d\n",name);
    
    jit_name_entry* entry = nametable->at(name);
    if (loglevel > 0) printf("name_table_entry* = %p\n",entry);
    if (loglevel > 0) printf("name_table_entry* ->version = %d\n",entry->version);
    
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
    inpA->owner = 2002;    
    jit_expr* exprA = _test_make_expr_array(9,inpA);            
    cphvb_intp nameA = jita_insert_name(nametable,ssamap,inpA,exprA);        
    if (loglevel >= INFO) printf("A (left):\narr* = %p\nexpr* = %p\nname = %d\n",inpA,exprA,nameA);    
    
    cphvb_array* inpB = new cphvb_array();
    inpB->owner = 3003;
    jit_expr* exprB = _test_make_expr_array(2,inpB);
    cphvb_intp nameB = jita_insert_name(nametable,ssamap,inpB,exprB);    
    if (loglevel >= INFO) printf("B (right):\narr* = %p\nexpr* = %p\nname = %d\n",inpB,exprB,nameB);
                
    cphvb_array* out = new cphvb_array();    
    out->owner = 1001;
    
    jit_expr* exprComb = new jit_expr();
    exprComb->tag = bin_op;    
    exprComb->depth = 1;
    exprComb->id = 1;
    exprComb->op.expression.opcode = CPHVB_ADD;
    exprComb->op.expression.left = exprA;
    exprComb->op.expression.right = exprB;
        
    cphvb_intp nameOut = jita_insert_name(nametable,ssamap,out,exprComb);  
    if (loglevel >= INFO) printf("Out :\narr* %p\nexpr* = %p\nname = %d\n",out,exprComb,nameOut);
    
    
    printf("nametable.size()=%d\n",nametable->size() );
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
    arr->owner = 1001;
    printf("arr* = %p\n", arr);
    printf("arr* owner = %d\n", arr->owner);
    
    if (loglevel > 0) printf("\n-- make expr()\n");    
    jit_expr* expr = _test_make_expr_array(5,arr);        
    printf("expr* = %p\n",expr);
    printf("expr* tag = %d\n",expr->tag);
    printf("expr* id = %d\n",expr->id);
    printf("expr* depth = %d\n",expr->depth);
    printf("expr* op.array = %p\n",expr->op.array);
    printf("expr* op.array->owner = %d\n",expr->op.array->owner);
    if (loglevel > 0) printf("-- make expr() DONE\n");    
    
    if (loglevel > 0) printf("\n-- jita_test_insert_name_arr() \n");        
    _test_jita_insert_name_arr();    
    if (loglevel > 0) printf("-- jita_test_insert_name_arr() DONE\n");        
    
    if (loglevel > 0) printf("\n-- jita_test_insert_name_complex() \n");        
    _test_jita_insert_name_complex();
    if (loglevel > 0) printf("-- jita_test_insert_name_complex() DONE\n");        
    
    if (loglevel > 0) printf("jita_run_test() - done \n");
}

