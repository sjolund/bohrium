// jit_common
// could hold commonly used functions and data structures. 


// print functions

#include "jit_common.h"
#include <string>
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

void cphvb_pprint_instr_list_small( cphvb_instruction* instruction_list, cphvb_intp instruction_count, const char* txt )
{
    cphvb_intp count;
    printf("%s %d {\n", txt, (int)instruction_count);
    for(count=0; count < instruction_count; count++) {
        cphvb_pprint_instr_small( &instruction_list[count] );
    }
    printf("}\n");
}


string jit_pprint_nametable(jit_name_table* nt) {
    stringstream ss;
    jit_name_table::iterator it;  
    ss << "Nametable: [";  
    for (it=nt->begin();it != nt->end();it++) {
        ss << *it << ":" << &(*it)->expr << " ";
    }    
    ss << "]";
    return ss.str();
}
