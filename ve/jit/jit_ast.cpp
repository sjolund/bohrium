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
//#include <utility>

void constant_value_text(cphvb_constant* constant, char buff[]);


void print_nametable(std::map<cphvb_array*,ast*> nametable);
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
bool is_userfunc(jit_expr* expr) {
    return expr->tag == expr_type_userfunc;
}


bool is_constant(jit_expr* expr) {
    return expr->tag == const_val;
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

cphvb_error array_to_exp(cphvb_array* array, ast* result) {     
    try 
    {               
        result->tag = array_val;
        result->id = 0;
        result->op.array = array;
        result->depth = 0;
    } 
    catch (int e) 
    {
        std::cout << "Error in array_to_exp: " << e << "\n";        
    }
    
    return CPHVB_SUCCESS;
}

cphvb_error const_to_exp(cphvb_constant* constant, ast* result) {       
    result->tag = const_val;
    result->id = 0;
    result->op.constant = constant;
    result->depth = 0;    
    
    //printf("p: %p tag: %d id %d: op.const: %s\n",result,result->tag, result->id, " hh ");    
    return CPHVB_SUCCESS;
}

cphvb_error operand_to_exp(cphvb_instruction *inst, cphvb_intp operand_no, ast* result) {
    
    if (cphvb_is_constant(inst->operand[operand_no])) {
        return const_to_exp(&inst->constant,result);
    } else {              
        cphvb_error err = array_to_exp(inst->operand[operand_no],result);                
        return err;
    }
}



ExprType ast_operand_count_to_tag(cphvb_intp operand_count) {
    switch(operand_count) {
        case 2: return un_op; break;
        case 3: return bin_op; break;        
        default: return no_type; break;
    }
    
}

ast* nametable_lookup(std::map<cphvb_array*,ast*>* nametable,cphvb_array* array_key ) {
            
    return (*nametable)[array_key];
}

int nametable_register(std::map<cphvb_array*,ast*>* nametable, cphvb_array* array_key, ast* expr_value ) {        
    //printf("k %p .. v %p\n", array_key, expr_value);
    (*nametable)[array_key] = expr_value;
    
    //printf("__%p == %p \n",array_key,nametable_lookup(nametable,array_key));
    
    return 0;
}



void ast_handle_instruction(std::list<ast*>* expression_list, std::map<cphvb_array*,ast*>* nametable, cphvb_instruction* instr) {
    //ast_log(">ast_handle_instruction()",LOG_DEBUG);    
    cphvb_array* name_array = instr->operand[0];
    ast* expr = new ast();    
    expr->tag = ast_operand_count_to_tag(cphvb_operands(instr->opcode));
    expr->id = 10;
    expr->op.expression.opcode = instr->opcode;
    
    ast* expr1 = NULL; //= expr->op.expression.left;
    ast* expr2 = NULL; // = expr->op.expression.right;
    
        
    if (!cphvb_is_constant(instr->operand[1])) {
        expr1 = nametable_lookup(nametable,instr->operand[1]);
    }
    
    
    if (expr1 == NULL) {
        //printf("expr1 == null\n");
        expr1 = new ast();        
        operand_to_exp(instr,1,expr1);                
        if (!cphvb_is_constant(instr->operand[1])) {
            nametable_register(nametable,instr->operand[1],expr1);        
        }
    } 
    
    if (cphvb_operands(instr->opcode) == 3) {
        
        // binary operation
        if (!cphvb_is_constant(instr->operand[2])) {
            expr2 = nametable_lookup(nametable,instr->operand[2]);            
            
        }
    
        if (expr2 == NULL) {
            //printf("expr2  == null\n");
            expr2 = new ast();            
            operand_to_exp(instr,2,expr2);            
            if (!cphvb_is_constant(instr->operand[2])) {
                //printf("-*-  \n");    
                nametable_register(nametable,instr->operand[2],expr2);
            }
        } 
    }

    expr->op.expression.left = expr1;
    expr->op.expression.right = expr2;
    
    //printf("! %p\n",instr->operand[0]);
    //printf("¤ L %p\n",expr->op.expression.left);
    //printf("¤ R %p\n",expr->op.expression.right);
    
    nametable_register(nametable,instr->operand[0],expr);
     
    //printf("tag: %d",(*nametable)[instr->operand[0]]->tag);
    //printf("nt:s: %d\n",(*nametable).size());

    //print_ast_node(expr1);    
    //print_ast_node(expr2);

    //print_nametable(*nametable);   
    
    
    
    
    // check operand 2
    
    
    //if (CPHVB_SUCCESS != create_ast_from_instruction(instr, expr)) {
        // handle error
    //    printf("error creating Expr from instruction");        
    //}
    
    // check name_array
    
    // 
    
//~ handle_instruction(nametable,instruction) 
    //~ op1,op2,op3 = instruction_to_operands(instruction)    
    //~ inst_name = operand_name(op1)
    //~ inst_exp.operation = instruction_operation(instruction)    
    //~ 
    //~ for each oprand in operands(op2,op3):
        //~ name = operand_name(op)
        //~ exp = nametable.get(name)
        //~ 
        //~ if not exp:
            //~ exp = operand_to_exp(op)
            //~ nametable.register(name,exp)
        //~ 
        //~ inst_exp.oprands.add(exp) // add in correct order
        //~ 
    //~ nametable.register(inst_name,inst_exp)        
}


/**
 * Create an Expr from the instruction. Only works with valued Expr. e.g 
 * Instructions with 2 or 3 operands. 
 **/
cphvb_error create_ast_from_instruction(cphvb_instruction *inst, ast* result) {                
    
    int printlevel = 0;
    
    if (cphvb_operands(inst->opcode) < 2) {
        //printf(".. Not a valid instruction to create a Exp node from: %d\n", inst->opcode);
        return CPHVB_SUCCESS;
    }
    //printf("opcode == 3\n\n");    
    //std::cout << "test:" << sizeof(ast) << "\n";
    ast* exp = result;
    //ast* exp = new ast; // (ast) malloc(sizeof(ast));
        
    if(cphvb_operands(inst->opcode) == 3) {
        
        exp->tag = bin_op;
        exp->op.expression.opcode = inst->opcode;
        //std::cout << "inst->opcode: " << inst->opcode << "\n";
        exp->op.expression.left = new ast;
        if (operand_to_exp(inst,1,exp->op.expression.left) != CPHVB_SUCCESS) {
            // print a error message ??
            return CPHVB_ERROR;
        }
        
        //std::cout << "exp->op.binary.left:" << exp->op.expression.opcode << "\n"; 
        exp->op.expression.right = new ast;        
        if (operand_to_exp(inst,2,exp->op.expression.right) != CPHVB_SUCCESS) {
            // print a error message ??
            return CPHVB_ERROR;
        }        
               

    } else if (cphvb_operands(inst->opcode) == 2)  {
        exp->tag = un_op;        
        exp->op.expression.opcode = inst->opcode;
        exp->op.expression.left = new ast;
        if (operand_to_exp(inst,1,exp->op.expression.left) != CPHVB_SUCCESS) {
            // print a error message ??
            return CPHVB_ERROR;
        }
    }

    //std::cout << "tag:" <<  exp->tag << "\n";
    //std::cout << "id:" <<   exp->id << "\n";
    //std::cout << "opcode:" << cphvb_opcode_text(exp->op.binary.opcode) << "\n";
    //std::cout << "opcode:" << exp->op.binary.opcode << "\n";
    
    //result->tag = exp->tag;
    *result = *exp;
    //std::cout << "tag:" <<  result << "\n";
    return CPHVB_SUCCESS;
}



std::map<cphvb_array*,ast*> assignment_table = std::map<cphvb_array*,ast*>();

/**
 * lookup and array in the assignmen_table. 
 * Return a pointer to the ast (Exp) af the array if present, NULL otherwise.
 **/
ast* at_lookup(std::map<cphvb_array*,ast*>* assignments, cphvb_array* array) {        
    //std::map<cphvb_array*,ast*> at = *assignments;
    
    return (*assignments)[array];
}

bool at_add(std::map<cphvb_array*,ast*>* assignments, cphvb_array* array, ast* ast) {
     
    try {
        if (at_lookup(assignments,array)  == NULL) {                    
            (*assignments)[array] = ast;
        }
    } catch (int e) {
        printf("Error happened while adding and array to the assignment_table\n");
        return false;
    } 
    return true;
}



void constant_to_string(cphvb_constant* constant, char buff[]) {    
    constant_value_text(constant,buff);
    std::cout << sprintf(buff,"Const: (%s) %s \n",cphvb_type_text(constant->type), buff ) << "\n";    
}



void print_nametable(std::map<cphvb_array*,ast*> nametable) {    
    std::map<cphvb_array*,ast*>::iterator it;
    
    char* buff = new char[50];
    printf("øøøøøøøøøøøøøøøøøøøøøøøøøøøøøøø\n");
    sprintf(buff,"print_nametable size() == %lu\n",nametable.size());
    //ast_log(buff,LOG_DEBUG);
    
    //printf("print_nametable size() == %d\n",nametable.size());
    for ( it=nametable.begin(); it != nametable.end(); it++ ) {
        
        cphvb_array* key = it->first;
        ast* val = it->second;     
        printf("%p : %p\n",key,val);
        print_ast_recursive(0,it->second);
        printf("\n");
    }    
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

void print_ast_recursive(int step, ast* node) {
    
    switch(node->tag) {
        case bin_op:
            printf("B[%d]  %s\n",step, opcode_symbol_text(node->op.expression.opcode));
            print_ast_recursive(step+1,node->op.expression.left);
            print_ast_recursive(step+1,node->op.expression.right);
            break;
        
        case un_op:
            printf("U[%d] %d\n",step,node->tag);
            print_ast_recursive(step+1,node->op.expression.left);
            break;
        
        case const_val:
            printf("[%d] Const: %p %d\n",step,node->op.constant,node->tag);
            break;
        
        case array_val:
            printf("[%d] Array: %p %d\n",step,node->op.array,node->tag);
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


