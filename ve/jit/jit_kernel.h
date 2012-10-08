// jit_kernel.h


#ifndef __JIT_KERNEL_H
#define __JIT_KERNEL_H

#include "cphvb.h"
#include "jit_ast.h"
#include <map>
#include "jit_compile.h"

// instruction_num, operand

typedef pair<cphvb_intp,cphvb_intp> jit_instruction_list_coord;
// could be cphvb_intp[2] instead.

typedef struct {
    cphvb_intp num_arrays;
    cphvb_intp num_constants;
    
    // mapping between the arrays and constant input array indexes and a instruction list.
    map<cphvb_index,jit_instruction_list_coord>* array_index_map;
    map<cphvb_index,jit_instruction_list_coord>* constant_index_map;
    
} jit_kernel_instruction_list_map;


typedef struct {

    // the compute kernel - access to the kernel program, name+
    jit_comp_kernel     compute_kernel;
       
    // hash key for the kernel
    cphvb_intp          hash;

    // output arrays, will be one but could imagen more complex kernels with multiple outputs. 
    cphvb_array*       outputarray;
    //cphvb_array**       outputarrays;
    //cphvb_index         outputarrays_length;
    
    // distinct array input count
    cphvb_array**       inputarrays;
    cphvb_index         inputarrays_length;
        
    // constant input count
    cphvb_constant**    inputconstants;
    cphvb_index         inputconstants_length;
    
    // mapping between kernel and a cphvb vector program.
    jit_kernel_instruction_list_map* kernel_IL_map;
    
    // 
            
    /// if InstructionListApproah
    // a InstructionList to input and output arrays mapping.
    /// how to handle instructions not from previous instructionlists?
    /// keep previous lists?
    /// do a simple transformation and include in the nametable?
    /// the nametable should hold all information, set in the lifetime of the execution?
    ///  the data would be very small compared to the data being computed.
    ///  dead-metadata (free->discard'ed) could be removed as the last part of a execution.
    
} jit_kernel;



cphvb_intp jit_execute_jit_kernel(jit_kernel* kernel);


#endif
