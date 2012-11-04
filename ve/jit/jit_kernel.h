// jit_kernel.h
#ifndef __JIT_KERNEL_H
#define __JIT_KERNEL_H

#include "cphvb.h"
#include "jit_ast.h"
#include "jit_compile.h"
#include "jit_analyser.h"
#include "jit_mcache.h"
#include "jit_computing.h"
#include <map>
#include <vector>



typedef struct {
    cphvb_intp  instruction;    
    cphvb_intp  operand;
    
            
} jit_instruction_list_coord2;

typedef pair<cphvb_intp,cphvb_intp> jit_instruction_list_coord; // <instruction_num, operand>
// could be cphvb_intp[2] instead.

typedef struct {
    cphvb_index array_map_length;
    cphvb_index constant_map_length;
    cphvb_index output_array_map_length;

    jit_instruction_list_coord2* array_map;
    jit_instruction_list_coord2* constant_map;
    jit_instruction_list_coord2* output_array_map;

} jit_io_instruction_list_map;


typedef struct {
    vector< jit_instruction_list_coord* >* array_map;
    vector< jit_instruction_list_coord* >* constant_map;
    vector< jit_instruction_list_coord* >* output_array_map;    
} jit_io_instruction_list_map_lists;


enum jit_execute_kernel_type { JIT_EXPR_KERNEL, JIT_COMPILE_KERNEL};

typedef struct {
    jit_expr*           expr;
    cphvb_instruction*  instructions;    
    cphvb_index         instructions_length;
    cphvb_index         instruction_num;
    bool                is_userfunc;
    
    
} jit_expr_kernel;

typedef struct {
    // type of execution kernel
    jit_execute_kernel_type type;    
    union { jit_expr_kernel*    expr_kernel;
            jit_comp_kernel*    compute_kernel;   
    };
    // id for the kernel.
    cphvb_intp          id;

    // hash key for the expr - currently not used
    cphvb_intp          hash;

    // mapping between execute expr and a cphvb vector program.
    jit_io_instruction_list_map_lists* il_map;
            
} jit_kernel;



// Used to wrap a kernel with the extracted pointers to the arrays and constants from an
// instruction list. Used as input for execution of a kernel.
typedef struct {    
    jit_kernel*         kernel;
        
    // output arrays, supporting custom function would require no restrictions on output arrays aswell as input arrays.
    cphvb_array**       outputarrays;           // not used. The first outputarray_length elements of arrays is output arrays
    cphvb_index         outputarrays_length;    // used to know the number of output elements to allocate.
    
    // distinct array input count
    cphvb_array**       arrays;
    cphvb_index         arrays_length;
        
    // constant input count
    cphvb_constant**    inputconstants;
    cphvb_index         inputconstants_length;
    
    
} jit_execute_kernel;



typedef struct {
    cphvb_intp              id;
    //jit_kernel**            kernels;
    jit_execute_kernel**    exekernels;
    cphvb_index             kernels_length;
    cphvb_opcode*           instruction_opcode_list;
        
} jit_compound_kernel;

//cphvb_intp jit_execute_jit_kernel(jit_kernel* kernel);
//cphvb_intp build_execute_expr(jit_analyse_state* s, jit_expr* expr, jit_execute_expr* out);

cphvb_intp build_compound_kernel(jit_analyse_state* s, set<cphvb_intp>* execution_list, cphvb_intp compound_id, jit_compound_kernel* out);
cphvb_intp bind_compound_kernel(jit_compound_kernel* ckernel, cphvb_instruction* instruction_list, cphvb_intp id);
cphvb_intp execute_compound_kernel(jit_compute_functions* compute_functions, jit_compound_kernel* ckernel, cphvb_instruction* list);

const char* executekernel_type_to_string(jit_execute_kernel* exekernel);

bool is_expr_kernel(jit_kernel* kernel);
bool is_compile_kernel(jit_kernel* kernel);



#endif





















