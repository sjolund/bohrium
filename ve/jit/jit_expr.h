//jit_expr.h

#ifndef __JIT_EXPR_H
#define __JIT_EXPR_H

#include "cphvb.h"
#include <map>
#include "jit_ast.h"
#include "jit_analyser.h"
#include "jit_codegenerator.h"
#include "jit_logging.h"
#include <iostream>
#include <sstream>
#include <string>

typedef struct {
    cphvb_array**       arrays;    
    cphvb_index         arrays_length;
    
    cphvb_constant**    constants;
    cphvb_index         constants_length;

    char*               computation_string;
    
} op_extract_state;


enum jit_operand_type {JIT_ARRAY, JIT_CONSTANT};
typedef struct {
    jit_operand_type    type;
    union {
        cphvb_array*    array;
        cphvb_constant* constant;
    };      
} jit_operand;

void jit_expr_operand_extract(jit_analyse_state* s, jit_expr* expr, op_extract_state* es, bool create_computation_string);

#endif
