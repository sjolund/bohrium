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

//~ bool ssa_add_w_relation(map<cphvb_array*,ssavt*>* ssalist, ssa_used_at_t ssauat, cphvb_array* array, ast* expr) {
    //~ if (_JIT_LOG_LEVEL >= 4) printf("> ssa_add_w_relation()\n");
    //~ 
    //~ if (ssa_add_base(ssalist,array,expr)) {
        //~ ;
        //~ cphvb_intp version = ssa_lookup_array_version(ssalist,array);
        //~ 
        //~ // add a relation to the expressions used 
        //~ 
        //~ 
    //~ }
//~ }


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



//~ cphvb_intp add_ua(ssa_used_at_t* ssauaTable,cphvb_array* used_array,cphvb_intp used_array_version,cphvb_array* at_array, cphvb_intp at_array_version) {
//~ 
    //~ // check that the used and target arrays are present!
    //~ 
    //~ std::vector< std::pair<chpvb_array*,cphvb_intp>* >* usedAtVector;
    //~ 
    //~ ssa_used_at_t::iterator it = ssauaTable->find(at_array);
    //~ if (it == ssauaTable->end()) {
        //~ // create new std::vector< std::pair<chpvb_array*,cphvb_intp>* >    
    //~ }
    //~ 
    //~ std::pair<cphvb_array*,cphvb_intp> ua_entry = new pair(used_array,used_array_version);
    //~ 
    //~ usedAtVector->at(at_array_version)
    //~ 
    //~ if (it != ssauaTable->end()) {
        //~ 
        //~ std::vector<std::vector<>> it->second() //== vector
        //~ it->
    //~ }
    //~ 
    //~ 
//~ 
    //~ return 0;
//~ }


void ast_handle_instruction_ssa_ua(ssa_used_at_t ssauaTable, std::list<ast*>* expression_list,std::map<cphvb_array*,ssavt*>* nametable, cphvb_instruction* instr) {
    
    
    // parse controll instructions
    cphvb_intp num_operands = cphvb_operands(instr->opcode);
    if (num_operands > 1) {
        
        cphvb_array* name_array = instr->operand[0];
        ast* expr = new ast();    
        expr->tag = ast_operand_count_to_tag(cphvb_operands(instr->opcode));
        expr->op.expression.opcode = instr->opcode;
        
        expr->id = 10;
        
        ast* expr1 = NULL; // = expr->op.expression.left;
        ast* expr2 = NULL; // = expr->op.expression.right;
        
        if (!cphvb_is_constant(instr->operand[1])) {
            expr1 = ssa_lookup_latest(nametable,instr->operand[1]);
        }
        
        if (expr1 == NULL) {        
            expr1 = new ast();        
            operand_to_exp(instr,1,expr1);                
            if (!cphvb_is_constant(instr->operand[1])) {
                ssa_add(nametable,instr->operand[1],expr1);        
            }
        } 
        
        if (cphvb_operands(instr->opcode) == 3) {
            
            // binary operation
            if (!cphvb_is_constant(instr->operand[2])) {
                expr2 = ssa_lookup_latest(nametable,instr->operand[2]);                
            }
            
            if (expr2 == NULL) {            
                printf("expr2  == null\n");
                expr2 = new ast();            
                operand_to_exp(instr,2,expr2);            
                if (!cphvb_is_constant(instr->operand[2])) {
                    //printf("-*-  \n");    
                    ssa_add(nametable,instr->operand[2],expr2);
                }
            } 
        }

        expr->op.expression.left = expr1;
        expr->op.expression.right = expr2;
        
        if (_JIT_LOG_LEVEL >= 4) {
            printf("\n! %p\n",instr->operand[0]);
            printf("¤ L %p\n",expr->op.expression.left);
            printf("¤ R %p\n",expr->op.expression.right);
        }
        ssa_add(nametable,instr->operand[0],expr);
                        
        //add_ua(ssauaTable,instr->operand[2],ssa_lookup_array_version(nametable,instr->operand[2]),instr->operand[0],ssa_lookup_array_version(nametable,instr->operand[0]));
        //add_ua(ssauaTable,instr->operand[1],ssa_lookup_array_version(nametable,instr->operand[1]),instr->operand[0],ssa_lookup_array_version(nametable,instr->operand[0]));
        
         
        //printf("tag: %d",(*nametable)[instr->operand[0]]->tag);
        //printf("nt:s: %d\n",(*nametable).size());

        //print_ast_node(expr1);    
        //print_ast_node(expr2);

        //print_nametable(*nametable);   
        //printf("\n");
        //print_ssa_nametable(*nametable);
        //print_ssavt( ssa_lookup_vt(nametable,instr->operand[0]));
                    
            
        if (num_operands == 3) {
            // binary arithmic operations
            
        } else if (num_operands == 2) {
            // unary arithmic operations        
        }
        
    } else {
        // handle controll- and userfunctions
        
        switch(instr->opcode) {
            case CPHVB_NONE: // NOOP.                
                break; 
                
            case CPHVB_DISCARD:
                break; 
            
            case CPHVB_SYNC:
                break; 

            
            case CPHVB_USERFUNC: 
                break;                     
        }        
    }
    
    
}

void ast_handle_instruction_ssa(std::list<ast*>* expression_list,std::map<cphvb_array*,ssavt*>* nametable, cphvb_instruction* instr) {
    //ast_log("> ast_handle_instruction_ssa()\n",LOG_INFO);    
    
    // parse controll instructions
    cphvb_intp num_operands = cphvb_operands(instr->opcode);
    if (num_operands > 1) {
        
        cphvb_array* name_array = instr->operand[0];
        ast* expr = new ast();    
        expr->tag = ast_operand_count_to_tag(cphvb_operands(instr->opcode));
        expr->op.expression.opcode = instr->opcode;
        
        expr->id = 10;
        
        ast* expr1 = NULL; // = expr->op.expression.left;
        ast* expr2 = NULL; // = expr->op.expression.right;
        
        if (!cphvb_is_constant(instr->operand[1])) {
            expr1 = ssa_lookup_latest(nametable,instr->operand[1]);
        }
        
        if (expr1 == NULL) {        
            expr1 = new ast();        
            operand_to_exp(instr,1,expr1);                
            if (!cphvb_is_constant(instr->operand[1])) {
                ssa_add(nametable,instr->operand[1],expr1);        
            }
        } 
        
        if (cphvb_operands(instr->opcode) == 3) {
            
            // binary operation
            if (!cphvb_is_constant(instr->operand[2])) {
                expr2 = ssa_lookup_latest(nametable,instr->operand[2]);                
            }
            
            if (expr2 == NULL) {            
                printf("expr2  == null\n");
                expr2 = new ast();            
                operand_to_exp(instr,2,expr2);            
                if (!cphvb_is_constant(instr->operand[2])) {
                    //printf("-*-  \n");    
                    ssa_add(nametable,instr->operand[2],expr2);
                }
            } 
        }

        expr->op.expression.left = expr1;
        expr->op.expression.right = expr2;
        
        if (_JIT_LOG_LEVEL >= 4) {
            printf("\n! %p\n",instr->operand[0]);
            printf("¤ L %p\n",expr->op.expression.left);
            printf("¤ R %p\n",expr->op.expression.right);
        }
        ssa_add(nametable,instr->operand[0],expr);
                
        
        //add_ua(ssauaTable,instr->operand[2],ssa_lookup_version(instr->operand[2]),instr->operand[0],ssa_lookup_version(instr->operand[0]))
        //add_ua(ssauaTable,instr->operand[1],ssa_lookup_version(instr->operand[1]),instr->operand[0],ssa_lookup_version(instr->operand[0]))
        
         
        //printf("tag: %d",(*nametable)[instr->operand[0]]->tag);
        //printf("nt:s: %d\n",(*nametable).size());

        //print_ast_node(expr1);    
        //print_ast_node(expr2);

        //print_nametable(*nametable);   
        //printf("\n");
        //print_ssa_nametable(*nametable);
        //print_ssavt( ssa_lookup_vt(nametable,instr->operand[0]));
                    
            
        if (num_operands == 3) {
            // binary arithmic operations
            
        } else if (num_operands == 2) {
            // unary arithmic operations        
        }
        
    } else {
        // handle controll- and userfunctions
        
        switch(instr->opcode) {
            case CPHVB_NONE: // NOOP.                
                break; 
                
            case CPHVB_DISCARD:
                break; 
            
            case CPHVB_SYNC:
                break; 

            
            case CPHVB_USERFUNC: 
                break;                     
        }        
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









// Expr* ssa_lookup_latest(ssalist,array) (use pair<version,expr> ? )

// cphvb_intp ssa_lookup_latest_version(ssalist,array)

// Expr* ssa_lookup(ssalist, array, version) // lookup name:version -> Full Expr

// ssavt* ssa_vt_lookup(ssalist, array) // lookup the version table map for a array.

// ssa_add(ssalist,array,expr)


//name == <array*, version>,
//name = Expr



//~ /**  
 //~ * Lookup the SSA name and exp for the array in the ssalist. If the array
 //~ * exists the ssa_version_table for the base array is returned.
 //~ * If it does not exists out is null
 //~ * */
//~ cphvb_error ssa_lookup1(map<cphvb_array*,ssavt*> ssalist, cphvb_array* array, ssavt* out) 
//~ {    
    //~ // error handling missing
    //~ 
    //~ std::map<cphvb_array*,ssavt*>::iterator it = ssalist.find(array);
    //~ if(it == ssalist.end()) {
        //~ out = NULL;        
    //~ } else {
        //~ out = it->second;
    //~ }
    //~ 
    //~ return CPHVB_SUCCESS;
//~ }



// cphvb_instruction* inst
//~ cphvb_error ssa_add1(map<cphvb_array*,ssavt*> ssalist, cphvb_array* key_pointer, ast* expr) {    
    //~ ssavt* vt;
    //~ cphvb_error result;
    //~ // lookup the out_base
    //~ result = ssa_lookup(&ssalist, key_pointer, vt);
    //~ if (result != CPHVB_SUCCESS) {
        //~ return result;
    //~ }
        //~ 
    //~ if (vt == NULL) {
        //~ std::map<cphvb_intp,ast*> version_table;
        //~ version_table = std::map<cphvb_intp,ast*>();
        //~ version_table[0] = expr;
        //~ 
        //~ // no entry exist for the array, create one.
        //~ vt = (ssavt*) malloc(sizeof(ssavt));
        //~ vt->version = 0;
        //~ vt->table = &version_table;        
        //~ //vt->table[0] = expr;
        //~ 
        //~ ssalist[key_pointer] = vt;
        //~ 
    //~ } else {
        //~ 
        //~ vt->version++;
        //~ //vt->table[0][0] = expr;                
        //~ //(*vt->table).insert(std::pair<cphvb_intp,ast*>(vt->version,expr));                
        //~ (*vt->table)[vt->version] = expr;
        //~ //std::map<cphvb_intp,ast*> version_table = *vt->table;
        //~ //version_table.insert(std::pair<cphvb_intp,ast*>(vt->version,expr));                
    //~ }
    //~ 
//~ 
    //~ // assign
//~ 
    //~ return CPHVB_SUCCESS;    
//~ }



















