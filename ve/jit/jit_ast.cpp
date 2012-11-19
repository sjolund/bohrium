// jit_ast.cpp

#ifndef _JIT_LOG_LEVEL
#define _JIT_LOG_LEVEL 1
#endif

#include "cphvb.h"
#include "jit_ast.h"
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <list>
#include "jit_ssa_analyser.h"
#include <stdarg.h>
#include "StringHasher.hpp"

void constant_value_text(cphvb_constant* constant, char buff[]);
void print_ast_recursive(int step, ast* node);
const char* opcode_symbol_text(cphvb_opcode opcode);


bool is_bin_op(jit_expr* expr) {
    return expr->tag == bin_op;
}
bool is_un_op(jit_expr* expr) {
    return expr->tag == un_op;
}
bool is_array(jit_expr* expr) {
    return expr->tag == array_val;
}
bool is_constant(jit_expr* expr) {
    return expr->tag == const_val;
}
bool is_userfunc(jit_expr* expr) {
    return expr->tag == expr_type_userfunc;
}



jit_expr* cphvb_constant_to_jit_expr(cphvb_constant* constant) {
    jit_expr* e = new jit_expr();    
    e->tag = const_val;    
    e->op.constant = constant;
    
    e->id = 0;
    e->depth = 0;
    e->parent = NULL;
    return e;
}



jit_expr* cphvb_array_to_jit_expr(cphvb_array* array) {
    jit_expr* e = new jit_expr();
    e->tag = array_val;
    e->op.array = array;
    
    e->id = 0;    
    e->depth = 0;
    e->name = -1;
    e->parent = NULL;
    return e;   
}



// ==============================
// Print function definitions
// ==============================
/// todo: move to common


void constant_to_string(cphvb_constant* constant, char buff[]) {    
    constant_value_text(constant,buff);
    std::cout << sprintf(buff,"Const: (%s) %s \n",cphvb_type_text(constant->type), buff ) << "\n";    
}

void add_fill(int step, std::stringstream* ss) {
    for(int i=0;i<step;i++) {
        *ss << "--";
    }
}

void print_ast_name_recursive_stream(int step, ast* node, std::stringstream* ss) {
    
    switch(node->tag) {
        case bin_op:
            add_fill(step,ss);
            *ss << "B " << opcode_symbol_text(node->op.expression.opcode) << "  -  " << node->name << "\n";            
            print_ast_name_recursive_stream(step+1,node->op.expression.left,ss);
            print_ast_name_recursive_stream(step+1,node->op.expression.right,ss);
            break;
        
        case un_op:
            add_fill(step,ss);
            *ss << "U " << opcode_symbol_text(node->op.expression.opcode) << "  -  " << node->name <<"\n";            
            print_ast_name_recursive_stream(step+1,node->op.expression.left,ss);
            break;
        
        case const_val:
            add_fill(step,ss);
            *ss << "Const: " << node->op.constant << " " << node->tag << "\n"; 
            //*ss << "[" << step << "] Const: " << node->op.constant << " " << node->tag << "\n"; 
            //printf("[%d] Const: %p %d\n",step,node->op.constant,node->tag);
            break;
        
        case array_val:
            add_fill(step,ss);
            *ss << "Array: " << node->op.array << " " << node->tag << "\n"; 
            //*ss << "[" << step << "] Array: " << node->op.array << " " << node->tag << "\n"; 
            //printf("[%d] Array: %p %d\n",step,node->op.array,node->tag);
            break;
    }    
}


void print_ast_recursive_stream(int step, ast* node, std::stringstream* ss) {
    
    switch(node->tag) {
        case bin_op:
            add_fill(step,ss);
            *ss << "B " << opcode_symbol_text(node->op.expression.opcode) << "\n";            
            print_ast_recursive_stream(step+1,node->op.expression.left,ss);
            print_ast_recursive_stream(step+1,node->op.expression.right,ss);
            break;
        
        case un_op:
            add_fill(step,ss);
            *ss << "U " << opcode_symbol_text(node->op.expression.opcode) << "\n";            
            print_ast_recursive_stream(step+1,node->op.expression.left,ss);
            break;
        
        case const_val:
            add_fill(step,ss);
            *ss << "Const: " << node->op.constant << " " << node->tag << "\n"; 
            //*ss << "[" << step << "] Const: " << node->op.constant << " " << node->tag << "\n"; 
            //printf("[%d] Const: %p %d\n",step,node->op.constant,node->tag);
            break;
        
        case array_val:
            add_fill(step,ss);
            *ss << "Array: " << node->op.array << " " << node->tag << "\n"; 
            //*ss << "[" << step << "] Array: " << node->op.array << " " << node->tag << "\n"; 
            //printf("[%d] Array: %p %d\n",step,node->op.array,node->tag);
            break;
    }
    
} 


cphvb_error print_ast_node(ast* node) {    
    ExprType t;
    //printf("print_ast_node\n");    
    //std::cout << node << "\n";                    
    
    if (node == NULL) {
        return CPHVB_ERROR;
    }
    char buff[50];
    switch(node->tag) {
        case bin_op:                     
            printf("B:  id:%ld  op: %s\n", node->id, cphvb_opcode_text(node->op.expression.opcode) );            
            //std::cout << node->id << "\n";
            t = node->op.expression.left->tag;            
            if (t == bin_op || t == un_op)        
            {
                printf("1] Exp \n");
            } else {
                if (t == const_val) {                    
                    constant_to_string(node->op.expression.left->op.constant, buff);
                    printf("1] %s",buff);
                    //printf("1] Const: (%s) %s \n",cphvb_type_text(node->op.binary.left->op.constant->type),  constant_value_text(node->op.binary.left->op.constant));
                } else {
                    printf("1] Array \n");
                }
            }
                        
            t = node->op.expression.right->tag;            
            if (t == bin_op || t == un_op) {
                printf("2] Exp \n");
            } else {
                if (t == const_val) {
                    constant_to_string(node->op.expression.left->op.constant,buff);
                    printf("2] %s",buff);
                    //printf("2] Const: (%s) %f \n",cphvb_type_text(node->op.binary.right->op.constant->type),constant_value_text(node->op.binary.right->op.constant));
                } else {
                    printf("2] Array \n");
                }
            }                
            // check the "children" expressions.
            
            
            break;
        case un_op:            
            printf("U: id:%ld op_ %s\n", node->id, cphvb_opcode_text(node->op.expression.opcode));            
            break;
        case const_val:                
            constant_to_string(node->op.constant,buff);
            printf("Const: %s\n", buff); 
            break;
        case array_val:             
            printf("Array: %p\n",node->op.array); 
            break;
        default:            
            printf("- \n"); 
            break;
    }        
    return CPHVB_SUCCESS;
}



const char* opcode_symbol_text(cphvb_opcode opcode) {
    switch(opcode) {
        case CPHVB_ADD:
            return "+";
        case CPHVB_SUBTRACT:
            return "-";
        case CPHVB_MULTIPLY:
            return "*";
        case CPHVB_DIVIDE:
            return "/";          
        case CPHVB_IDENTITY:
            return "==";
      }
      return "";
}


void constant_value_text(cphvb_constant* constant, char buff[]) {    
    cphvb_type type = constant->type;        
        
    // case the different cphvb_type's
    //printf("** %s : ", cphvb_type_text( type) );
    
    switch (type) {
        case CPHVB_INT8:
            sprintf(buff,"%d", constant->value.int8);            
            break; 
            
        case CPHVB_INT16:                         
            sprintf(buff,"%hd",constant->value.int16);
            break;
        
        case CPHVB_INT32:             
            sprintf(buff,"%d",constant->value.int32);
            break;
        
        case CPHVB_INT64:             
            sprintf(buff,"%ld",constant->value.int64);
            break;            
            
        case CPHVB_UINT8:
            sprintf(buff,"%d",constant->value.uint8);
            break;      
                
        case CPHVB_UINT16:
            sprintf(buff,"%u",constant->value.uint16);
            break;      
                
        case CPHVB_UINT32:        
            sprintf(buff,"%d",constant->value.uint32);            
            break;      
                
        case CPHVB_UINT64:
            sprintf(buff,"%lu",constant->value.uint64);            
            break;
                    
        case CPHVB_FLOAT16:
            sprintf(buff,"%f",(float) constant->value.float16);
            break;
                
        case CPHVB_FLOAT32:
            sprintf(buff,"%f",constant->value.float32);
            break;
        
        case CPHVB_FLOAT64:
            sprintf(buff,"%f",constant->value.float64);
            break;
    }
        
    return ;
    //std::cout << constant->value.int8;
    //printf(out,constant->value);
}

void constant_value_text_s(cphvb_constant* constant) {
    char buff[10];
    constant_value_text(constant,buff);
    printf("%s",buff);
}
    

void test_constant_to_string(void) {    
    char buff[50];
    std::cout << "testing ConstantToString:\n";    
    cphvb_constant* con = new cphvb_constant;    
        
    con->value.int8 = 51;
    con->type = CPHVB_INT8;
    constant_value_text( con, buff );    
    std::cout << buff << "\n";    

    con->value.int16 = 52;    
    con->type = CPHVB_INT16;        
    constant_value_text(  con, buff );    
    std::cout << buff << "\n";  
    
    con->value.int32 = 53;    
    con->type = CPHVB_INT32;        
    constant_value_text(  con, buff );    
    std::cout << buff << "\n"; 
    
    con->value.int64 = 54;    
    con->type = CPHVB_INT64;        
    constant_value_text(  con, buff );    
    std::cout << buff << "\n"; 
    
    con->value.uint8 = 55;    
    con->type = CPHVB_UINT8;        
    constant_value_text(  con, buff );    
    std::cout << buff << "\n"; 
    
    con->value.int16 = 56;    
    con->type = CPHVB_UINT16;        
    constant_value_text(  con, buff );    
    std::cout << buff << "\n"; 
    
    con->value.uint32 = 57;    
    con->type = CPHVB_UINT32;        
    constant_value_text(  con, buff );    
    std::cout << buff << "\n"; 
    
    con->value.uint64 = 58;    
    con->type = CPHVB_UINT64;        
    constant_value_text(  con, buff );    
    std::cout << buff << "\n"; 
        
    con->value.float16 = 59.1;    
    con->type = CPHVB_FLOAT16;        
    constant_value_text(  con, buff );    
    std::cout << buff << "\n"; 
    
    con->value.float32 = 60.2;    
    con->type = CPHVB_FLOAT32;        
    constant_value_text(  con, buff );    
    std::cout << buff << "\n"; 
        
    con->value.float64 = 61.3;    
    con->type = CPHVB_FLOAT64;        
    constant_value_text(  con, buff );    
    std::cout << buff << "\n"; 
}



