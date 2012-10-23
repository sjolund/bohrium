// jit_expr.cpp


#include "jit_expr.h"
#include <map>

using namespace std;

/**
 * array information is extracted such as to match the computation function for the expression.
 * Used as part of the kernel.
 **/ 
void _expr_operand_extract_traverser(jit_expr* expr, jit_analyse_state* s, map<cphvb_intp,cphvb_intp>* as,
                                        map<cphvb_constant*,cphvb_intp>* cs, stringstream* comp_ss,
                                        bool create_computation_string ) {
    printf("s _expr_operand_extract_traverser()\n");
    
    jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);
    
    if (is_array(expr) || entr->is_executed) {       
        cphvb_intp ia = -1;        
        // if not added yet        
        if(as->find(expr->name) == as->end()) {                    
            ia = as->size();
            //printf("---------- %ld  %ld\n",expr->name,ia);
            as->insert(pair<cphvb_intp,cphvb_intp>(expr->name,ia));             
        } else {
           ia = as->find(expr->name)->second;
        }

        // create computation expression
        if (create_computation_string && ia > -1) {
            if (is_array(expr)) {
                *comp_ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(expr->op.array->type) << "*) as[" << ia << "]->data))";
            } else {
                // if it is a executed entr
                *comp_ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(entr->arrayp->type) << "*) as[" << ia << "]->data))";
            }
        }       
        
    } else if(is_bin_op(expr)) {
        if (create_computation_string) {
            *comp_ss << "(";
            _expr_operand_extract_traverser(expr->op.expression.left,s,as,cs,comp_ss,create_computation_string);
            jitcg_codetext_opcode_stream(expr->op.expression.opcode,comp_ss);                                     
            _expr_operand_extract_traverser(expr->op.expression.right,s,as,cs,comp_ss,create_computation_string);
            *comp_ss << ")";
        } else {
            _expr_operand_extract_traverser(expr->op.expression.left,s,as,cs,comp_ss,create_computation_string);
            _expr_operand_extract_traverser(expr->op.expression.right,s,as,cs,comp_ss,create_computation_string);
        }   
        
        
    } else if (is_un_op(expr)) {
        if (create_computation_string) {
            jitcg_codetext_opcode_stream(expr->op.expression.opcode,comp_ss);  
            *comp_ss << "(";        
            _expr_operand_extract_traverser(expr->op.expression.left,s,as,cs,comp_ss,create_computation_string);                
            *comp_ss << ")";
        } else {
            _expr_operand_extract_traverser(expr->op.expression.left,s,as,cs,comp_ss,create_computation_string);                
        }
        
    } else  if (is_constant(expr)) {     
        cphvb_intp ic = -1;
        // if not added yet
        
        if(cs->find(expr->op.constant) == cs->end()) {            
            ic = cs->size();
            cs->insert(pair<cphvb_constant*,cphvb_intp>(expr->op.constant,ic));                                
        } else {
            ic = cs->find(expr->op.constant)->second;
        }

        if (create_computation_string) {
            *comp_ss << "*((" << cphvb_type_typetext(expr->op.constant->type)  << "*)" << " &cs[" << ic << "]->value)";
        }
    }        
    logInfo("e jitcg_expr_travers_array()\n");
}


void jit_expr_operand_extract(jit_analyse_state* s, jit_expr* expr, op_extract_state* es, bool create_computation_string) {
    stringstream ss;

    map<cphvb_intp,cphvb_intp>* as = new map<cphvb_intp,cphvb_intp>();
    map<cphvb_constant*,cphvb_intp>* cs = new map<cphvb_constant*,cphvb_intp>();

    // travers the expression and fill as and cs.
    _expr_operand_extract_traverser(expr,s,as,cs,&ss,create_computation_string);    
    
    // create arrays input-array
    es->arrays_length = (as->size());
    //printf("test %p %lu\n",es->arrays_length,as->size());
                
    es->arrays = ((cphvb_array**) malloc(sizeof(cphvb_array*)*as->size()));
    printf("%p\n",es->arrays);    
    std::map<cphvb_intp,cphvb_intp>::iterator ita;    
    for(ita = as->begin();ita != as->end();ita++) {
        cphvb_array* ainput = cphvb_base_array(jita_nametable_lookup(s->nametable,(*ita).first)->arrayp);                    
        es->arrays[(*ita).second] = ainput;
    }
    es->constants_length = cs->size();
    //printf("test %lu\n", es->constants_length,cs->size());
    // create constants input-array
    
    es->constants = (cphvb_constant**) malloc(sizeof(cphvb_constant*)*cs->size());
    
    std::map<cphvb_constant*,cphvb_intp>::iterator itc;     
    for(itc = cs->begin();itc != cs->end();itc++) {                
        es->constants[(*itc).second] = (*itc).first;
    }
        
    printf("setting comutation string\n");
    printf("computation string: %s\n",ss.str().c_str());
    
    es->computation_string = (char*) malloc(sizeof(char)*ss.str().size());
    strcpy(es->computation_string,ss.str().c_str());
    ss.clear();     
}



