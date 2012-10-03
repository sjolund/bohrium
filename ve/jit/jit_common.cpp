// jit_common
// could hold commonly used functions and data structures. 


// print functions

#include "jit_common.h"
#include <string>
#include <set>
#include <iostream>
#include <sstream>

#define PPRINT_BUF_STRIDE_SIZE 50
#define PPRINT_BUF_SHAPE_SIZE 50
#define PPRINT_BUF_OPSTR_SIZE 512
#define PPRINT_BUF_SIZE PPRINT_BUF_OPSTR_SIZE*4

using namespace std;

static void cphvb_sprint_instr_small( cphvb_instruction *instr, char buf[] ) {

    char op_str[PPRINT_BUF_OPSTR_SIZE];
    char tmp[PPRINT_BUF_OPSTR_SIZE];
    int op_count = cphvb_operands(instr->opcode);
    int i;
    sprintf(buf, "%s \tOPS=%d{ ", cphvb_opcode_text( instr->opcode), op_count );
    for(i=0; i < op_count; i++) {

        if (!cphvb_is_constant(instr->operand[i])) {
        
            sprintf(op_str,"A:%p",instr->operand[i]);
            
            //cphvb_sprint_array( instr->operand[i], op_str );
        } else {
            sprintf(op_str, "CONSTANT");
        }   
        sprintf(tmp, "  OP%d %s", i, op_str);
        strcat(buf, tmp);
        
    }
    strcat(buf, " ");
    if (instr->opcode == CPHVB_USERFUNC)
    {
        cphvb_userfunc* userfunc = instr->userfunc;
        for(i=0; i < userfunc->nout; i++) {
            //cphvb_sprint_array( userfunc->operand[i], op_str );
            sprintf(op_str,"A:%p",instr->operand[i]);
            sprintf(tmp, "  OUT%d %s\n", i, op_str);
            strcat(buf, tmp);
            if (userfunc->operand[i]->base != NULL) {
                sprintf(op_str,"A:%p",instr->operand[i]->base);
                //cphvb_sprint_array( userfunc->operand[i]->base, op_str );
                sprintf(tmp, "      %s\n", op_str);
                strcat(buf, tmp);
            }
        }
        for(i=userfunc->nout; i < userfunc->nout + userfunc->nin; i++) {
            //cphvb_sprint_array( userfunc->operand[i], op_str );
            sprintf(op_str,"A:%p",instr->operand[i]);
            sprintf(tmp, "  IN%d %s\n", i, op_str);
            strcat(buf, tmp);
            if (userfunc->operand[i]->base != NULL) {
                sprintf(op_str,"A:%p",instr->operand[i]->base);
                //cphvb_sprint_array( userfunc->operand[i]->base, op_str );
                sprintf(tmp, "      %s\n", op_str);
                strcat(buf, tmp);
            }
        }
    }
    strcat(buf, "}");
}

void cphvb_pprint_instr_small( cphvb_instruction *instr ) {

    char buf[PPRINT_BUF_SIZE];
    cphvb_sprint_instr_small( instr, buf );
    puts( buf );
}

void jit_pprint_cg_state(jitcg_state* cgs) {
    stringstream ss;    
    ss << "CGS : " << cgs->sync_expr_name << "\n";
    ss << "  as: " << cgs->array_inputs_len << "\n";
    ss << "  cs: " << cgs->constant_inputs_len << "\n";
    ss << "Kernel *   : " << cgs->kernel << "\n";    
    ss << "Math       : " << cgs->source_math << "\n";
    printf(ss.str().c_str());
}
    

void cphvb_pprint_instr_list_small( cphvb_instruction* instruction_list, cphvb_intp instruction_count, const char* txt )
{
    cphvb_intp count;
    printf("%s %d {\n", txt, (int)instruction_count);
    for(count=0; count < instruction_count; count++) {
        cphvb_pprint_instr_small( &instruction_list[count] );
    }
    printf("}\n");
}
void _jit_expr_tag_short_text_stream(jit_expr_tag tag,stringstream* ss) {
    switch(tag) {
        case bin_op:
            *ss << "B";
            break;
        case un_op:
            *ss << "U";
            break;
        case const_val:
            *ss << "C";
            break;
        case array_val:
            *ss << "A";
            break;
        default:
            *ss << "-";
    }
}
void _jit_expr_tag_text_stream(jit_expr_tag tag,stringstream* ss) {
    switch(tag) {
        case bin_op:
            *ss << "Bin";
            break;
        case un_op:
            *ss << "Una";
            break;
        case const_val:
            *ss << "Con";
            break;
        case array_val:
            *ss << "Arr";
            break;
        default:
            *
            ss << "-";
    }
}

void jit_pprint_expr_short_ss(jit_expr* expr,stringstream* ss) {
    if (expr->tag == array_val || expr->tag == const_val) {
        _jit_expr_tag_short_text_stream(expr->tag, ss);
        return;
    }
    
    *ss << " ";
    _jit_expr_tag_short_text_stream(expr->tag, ss);

    if (expr->tag == bin_op) {
        *ss << "  |  ";
        _jit_expr_tag_short_text_stream(expr->tag, ss);        
    }
    
}

void jit_pprint_nametable_entry(jit_name_entry* entry) {
    stringstream ss;    
    
    ss << entry->expr->name << " :[ ";
        
    //ss << cphvb_opcode_text(entry->expr->op.expression.opcode);    
    ss << " ] " <<  entry->arrayp << " . ";
    jit_pprint_expr_short_ss(entry->expr,&ss);
    ss << "\n";
    
    printf(ss.str().c_str());
}

void nametable_entry_text(jit_name_entry* ne, stringstream* ss) {        
    *ss <<  " e:" << ne->expr << " a:" << ne->arrayp <<  " b:" << ne->arrayp->base << "\n";    
}

void jit_pprint_ssamap(jit_ssa_map* ssamap) {    
    stringstream ss;
    jit_ssa_map::iterator it;
    int i;
    for(it=ssamap->begin();it!=ssamap->end();it++) {
        ss << (*it).first << " v's: [";
        for(i=0; i<(*it).second->size();i++) {
             ss << (*it).second->at(i) << "," ;
        } 
        ss << "]\n";
    }
    printf(ss.str().c_str())    ;
}

void jit_pprint_dependency_entry_values(jit_name_table* table) {

    stringstream ss;
    jit_name_table::iterator it;
    set<cphvb_intp>* nte;
    set<cphvb_intp>::iterator nit;
    ss << "Nametable: [\n";  
    for (it=table->begin();it !=table->end();it++) {


        
        nte = (*it)->tdon;
        for(nit=nte->begin();nit!=nte->end();nit++) {
            ss << (*nit) << ",";
        }        
        
        nte = (*it)->tdto;
        ss << "]\nDTO [";
        for(nit=nte->begin();nit!=nte->end();nit++) {
            ss << (*nit) << ",";
        }        
        ss << "]\n\n";

        
        //nametable_entry_text((*it),&ss);
        if ((*it)->expr->depth > 0) {
            ss << "Array: " << (*it)->arrayp << " =\n";
        }
        print_ast_recursive_stream(0,(*it)->expr,&ss);
                
    }    
    ss << "]";
    printf(ss.str().c_str());            
}

void jit_pprint_dependency_graph(jit_dependency_graph* graph) {
    stringstream ss; 
    jit_dependency_graph::iterator it;
    vector<jit_name_entry*>::iterator nit;
    vector<jit_name_entry*>* nte; 
    for(it=graph->begin(); it != graph->end();it++) {
        ss << (*it).first << ": N " << (*it).second << "  E " << (*it).second->name_table_entry->arrayp->base << "\nDON [";
        nte = (*it).second->dep_ons;
        for(nit=nte->begin();nit!=nte->end();nit++) {
            ss << (*nit)->expr->name << ",";
        }
        
        nte = (*it).second->dep_tos;
        ss << "]\nDTO [";
        for(nit=nte->begin();nit!=nte->end();nit++) {
            ss << (*nit)->expr->name << ",";
        }        
        ss << "]\n\n";
    }
    printf(ss.str().c_str());
}

void jit_pprint_expr(jit_expr* expr) {
    stringstream ss;
    print_ast_recursive_stream(0,expr,&ss);
    printf(ss.str().c_str());
}

void jit_pprint_base_dependency_list(vector<cphvb_intp>* ep_list) {
    stringstream ss;
    vector<cphvb_intp>::iterator itvec;     
    ss << ": [";
    for(itvec = ep_list->begin(); itvec != ep_list->end();itvec++) {
        ss << (*itvec) << ",";
    } 
    ss << "]\n";
    
    
    printf(ss.str().c_str());
    ss.clear();   
}

void jit_stream_text_set(set<cphvb_intp>* iset, stringstream* ss) {
    
    set<cphvb_intp>::iterator it;     
    *ss << ": [";
    for(it = iset->begin(); it != iset->end();it++) {
        (*ss) << (*it) << ",";
    } 
    (*ss) << "]";
}

void jit_pprint_set(set<cphvb_intp>* iset) {
    stringstream ss;
    set<cphvb_intp>::iterator it;     
    ss << ": [";
    for(it = iset->begin(); it != iset->end();it++) {
        ss << (*it) << ",";
    } 
    ss << "]\n";
    
    
    printf(ss.str().c_str());
    ss.clear();   
}



void jit_pprint_base_dependency_table(jit_base_dependency_table* base) {
    stringstream ss;    
    jit_base_dependency_table::iterator it;
    vector<cphvb_intp>::iterator itvec;
    for(it=base->begin();it!=base->end();it++) {
        ss << "base " << it->first << ": [";    
        for(itvec = it->second->begin(); itvec != it->second->end();itvec++) {
            ss << (*itvec) << ",";
        } 
        ss << "]\n";
    }
    
    printf(ss.str().c_str());
    ss.clear();    
}

void jit_pprint_list_of_expr(vector<jit_expr*>* expr_list) {
    stringstream ss;    
    vector<jit_expr*>::iterator it;
    ss << " list [";   
    for(it=expr_list->begin();it!=expr_list->end();it++) {
        ss << (*it)->name << ",";        
    }
    ss << "]\n";
    
    printf(ss.str().c_str());
    ss.clear(); 
}

void jit_pprint_nametable_dependencies(jit_name_table* nt) {
    stringstream ss;
    jit_name_table::iterator it;  
    ss << "Nametable dep's: [\n";  
    for (it=nt->begin();it != nt->end();it++) {
        ss << (*it)->expr->name << ": " ;
        ss << "tdon: ";
        jit_stream_text_set((*it)->tdon,&ss);
        ss << "tdto: ";
        jit_stream_text_set((*it)->tdto,&ss);
        ss << "\n";      
    }    
    ss << "]";
    printf(ss.str().c_str());    
}

string jit_pprint_nametable(jit_name_table* nt) {
    stringstream ss;
    jit_name_table::iterator it;  
    ss << "Nametable: [\n";  
    for (it=nt->begin();it != nt->end();it++) {
        ss << (*it)->expr->name << ":" ;
        //nametable_entry_text((*it),&ss);
        if ((*it)->expr->depth > 0) {
            ss << "Array: " << (*it)->arrayp << " =\n";
        }
        print_ast_recursive_stream(0,(*it)->expr,&ss);
                
    }    
    ss << "]";
    printf(ss.str().c_str());
    return ss.str();
}

