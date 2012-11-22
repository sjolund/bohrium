// jit_ast.h


#ifndef __JIT_AST_H
#define __JIT_AST_H

#include "cphvb.h"
#include <utility>
#include <map>
#include <vector>
#include <list>
#include <sstream>
#include <iostream>

typedef enum  {no_type = -1, bin_op = 0, un_op = 1, const_val = 2, array_val = 3, expr_type_userfunc = 4} ExprType; 
typedef enum  {no_status = -1, jit_expr_status_executed} jit_expr_status;

typedef struct Exp {
    ExprType                                        tag;
    cphvb_intp                                      id; 
    cphvb_intp                                      name; 
    cphvb_intp                                      depth;
    jit_expr_status                                 status;
    
    
    struct Exp*                                     parent;                                    
    union { cphvb_constant*                         constant;
            cphvb_array*                            array;
            cphvb_userfunc*                         userfunc;
                                                                                                               
            struct {
                cphvb_opcode            opcode;
                struct Exp*             left;
                struct Exp*             right; }    expression;            
    } op;

    std::vector<Exp*>*                              userfunction_inputs;    
} ast;

typedef ast jit_expr;
typedef ExprType jit_expr_tag;

// ==============================
// Function definitions
// ==============================

bool is_bin_op(jit_expr* expr);
bool is_un_op(jit_expr* expr);
bool is_array(jit_expr* expr);
bool is_userfunc(jit_expr* expr);
bool is_constant(jit_expr* expr);

jit_expr* cphvb_array_to_jit_expr(cphvb_array* array);
jit_expr* cphvb_constant_to_jit_expr(cphvb_constant* array);



char* expr_type_to_string(ExprType enumval);
cphvb_error print_ast_node(ast* node);
void constant_value_text_s(cphvb_constant* constant);
char* constant_value_text(cphvb_constant* constant);
void print_ast_recursive_stream(int step, ast* node, std::stringstream* ss);
void print_ast_name_recursive_stream(int step, ast* node, std::stringstream* ss);


#endif
