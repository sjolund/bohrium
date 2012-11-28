// jit_kernel.cpp


#include <time.h>

#include "cphvb.h"
#include "jit_kernel.h"
#include "jit_kernel_cache.h"
#include "jit_analyser.h"
#include "jit_expr.h"
#include "jit_common.h"
#include "jit_computing.h"
#include "jit_codegenerated_kernel_functions.h"
#include "jit_compile.h"

#define K_TIMEING 0 // 0 none, 1 all, 2 compiled only
#define K_PRINT_COMPUTESTRING 0

// Settings for comparison
#ifndef JITCG_FUNCTEXT
#define JITCG_FUNCTEXT JITCGFT_NoCast 
//#define JITCG_FUNCTEXT JITCGFT_Vanilla
#endif

#ifndef JITC_COMPILE_METHOD
//#define JITC_COMPILE_METHOD COMPILE_METHOD_TCC
#define JITC_COMPILE_METHOD COMPILE_METHOD_GCC
#endif



using namespace std;

//cphvb_index oldtv_sec = 0;
//cphvb_index oldtv_nsec = 0;


void expr_count_operands(jit_expr* expr, cphvb_intp* arrays_out, cphvb_intp* constants_out) {
    if(is_array(expr)) {        
        *arrays_out += 1;
    }
    if (is_constant(expr)){
        *constants_out += 1;
    }
    if (is_un_op(expr)){
        expr_count_operands(expr->op.expression.left,arrays_out,constants_out);
            
    }
    if (is_bin_op(expr)) {
        expr_count_operands(expr->op.expression.left,arrays_out,constants_out);
        expr_count_operands(expr->op.expression.right,arrays_out,constants_out);
    }    
} 


bool is_expr_kernel(jit_kernel* kernel) {
    return kernel->type == JIT_EXPR_KERNEL;
}
bool is_compile_kernel(jit_kernel* kernel) {
    return kernel->type == JIT_COMPILE_KERNEL;
}

const char* executekernel_type_to_string(jit_execute_kernel* exekernel) {
    switch(exekernel->kernel->type) {
        case JIT_EXPR_KERNEL:
            return "expr-kernel";
        case JIT_COMPILE_KERNEL:
            return "compile-kernel";
        default:
            return "NONE";        
    }
}

void instruction_copy_to(cphvb_instruction* from, cphvb_instruction* to) {
    bool cloglevel[] = {0,0,0};
    logcustom(cloglevel,0,"instruction_copy_to()\n");
    
    to->opcode = from->opcode;
    to->status = from->status;    
    to->constant = from->constant;    
    //to->operand cphvb_array* = operand[CPHVB_MAX_NO_OPERANDS];
        
    logcustom(cloglevel,1,"%ld %ld %p %p\n",to->status,to->opcode,&to->constant,from->userfunc);

    if (from->opcode == CPHVB_USERFUNC) {        
        to->userfunc = (cphvb_userfunc*) malloc(from->userfunc->struct_size);
        memcpy(to->userfunc,from->userfunc,from->userfunc->struct_size);        
    } else {
        to->userfunc = NULL;
    }
    //logcustom(cloglevel,1,"FrTo: %d %d, %d %d\n",from->userfunc->nin,to->userfunc->nin,from->userfunc->nout,to->userfunc->nout);
}

/**
 * Update the depth indicator for and expression to reflect if children
 * have been added to the executionlist. (dependency-cut-offs).
 *
 * Could be moved into the dependency analysis part. 
 * @return correct depth of the expr.
 **/
cphvb_index jit_update_expr_depth(jit_analyse_state* s, jit_expr* expr) {
    bool cloglevel[] = {0,0};    
    logcustom(cloglevel,0,"jit_update_expr_depth()\n");
    
    if(is_constant(expr) || is_array(expr)) {
        return 0;
    }

    jit_name_entry* entr = jita_nametable_lookup(s->nametable, expr->name);
    if (entr->is_executed) {
        expr->depth = 0;        
        return 0;
    }

    cphvb_index ld, rd;

    if(is_un_op(expr)) {
        ld = jit_update_expr_depth(s,expr->op.expression.left);
        expr->depth = ld+1;
    }
    
    if(is_bin_op(expr)) {
        ld = jit_update_expr_depth(s,expr->op.expression.left);
        rd = jit_update_expr_depth(s,expr->op.expression.right);
        expr->depth  = max(ld,rd)+1;        
    }
        
    return expr->depth;    
}

/**
 * Allocate result arrays for an execute kernel. 
 **/
cphvb_error allocate_for_exekernel(jit_execute_kernel* ekernel) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"AFE allocate_for_exekernel(%ld)\n",ekernel->kernel->id);
    cphvb_error res;
    logcustom(cloglevel,1,"AFE num outputs %ld \n",ekernel->outputarrays_length);

    
    
    // this will be 1. Could be extended to more if more complex kernels where to be created.
    for(int i=0;i<ekernel->outputarrays_length;i++) {
        logcustom(cloglevel,0,"outputarray* = %p\n",ekernel->arrays[i]);
        if (cphvb_base_array(ekernel->arrays[i])->data == NULL) {
            res = jit_vcache_malloc(ekernel->arrays[i]);
            if (res != CPHVB_SUCCESS) {
                logcustom(cloglevel,1,"AFE Failed! allocation [%d]\n",i);                
                return res;
            }
            logcustom(cloglevel,1,"AFE Allocated output array[%d]\n",i);
        }
        logcustom(cloglevel,1,"AFE Array is bounded: %p\n",cphvb_base_array(ekernel->arrays[i])->data);
    }
    return CPHVB_SUCCESS;    
}           

// ==============================
// IL Map related functions
// ==============================

/**
 * 
 **/
cphvb_array* ilmap_fetch_instruction_array(cphvb_instruction* instruction, cphvb_index operand) {
    bool cloglevel[] = {0,0};
    // if the instruction is a userfunc
    if(instruction->opcode == CPHVB_USERFUNC) {
        logcustom(cloglevel,0,"Is userfunc\n");
        return instruction->userfunc->operand[operand];            
    }    
    return instruction->operand[operand];
}

/**
 * 
 **/
void expr_travers_distinct_traverser(jit_analyse_state* s, jit_expr* expr, map<cphvb_array*,cphvb_intp>* arrays, map<cphvb_constant*,cphvb_intp>* constants ) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"ETDT expr_travers_distinct_traverser()\n");
    jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);
    if(is_constant(expr)) {
        logcustom(cloglevel,0,"ETDT constant \n");
        constants->insert(pair<cphvb_constant*,cphvb_intp>(expr->op.constant,(cphvb_intp)constants->size()));
        
        
    } else if (is_array(expr) || entr->is_executed) {
        logcustom(cloglevel,0,"ETDT array %d %d\n", entr->is_executed,is_array(expr));
        cphvb_array* arr = (entr->is_executed)? entr->arrayp :  entr->expr->op.array ;
        map<cphvb_array*,cphvb_intp>::iterator it;
        if(arrays->find(arr) == arrays->end()) {            
            arrays->insert(pair<cphvb_array*,cphvb_intp>(arr,(cphvb_intp)arrays->size()));
            logcustom(cloglevel,0,"ETDT added %p\n",arr);         
        }
        
    } else if(is_bin_op(expr)) {
        logcustom(cloglevel,0,"ETDT binary \n");
        expr_travers_distinct_traverser(s,expr->op.expression.left,arrays,constants);
        expr_travers_distinct_traverser(s,expr->op.expression.right,arrays,constants);
            
    } else if (is_un_op(expr)) {
        logcustom(cloglevel,0,"ETDT unary \n");
        expr_travers_distinct_traverser(s,expr->op.expression.left,arrays,constants);
    }        
}

/**
 * 
 **/
void expr_travers_distinct(jit_analyse_state* s, jit_name_entry* entr, map<cphvb_array*,cphvb_intp>* arrays, map<cphvb_constant*,cphvb_intp>* constants ) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"ETD expr_travers_distinct()\n");
    arrays->insert(pair<cphvb_array*,cphvb_intp>(entr->arrayp,0));
    
    logcustom(cloglevel,1,"ETD output %p \n",entr->arrayp);    
    expr_travers_distinct_traverser(s,entr->expr,arrays,constants);
    entr->is_executed = true;  
}

/**
 * 
 **/
void  bind_execution_kernel_to_expression(map<cphvb_array*,cphvb_intp>* arrays, map<cphvb_constant*,cphvb_intp>* constants,jit_execute_kernel* exekernel){
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"BEKtE arrays=%ld constants=%ld\n",arrays->size(),constants->size());
    map<cphvb_array*,cphvb_intp>::iterator ait;
    for(ait=arrays->begin(); ait != arrays->end(); ait++) {            
        exekernel->arrays[ait->second] =  ait->first;
        logcustom(cloglevel,1,"BindEK exeArrays: %ld %p\n",ait->second,ait->first);
    }

    for(int i=0; i<exekernel->outputarrays_length ;i++) {
        exekernel->outputarrays[i] = exekernel->arrays[i];
        logcustom(cloglevel,1,"BindEK exeout: %d %p\n",i,exekernel->arrays[i]);
    }

    map<cphvb_constant*,cphvb_intp>::iterator cit;
    for(cit=constants->begin(); cit != constants->end(); cit++) {            
        exekernel->inputconstants[cit->second] =  cit->first;
        logcustom(cloglevel,1,"BindEK exeConstants: %ld %p\n",cit->second,cit->first);
    }    
}


/**
 * 
 **/
cphvb_intp bind_execution_kernel(jit_execute_kernel* exekernel, cphvb_instruction* instructionlist) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"BEK bind_execution_kernel()\n");    
    cphvb_intp operand, instr;

    logcustom(cloglevel,1,"BEK array_lenght: %d\n",exekernel->kernel->il_map->array_map->size());
    //jit_pprint_il_map2(exekernel->kernel->il_map);        
    // input arrays
    // output arrays

    for(uint i = 0; i < exekernel->kernel->il_map->array_map->size(); i++) {
        instr = exekernel->kernel->il_map->array_map->at(i)->first;
        operand = exekernel->kernel->il_map->array_map->at(i)->second;        
        exekernel->arrays[i] = ilmap_fetch_instruction_array(&instructionlist[instr],operand);
        logcustom(cloglevel,1,"BEK %d inA: %p (%ld,%ld)\n",i,exekernel->arrays[i],instr,operand);
        //jit_pprint_array_values(exekernel->inputarrays[i]);
    }

    // input constants
    for(uint i = 0; i < exekernel->kernel->il_map->constant_map->size(); i++) {
        instr = exekernel->kernel->il_map->constant_map->at(i)->first;
        operand = exekernel->kernel->il_map->constant_map->at(i)->second;
        exekernel->inputconstants[i] = &instructionlist[instr].constant;
        logcustom(cloglevel,1,"BEK inC: %p\n",exekernel->inputconstants[i]);
    }

    return 0;

}

/**
 * 
 **/
cphvb_intp bind_compound_kernel(jit_compound_kernel* ckernel, cphvb_instruction* instruction_list, cphvb_intp id) {
    bool cloglevel[] = {0};
    for(int i=0;i<ckernel->kernels_length;i++) {
        logcustom(cloglevel,0,"BCK Bind_compound_kernel's exekernel: %d\n",i);   
        bind_execution_kernel(ckernel->exekernels[i],instruction_list);
        //jit_pprint_execute_kernel(ckernel->exekernels[i]);
    }
    return 0;
}

/**
 * 
 **/
void build_expr_il_map_userfunc(jit_analyse_state* s,jit_name_entry* entr,
                                jit_io_instruction_list_map_lists* il_map,
                                vector<cphvb_instruction*>* is) {
                                    
        bool cloglevel[] = {0,0,0};
        logcustom(cloglevel,0,"BEIMU build_expr_il_map_userfunc() \n");
        jit_name_entry* tentr;
        int i = 0;

        for(i=0;i<entr->span;i++) {
            //logcustom(cloglevel,1,"BEIMU --- %d %p %p ",entr->expr->name,entr->instr->userfunc->operand[0],entr->instr->userfunc->operand[1]);
            //logcustom(cloglevel,1,"BEIMU %d out: (%d %d) %p (%p)\n",i,entr->instr_num,i,entr->instr,entr->instr->userfunc->operand[i]);
            il_map->output_array_map->push_back(new jit_instruction_list_coord(entr->instr_num,i));
            il_map->array_map->push_back(new jit_instruction_list_coord(entr->instr_num,i));
                        
            logcustom(cloglevel,1,"BEIMU O (%d %d) \n",entr->instr_num,i);            
        }
        
        for(uint i=0;i<entr->expr->userfunction_inputs->size();i++) {
            //logcustom(cloglevel,1,"BEIMU +++ %d %p %p ",entr->expr->name,entr->instr->userfunc->operand[0],entr->instr->userfunc->operand[1]);
                                                    
            tentr = jita_nametable_lookup(s->nametable,entr->expr->userfunction_inputs->at(i)->name);            
            //logcustom(cloglevel,1,"BEIMU in: (%d %d) %p (%p)\n",tentr->instr_num,tentr->operand_num,entr->instr,entr->instr->userfunc->operand[tentr->operand_num]);
            
            il_map->array_map->push_back(new jit_instruction_list_coord(tentr->instr_num,tentr->operand_num));
                               
            logcustom(cloglevel,1,"BEIMU I (%d %d) \n",tentr->instr_num,tentr->operand_num);
        }
        if(cloglevel[2]) {
            jit_pprint_il_map2(il_map);
        }                 
        is->push_back(entr->instr);        
}

/**
 * 
 **/
void build_expr_il_map(jit_analyse_state* s,jit_name_entry* entr,
                        jit_io_instruction_list_map_lists* il_map,
                        vector<cphvb_instruction*>* is) {
                                    
        bool cloglevel[] = {0,0,0};
        logcustom(cloglevel,0,"BEIMU build_expr_il_map_userfunc() \n");
        
        // result array
        il_map->array_map->push_back(new jit_instruction_list_coord(entr->instr_num,0));

        if(is_un_op(entr->expr)) {
            il_map->array_map->push_back(new jit_instruction_list_coord(entr->instr_num,1));
        } else {
            if (is_constant(entr->expr->op.expression.left)) {
                il_map->constant_map->push_back(new jit_instruction_list_coord(entr->instr_num,1));
                il_map->array_map->push_back(new jit_instruction_list_coord(entr->instr_num,2));
            } else {
                il_map->array_map->push_back(new jit_instruction_list_coord(entr->instr_num,1));
                
                if (is_constant(entr->expr->op.expression.right)) {
                    il_map->constant_map->push_back(new jit_instruction_list_coord(entr->instr_num,2));
                } else {
                    il_map->array_map->push_back(new jit_instruction_list_coord(entr->instr_num,2));
                }
            }
        }
        if(cloglevel[2]) {
            jit_pprint_il_map2(il_map);
        }
        is->push_back(entr->instr);        
}

// ==============================
// 
// ==============================



/**
 * 
 **/
cphvb_intp execute_userfunction(jit_compute_functions* compute_functions,cphvb_instruction* instr) {
    bool cloglevel[] = {0,0,0};
    logcustom(cloglevel,0,"EXU \n");
    
    if (cloglevel[2]) {
        for(int i=0;i<instr->userfunc->nout ;i++) {
            logcustom(cloglevel,2,"IO %p (%ld) \n",instr->userfunc->operand[i],i);
        } 
        for(int i=0;i<instr->userfunc->nin ;i++) {
            logcustom(cloglevel,2,"IA %p (%ld) \n",instr->userfunc->operand[i+instr->userfunc->nout],i+instr->userfunc->nout);
        }       
    }


    timespec time1, time2;
    if (K_TIMEING == 1) {        
        clock_gettime(CLOCK_REALTIME, &time1);    
    }
    
    cphvb_userfunc_impl ufunc = compute_functions->userfunctions->find(instr->userfunc->id)->second;

    
    //printf("instr: %ld %ld %p %p\n",instr->status,instr->opcode, &instr->constant, instr->userfunc);
    
    //cphvb_pprint_instr(instr); 
    
    if (K_TIMEING == 1) {     
        clock_gettime(CLOCK_REALTIME, &time2);  
        printf("%ld : %ld\n",diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec)); 
    }
        
    if (ufunc(instr->userfunc,NULL) == CPHVB_SUCCESS) {
        logcustom(cloglevel,1," compute_apply == SUCCESS\n");
        return 0;
    }
    logcustom(cloglevel,1," compute_apply == ERROR\n");
    return 1;
}


/**
 * 
 **/
cphvb_intp execute_instruction(jit_compute_functions* compute_functions,cphvb_instruction* instr) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"EXE \n");
    // Allocation of target array
    if (cphvb_base_array(instr->operand[0])->data == NULL) {
        logcustom(cloglevel,1," allocating array for %p\n", instr->operand[0]);
        cphvb_data_malloc(instr->operand[0]);
    }
    //cphvb_pprint_instr(instr);
    logcustom(cloglevel,0,"instr 0 = %s, [0] %p, [1] %p, [2] %p\n", cphvb_opcode_text(instr->opcode),instr->operand[0],instr->operand[1],instr->operand[2]);
    timespec time1, time2;
    if (K_TIMEING == 1) {        
        clock_gettime(CLOCK_REALTIME, &time1);    
    }
    
    //cphvb_pprint_instr(instr);
    //printf("%p\n",instr->operand[0]); if (instr->operand[0] != NULL) { jit_pprint_cphvb_array(instr->operand[0],0); }
    //printf("%p\n",instr->operand[1]); if (instr->operand[1] != NULL) { jit_pprint_cphvb_array(instr->operand[1],0); }
    //printf("%p\n",instr->operand[2]); if (instr->operand[2] != NULL) { jit_pprint_cphvb_array(instr->operand[2],0); }
    
    cphvb_error res = compute_functions->instr_compute(instr);
    

    if (K_TIMEING == 1) {     
        clock_gettime(CLOCK_REALTIME, &time2);  
        printf("%ld : %ld\n",diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec)); 
    }
    
    if (res == CPHVB_SUCCESS) {
        logcustom(cloglevel,1," compute_apply == SUCCESS\n", instr->operand[0]);
        return 0;
    }    
    
    logcustom(cloglevel,1," compute_apply == ERROR\n", instr->operand[0]);
    return 1;
}


/**
 * 
 **/
cphvb_intp execute_kernel_expr(jit_compute_functions* compute_functions,jit_execute_kernel* exekernel) {
    bool cloglevel[] = {0,0,0};
    logcustom(cloglevel,0,"EKE execute_kernel_expr(%ld)\n", exekernel->kernel->id);        

    //exekernel->kernel->il_map
    cphvb_instruction* instr = &exekernel->kernel->expr_kernel->instructions[0];
    logcustom(cloglevel,0,"EKE Instr %p\n",instr);

    if(exekernel->kernel->expr_kernel->is_userfunc) {
        int i = 0;
        logcustom(cloglevel,1,"userfunc operand[]: %d %d %p\n",instr->userfunc->nin,instr->userfunc->nout,instr->userfunc->operand[0]);
        //~ for(i=0;i<exekernel->outputarrays_length;i++) {
            //~ instr->userfunc->operand[i] = exekernel->outputarrays[i];
            //~ logcustom(cloglevel,2,"EKE oInstr[%d]=%d, outputarrays[%d]=%p\n",i,exekernel->kernel->il_map->output_array_map->at(i)->second,i,exekernel->outputarrays[i]);
        //~ }

        for(i=0;i<exekernel->arrays_length;i++) {
            int index = i;//+exekernel->outputarrays_length;
            instr->userfunc->operand[index] = exekernel->arrays[i];            
        }        
        logcustom(cloglevel,2,"EKE Userfunc instruction bounded to IL\n");
        cphvb_intp res = execute_userfunction(compute_functions,instr);
        logcustom(cloglevel,2,"EKE U res: %ld\n", res);        

    } else {
        
        //instr->operand[0] = exekernel->arrays[0];
        //logcustom(cloglevel,1,"array il_map size=%d\n",exekernel->kernel->il_map->array_map->size());
        
        for(uint i=0;i<exekernel->kernel->il_map->array_map->size();i++) {            
            instr->operand[exekernel->kernel->il_map->array_map->at(i)->second] = exekernel->arrays[i];
            logcustom(cloglevel,1,"EKE Array: %d,  [%ld] = %p\n",i,exekernel->kernel->il_map->array_map->at(i)->second,exekernel->arrays[i]);
        }

        if(exekernel->kernel->il_map->constant_map->size() > 0) {
            cphvb_index i = exekernel->kernel->il_map->constant_map->at(0)->second;
            instr->operand[i] = NULL;
            instr->constant = *exekernel->inputconstants[0];

            logcustom(cloglevel,1,"EKE Constant:  %ld %p\n",i,instr->constant);
            //constant_value_text_s(exekernel->inputconstants[0]);
        }                    
        logcustom(cloglevel,2,"EKE instruction bounded to IL\n");  
        cphvb_intp res = execute_instruction(compute_functions,instr);
        logcustom(cloglevel,2,"EKE res: %ld\n", res);        
    }
        
    return 0;
}



/**
 * 
 **/
cphvb_intp execute_kernel_compiled(jit_execute_kernel* exekernel) {
    bool cloglevel[] = {0,0,0};
    logcustom(cloglevel,0,"EKC executing compiled kernel!\n");
        
    computefunc3 func = exekernel->kernel->compute_kernel->function;        

    if (cloglevel[2]) {jit_pprint_execute_kernel(exekernel);}
    //logcustom(cloglevel,1,"O:%p \n",exekernel->outputarrays[0]);    
    
    for(int i=0;i<exekernel->arrays_length;i++) {        
        //logcustom(cloglevel,1,"A:%p \n",exekernel);
        logcustom(cloglevel,1,"A: %p \n",exekernel->arrays[i]);
        if (cloglevel[1]) {jit_pprint_cphvb_array(exekernel->arrays[i],0);}
    }    
    if (exekernel->inputconstants_length >0) {        
        logcustom(cloglevel,1,"C: %p\n",exekernel->inputconstants[0]);
    }
    //jit_pprint_cphvb_array(exekernel->inputarrays[1],0);
    //logcustom(cloglevel,1,"AO: %p\n",exekernel->outputarrays[0]);

    //~ timespec ts;
    //~ ts.tv_sec = 0;
    //~ ts.tv_nsec = 0;
    timespec time1, time2;
    if (K_TIMEING > 0) { 
        clock_gettime(CLOCK_REALTIME, &time1);    
    }
    //printf("%p %p \n",exekernel->arrays[0],exekernel->arrays[1]);
    
    func(exekernel->arrays,exekernel->inputconstants,0,0);

    //kernel_func_1158854506_0(exekernel->arrays,exekernel->inputconstants,0,0);
    //kernel_func_4265622974_0(exekernel->arrays,exekernel->inputconstants,0,0);

    if (K_TIMEING > 0) {     
        clock_gettime(CLOCK_REALTIME, &time2);  
        printf("%ld : %ld\n",diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec)); 
    }
    
    //~ timeval_subtract (timeval* result, timeval* x, timeval* y)
    //~ printf("Time taken is: %ld.%ld\n ",ts.tv_sec - oldtv_sec, ts.tv_nsec - oldtv_nsec);
    
    //~ oldtv_sec = (cphvb_index)ts.tv_sec;
    //~ oldtv_nsec = (cphvb_index)ts.tv_nsec;

    
    //kernel_func_4265622974_4(exekernel->outputarrays[0],exekernel->inputarrays,exekernel->inputarrays_length,exekernel->inputconstants,0,0);
    logcustom(cloglevel,1,"computation done\n");
    return 0;
}


/**
 * 
 **/
cphvb_intp execute_compound_kernel(jit_compute_functions* compute_functions, jit_compound_kernel* ckernel, cphvb_instruction* instructionlist) {
    bool cloglevel[] = {0,0,0,0};
    timespec time1, time2;
    cphvb_error res;
    for(int i=0;i<ckernel->kernels_length;i++) {        
        res = allocate_for_exekernel(ckernel->exekernels[i]);
        if (res != CPHVB_SUCCESS) {
            return res;
        }
        if(ckernel->exekernels[i]->kernel->type == JIT_EXPR_KERNEL) {
            logcustom(cloglevel,0,"execute_compound_kernel: JIT_EXPR_KERNEL - exekernel: %d\n",i);
            if (cloglevel[3]) { clock_gettime(CLOCK_REALTIME, &time1); }             
            execute_kernel_expr(compute_functions,ckernel->exekernels[i]);
             if (cloglevel[3]) {                
                clock_gettime(CLOCK_REALTIME, &time2);  
                printf("* execute cached expression_kernel %ld : %d\n",diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec)); 
            }
        } else
        if(ckernel->exekernels[i]->kernel->type == JIT_COMPILE_KERNEL) {
            logcustom(cloglevel,0,"execute_compound_kernel: JIT_COMPILE_KERNEL - exekernel: %d\n",i);
            logcustom(cloglevel,0,"-- %ld\n",ckernel->exekernels[i]->kernel->hash);
            if (cloglevel[3]) { clock_gettime(CLOCK_REALTIME, &time1); }                       
            execute_kernel_compiled(ckernel->exekernels[i]);
            if (cloglevel[3]) {                
                clock_gettime(CLOCK_REALTIME, &time2);  
                printf("- execute cached expression_kernel %ld : %d\n",diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec)); 
            }
        } else {
            logcustom(cloglevel,0,"execute_compound_kernel: NONE - exekernel: %d\n",i);            
        }
    } 
    return 0;
}


/**
 * Does not set the outputs for the IL map, or fill in the output map.
 **/
string _expr_extract_traverser4(jit_analyse_state* s,jit_expr* expr,
                                        map<cphvb_array*,cphvb_intp>* as,
                                        map<cphvb_constant*,cphvb_intp>* cs,                                                                                
                                        jit_io_instruction_list_map_lists* il_map,
                                        vector<cphvb_instruction*>* is,
                                        cphvb_index output_count,                                        
                                        bool create_computation_string,
                                        bool create_instruction_list) {

    bool cloglevel[] = {0,0,0};    
    logcustom(cloglevel,0,"EETNC s _expr_operand_extract_traverser_nocast()\n");
    
    stringstream ss;
    
    jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);
    logcustom(cloglevel,2,"EETNC +++ %ld .. %d executed= %s\n",entr->expr->name, expr->tag,  entr->is_executed?"True":"False");
    if (is_constant(expr)) {
        logcustom(cloglevel,1,"EETNC Constant\n");     
        cphvb_intp ic = -1;
        //printf("=========== %s - %p - ",cs->find(expr->op.constant) == cs->end()?"NoTIn":"In",expr->op.constant);
        //constant_value_text_s(expr->op.constant);
        //printf("\n");
        
        if(cs->find(expr->op.constant) == cs->end()) {            
            ic = cs->size();
            logcustom(cloglevel,1,"EETNC insert into CS: %d - %p\n",cs->size(), expr->op.constant);            
            cs->insert(pair<cphvb_constant*,cphvb_intp>(expr->op.constant,ic));
            logcustom(cloglevel,1,"EETNC %p - %ld / %ld %d\n",entr,entr->instr_num,entr->operand_num,cs->size());
            
            jit_name_entry* tentr = jita_nametable_lookup(s->nametable,expr->parent->name);
            if (tentr->instr->operand[1] == NULL) {
                 il_map->constant_map->push_back(new jit_instruction_list_coord(tentr->instr_num,1));
            } else { //tentr->instr->operand[2] == NULL
                il_map->constant_map->push_back(new jit_instruction_list_coord(tentr->instr_num,2));
            }                
        } else {            
            ic = cs->find(expr->op.constant)->second;           
        }
        if (create_computation_string) {                
            //ss << " *((" << cphvb_type_typetext(expr->op.constant->type)  << "*)" << " &cs[" << ic << "]->value) ";
            ss << jitcg_build_constant_compute_component(ic);
        }
        
    } else if (is_array(expr) || entr->is_executed) {
        logcustom(cloglevel,1,"EETNC Array\n");     
        cphvb_intp ia = -1;        
        cphvb_array* arr = is_array(expr)? expr->op.array : entr->arrayp;
        
        // find or add to array list    
        if(as->find(arr) == as->end()) {                    
            ia = as->size();
            logcustom(cloglevel,1,"EETNC Array %p, %d t: %s\n",entr->arrayp, ia, cphvb_type_text(entr->arrayp->type));
            as->insert(pair<cphvb_array*,cphvb_intp>(arr,ia));
            
            // create IL map entry.                                    
            il_map->array_map->push_back(new jit_instruction_list_coord(entr->instr_num,entr->operand_num));                            
        } else {            
            ia = as->find(arr)->second;            
        }
            
        // create computation expression
        if (create_computation_string) {
            //ss << "*(" << jitcg_nameing_array_offset("",ia+output_count) <<  " + " << jitcg_nameing_array_datapointer("",ia+output_count) << ")";
            ss << jitcg_build_array_compute_component(ia);
            //ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(arr->type) << "*) ds[" << ia << "]->data))";                        
        }       
        
    } else if(is_bin_op(expr)) {
        if (create_instruction_list) {
            is->push_back(entr->instr);
        } 
        string s1 = _expr_extract_traverser4(s,expr->op.expression.left,as,cs,il_map,is,output_count,create_computation_string,create_instruction_list);
        string s2 = _expr_extract_traverser4(s,expr->op.expression.right,as,cs,il_map,is,output_count,create_computation_string,create_instruction_list);
        logcustom(cloglevel,2,"EETNC binary, s1,s2 created\n");
        if (create_computation_string) {            
            return build_expression_string(expr->op.expression.opcode,s1,s2);
        }        
        
    } else if (is_un_op(expr)) {
        if (create_instruction_list) {
            is->push_back(entr->instr);
        }
        string s1 = _expr_extract_traverser4(s,expr->op.expression.left,as,cs,il_map,is,output_count,create_computation_string,create_instruction_list);
        logcustom(cloglevel,2,"EETNC unary, s1 created\n");
        if (create_computation_string) {            
            return build_expression_string(expr->op.expression.opcode,s1,"NONE");        
        }        
    }
    
    logcustom(cloglevel,0,"EETNC e jitcg_expr_travers_array() -- %s\n",ss.str().c_str());
    return ss.str();  
}




/**
 * 
 **/
string expr_extract_traverser_nocast(jit_analyse_state* s,jit_expr* expr,
                                map<cphvb_array*,cphvb_intp>* oas,
                                map<cphvb_array*,cphvb_intp>* as,
                                map<cphvb_constant*,cphvb_intp>* cs,                                                                                
                                jit_io_instruction_list_map_lists* il_map,
                                vector<cphvb_instruction*>* is,                                                                    
                                bool create_computation_string,
                                bool create_instruction_list) {
    bool cloglevel[] = {0,0,0};    
    jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);
    
    il_map->output_array_map->push_back(new jit_instruction_list_coord(entr->instr_num,entr->operand_num));            
    oas->insert(pair<cphvb_array*,cphvb_intp>(entr->arrayp,0));
    
    il_map->array_map->push_back(new jit_instruction_list_coord(entr->instr_num,entr->operand_num));            
    as->insert(pair<cphvb_array*,cphvb_intp>(entr->arrayp,0));
    
    logcustom(cloglevel,1,"EETNC adding %p, %d t: %s\n",entr->arrayp, 0, cphvb_type_text(entr->arrayp->type));
    stringstream ss;
    //ss << "*(" << jitcg_nameing_array_offset("",0) << "+"<< jitcg_nameing_array_datapointer("",0) << ") = ";
    ss << jitcg_build_array_compute_component(0) << " = ";

    cphvb_index output_count = 1;
    ss << _expr_extract_traverser4(s,expr,as,cs,il_map,is,output_count,create_computation_string,create_instruction_list);    
    entr->is_executed = true;
    return ss.str();
}



/**
 * Does not set the outputs for the IL map, or fill in the output map.
 **/
string _expr_extract_traverser3(jit_analyse_state* s,jit_expr* expr,
                                        map<cphvb_array*,cphvb_intp>* as,
                                        map<cphvb_constant*,cphvb_intp>* cs,                                                                                
                                        jit_io_instruction_list_map_lists* il_map,
                                        vector<cphvb_instruction*>* is,                                        
                                        bool create_computation_string,
                                        bool create_instruction_list) {

    bool cloglevel[] = {0,0,0};    
    logcustom(cloglevel,0,"EET3 s _expr_operand_extract_traverser3()\n");
    
    stringstream ss;
    
    jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);
    logcustom(cloglevel,2,"EET3 %ld .. executed= %s\n",entr->expr->name, entr->is_executed?"True":"False");
    if (is_constant(expr)) {
        cphvb_intp ic = -1;

        //printf("=========== %s - %p - ",cs->find(expr->op.constant) == cs->end()?"NoTIn":"In",expr->op.constant);
        //constant_value_text_s(expr->op.constant);
        //printf("\n");
        
        if(cs->find(expr->op.constant) == cs->end()) {            
            ic = cs->size();
            logcustom(cloglevel,1,"EET3 insert into CS: %d - %p\n",cs->size(), expr->op.constant);            
            cs->insert(pair<cphvb_constant*,cphvb_intp>(expr->op.constant,ic));
            logcustom(cloglevel,1,"EET3 %p - %ld / %ld %d\n",entr,entr->instr_num,entr->operand_num,cs->size());
            
            jit_name_entry* tentr = jita_nametable_lookup(s->nametable,expr->parent->name);
            if (tentr->instr->operand[1] == NULL) {
                 il_map->constant_map->push_back(new jit_instruction_list_coord(tentr->instr_num,1));
            } else { //tentr->instr->operand[2] == NULL
                il_map->constant_map->push_back(new jit_instruction_list_coord(tentr->instr_num,2));
            }                
        } else {            
            ic = cs->find(expr->op.constant)->second;           
        }                
        ss << " *((" << cphvb_type_typetext(expr->op.constant->type)  << "*)" << " &cs[" << ic << "]->value) ";
        
        
    } else if (is_array(expr) || entr->is_executed) {        
        cphvb_intp ia = -1;        
        cphvb_array* arr = is_array(expr)? expr->op.array : entr->arrayp;
        // find or add to array list    
        if(as->find(arr) == as->end()) {                    
            ia = as->size();                        
            as->insert(pair<cphvb_array*,cphvb_intp>(arr,ia));

            // create IL map entry.                                    
            il_map->array_map->push_back(new jit_instruction_list_coord(entr->instr_num,entr->operand_num));                            
        } else {            
            ia = as->find(arr)->second;            
        }
            
        // create computation expression
        if (create_computation_string) {
            if (is_array(expr)) {
                ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(expr->op.array->type) << "*) ds[" << ia << "]->data))";                                            
            } else { // if it is a executed entr                                                
                ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(entr->arrayp->type) << "*) ds[" << ia << "]->data))";
            }            
        }       
        
    } else if(is_bin_op(expr)) {
        if (create_instruction_list) {
            is->push_back(entr->instr);
        } 
        string s1 = _expr_extract_traverser3(s,expr->op.expression.left,as,cs,il_map,is,create_computation_string,create_instruction_list);
        string s2 = _expr_extract_traverser3(s,expr->op.expression.right,as,cs,il_map,is,create_computation_string,create_instruction_list);
        logcustom(cloglevel,2,"EET3 binary, s1,s2 created\n");
        if (create_computation_string) {            
            return build_expression_string(expr->op.expression.opcode,s1,s2);
        }        
        
    } else if (is_un_op(expr)) {
        if (create_instruction_list) {
            is->push_back(entr->instr);
        }
        string s1 = _expr_extract_traverser3(s,expr->op.expression.left,as,cs,il_map,is,create_computation_string,create_instruction_list);
        logcustom(cloglevel,2,"EET3 unary, s1 created\n");
        if (create_computation_string) {            
            return build_expression_string(expr->op.expression.opcode,s1,"NONE");        
        }        
    }
    
    logcustom(cloglevel,0,"EET3 e jitcg_expr_travers_array() -- %s\n",ss.str().c_str());
    return ss.str();  
}



/**
 * 
 **/
string expr_extract_traverser(jit_analyse_state* s,jit_expr* expr,
                                map<cphvb_array*,cphvb_intp>* oas,
                                map<cphvb_array*,cphvb_intp>* as,
                                map<cphvb_constant*,cphvb_intp>* cs,                                                                                
                                jit_io_instruction_list_map_lists* il_map,
                                vector<cphvb_instruction*>* is,                                                                    
                                bool create_computation_string,
                                bool create_instruction_list) {

    jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);    
    il_map->output_array_map->push_back(new jit_instruction_list_coord(entr->instr_num,entr->operand_num));            
    oas->insert(pair<cphvb_array*,cphvb_intp>(entr->arrayp,0));

    il_map->array_map->push_back(new jit_instruction_list_coord(entr->instr_num,entr->operand_num)); 
    as->insert(pair<cphvb_array*,cphvb_intp>(entr->arrayp,0));    

    stringstream ss;
    ss << "*(((" << cphvb_type_typetext(entr->arrayp->type) << "*) ds[0]->data) + offs[0]) = ";
    ss << _expr_extract_traverser3(s,expr,as,cs,il_map,is,create_computation_string,create_instruction_list);    
    entr->is_executed = true;
    return ss.str();
}

void build_execution_kernel(jit_kernel* kernel,
                                cphvb_index oas_length,
                                cphvb_index as_length,
                                cphvb_index cs_length,
                                jit_execute_kernel* execute_kernel_out) {
    bool cloglevel[] = {0,0,0}; 
    
    execute_kernel_out->kernel = kernel;
    
    // create array output-array
    execute_kernel_out->outputarrays = ((cphvb_array**) malloc(sizeof(cphvb_array*)*oas_length));
    execute_kernel_out->outputarrays_length = oas_length;

    // create array input-array
    execute_kernel_out->arrays = ((cphvb_array**) malloc(sizeof(cphvb_array*)*(as_length)));
    execute_kernel_out->arrays_length = as_length;

    // create constants input-array
    execute_kernel_out->inputconstants_length = cs_length;
    if (cs_length > 0) {        
        execute_kernel_out->inputconstants = (cphvb_constant**) malloc(sizeof(cphvb_constant*)*cs_length);
    } else {
        execute_kernel_out->inputconstants = NULL;
    }

    logcustom(cloglevel,1,"%p %p\n",execute_kernel_out->arrays,execute_kernel_out->inputconstants);
}


cphvb_intp build_expr_kernel_out_input(jit_analyse_state* s, jit_name_entry* entr, cphvb_intp hash, cphvb_index id,jit_execute_kernel* execute_kernel_out,
map<cphvb_array*,cphvb_intp>* as,
map<cphvb_array*,cphvb_intp>* oas,
map<cphvb_constant*,cphvb_intp>* cs) {
    bool cloglevel[] = {0,0,0};    
    logcustom(cloglevel,0,"BEK build_expr_kernel() , userfunction %s\n", jit_pprint_true_false(entr->is_userfunction).c_str());
    // check that the expr is not a value.
    if ((is_array(entr->expr) || is_constant(entr->expr))) {
        return 1;
    }
    
    // setup travers datastructures        
    vector<cphvb_instruction*>* is  = new vector<cphvb_instruction*>();

    jit_io_instruction_list_map_lists* instr_list_map = (jit_io_instruction_list_map_lists*) malloc(sizeof(jit_io_instruction_list_map_lists));
    instr_list_map->array_map = new vector< jit_instruction_list_coord* >();
    instr_list_map->output_array_map = new vector< jit_instruction_list_coord* >();
    instr_list_map->constant_map = new vector< jit_instruction_list_coord* >();
    
    // Travers the expression and fill as and cs.
    if (entr->is_userfunction) {
        //build_expr_il_map_userfunc(s,entr,instr_list_map,is);                                                

        jit_name_entry* tentr;
        int i = 0;
        for(i=0;i<entr->span;i++) {            
            //oas->push_back();
            tentr = jita_nametable_lookup(s->nametable,entr->expr->name+i);            
            as->insert(pair<cphvb_array*,cphvb_intp>(tentr->arrayp,i));
        }
        
        for(uint i=0;i<entr->expr->userfunction_inputs->size();i++) {
            
            tentr = jita_nametable_lookup(s->nametable,entr->expr->userfunction_inputs->at(i)->name);                        
            as->insert(pair<cphvb_array*,cphvb_intp>(tentr->arrayp,i));
            
            //il_map->array_map->push_back(new jit_instruction_list_coord(tentr->instr_num,tentr->operand_num));
                               
            logcustom(cloglevel,1,"BEIMU I (%d %d) \n",tentr->instr_num,tentr->operand_num);
        }

        is->push_back(entr->instr);
        
        logcustom(cloglevel,1,"BEK instruction userfunc: %ld %ld \n",entr->instr->userfunc->nin,entr->instr->userfunc->nout);
    } else {
        build_expr_il_map(s,entr,instr_list_map,is);
        logcustom(cloglevel,1,"BEK instruction: %ld %ld \n",instr_list_map->array_map->size(),instr_list_map->constant_map->size());
        //string computestring = expr_extract_traverser(s,entr->expr,oas,as,cs,instr_list_map,is,false,true);

        if(cloglevel[2]) {
            printf("BEK -------- \n");
            jit_pprint_il_map2(instr_list_map);
            printf("BEK -------- \n");
        }
    }

    //jit_pprint_nametable_entry(entr);
    //jit_pprint_nametable(s->nametable);  
    
    // create kernel    
    jit_kernel* kernel = (jit_kernel*) malloc(sizeof(jit_kernel));    
    kernel->type = JIT_EXPR_KERNEL;
    kernel->il_map = instr_list_map;    
    kernel->hash = 1;
    kernel->id = id;

    
    kernel->expr_kernel = (jit_expr_kernel*) malloc(sizeof(jit_expr_kernel));
    kernel->expr_kernel->instructions = (cphvb_instruction*) malloc(sizeof(cphvb_instruction)*is->size());
    kernel->expr_kernel->expr = entr->expr;    
    kernel->expr_kernel->instructions_length = is->size();
    kernel->expr_kernel->instruction_num = entr->instr_num;
    kernel->expr_kernel->is_userfunc = entr->is_userfunction;
    
    // make copy of the instruction (can be viewed as the an instruction kernel).
    // Could insteed store an instr number and call the existing instruction.
    for(uint i=0; i<is->size(); i++) {
        instruction_copy_to(is->at(i),&kernel->expr_kernel->instructions[i]);
    }    
    //kernel->expr_kernel->instructions[0]->
    
    logcustom(cloglevel,1,"instruction userfunc: is.size()=%d %p\n",is->size(),kernel->expr_kernel->instructions[0].userfunc);
    
    build_execution_kernel(kernel,
                            instr_list_map->output_array_map->size(),
                            instr_list_map->array_map->size(),
                            instr_list_map->constant_map->size(),
                            execute_kernel_out);
    logcustom(cloglevel,1,"E- oas:%d   as:%d \n",oas->size(), as->size());

    return 0;
}

/**
 * Future funcitonality: Handle multi node expressions.
 * Can only handle "instruction" type expressions. 
 **/
cphvb_intp build_expr_kernel(jit_analyse_state* s, jit_name_entry* entr, cphvb_intp hash, cphvb_index id,jit_execute_kernel* execute_kernel_out) {
    bool cloglevel[] = {0,0,0};    
    logcustom(cloglevel,0,"BEK build_expr_kernel() , userfunction %s\n", jit_pprint_true_false(entr->is_userfunction).c_str());
    // check that the expr is not a value.
    if ((is_array(entr->expr) || is_constant(entr->expr))) {
        return 1;
    }
    
    // setup travers datastructures    
    //map<cphvb_array*,cphvb_intp>* oas = new map<cphvb_array*,cphvb_intp>();
    //map<cphvb_array*,cphvb_intp>* as  = new map<cphvb_array*,cphvb_intp>();
    //map<cphvb_constant*,cphvb_intp>* cs  = new map<cphvb_constant*,cphvb_intp>();   
    vector<cphvb_instruction*>* is  = new vector<cphvb_instruction*>();

    jit_io_instruction_list_map_lists* instr_list_map = (jit_io_instruction_list_map_lists*) malloc(sizeof(jit_io_instruction_list_map_lists));
    instr_list_map->array_map = new vector< jit_instruction_list_coord* >();
    instr_list_map->output_array_map = new vector< jit_instruction_list_coord* >();
    instr_list_map->constant_map = new vector< jit_instruction_list_coord* >();
    
    // Travers the expression and fill as and cs.
    if (entr->is_userfunction) {
        build_expr_il_map_userfunc(s,entr,instr_list_map,is);
        logcustom(cloglevel,1,"BEK instruction userfunc: %ld %ld \n",entr->instr->userfunc->nin,entr->instr->userfunc->nout);
    } else {
        build_expr_il_map(s,entr,instr_list_map,is);
        logcustom(cloglevel,1,"BEK instruction: %ld %ld \n",instr_list_map->array_map->size(),instr_list_map->constant_map->size());
        //string computestring = expr_extract_traverser(s,entr->expr,oas,as,cs,instr_list_map,is,false,true);

        if(cloglevel[2]) {
            printf("BEK -------- \n");
            jit_pprint_il_map2(instr_list_map);
            printf("BEK -------- \n");
        }
    }

    //jit_pprint_nametable_entry(entr);
    //jit_pprint_nametable(s->nametable);  
    
    // create kernel    
    jit_kernel* kernel = (jit_kernel*) malloc(sizeof(jit_kernel));    
    kernel->type = JIT_EXPR_KERNEL;
    kernel->il_map = instr_list_map;    
    kernel->hash = hash;
    kernel->id = id;

    kernel->expr_kernel = (jit_expr_kernel*) malloc(sizeof(jit_expr_kernel));
    kernel->expr_kernel->instructions = (cphvb_instruction*) malloc(sizeof(cphvb_instruction)*is->size());
    kernel->expr_kernel->expr = entr->expr;    
    kernel->expr_kernel->instructions_length = is->size();
    kernel->expr_kernel->instruction_num = entr->instr_num;
    kernel->expr_kernel->is_userfunc = entr->is_userfunction;
    // make copy of the instruction (can be viewed as the an instruction kernel).
    // Could insteed store an instr number and call the existing instruction.
    for(uint i=0; i<is->size(); i++) {
        instruction_copy_to(is->at(i),&kernel->expr_kernel->instructions[i]);
    }    
    //kernel->expr_kernel->instructions[0]->
    
    logcustom(cloglevel,1,"instruction userfunc: is.size()=%d %p\n",is->size(),kernel->expr_kernel->instructions[0].userfunc);
    
    build_execution_kernel(kernel,
                            instr_list_map->output_array_map->size(),
                            instr_list_map->array_map->size(),
                            instr_list_map->constant_map->size(),
                            execute_kernel_out);
    //logcustom(cloglevel,1,"E- oas:%d   as:%d \n",oas->size(), as->size());

    return 0;
}



/**
 * 
 **/
cphvb_intp build_compile_kernel_out_input(jit_analyse_state* s, jit_name_entry* entr, cphvb_intp hash, cphvb_index id,jit_execute_kernel* execute_kernel_out,
    map<cphvb_array*,cphvb_intp>* as,
    map<cphvb_array*,cphvb_intp>* oas,
    map<cphvb_constant*,cphvb_intp>* cs) {
    bool cloglevel[] = {0,0,0,0};    
    logcustom(cloglevel,0,"build_compile_kernel(%ld %d %p)\n",hash,id,execute_kernel_out);
    logcustom(cloglevel,2,"FunctionTextType = %s, compiler = %s \n", jit_pprint_functiontext_creator(JITCG_FUNCTEXT).c_str(), jit_pprint_compile_method(JITC_COMPILE_METHOD).c_str());
    
    
    // check that the expr is not a value.
    if ((is_array(entr->expr) || is_constant(entr->expr))) {
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1");
        return 1;
    }    

    // travers the expression and fill as and cs.    
    //~ map<cphvb_intp,cphvb_intp>* as = new map<cphvb_intp,cphvb_intp>();
    //~ map<cphvb_intp,cphvb_intp>* oas = new map<cphvb_intp,cphvb_intp>();
    //~ map<cphvb_constant*,cphvb_intp>* cs = new map<cphvb_constant*,cphvb_intp>();
    

    jit_io_instruction_list_map_lists* instr_list_map = (jit_io_instruction_list_map_lists*) malloc(sizeof(jit_io_instruction_list_map_lists));
    instr_list_map->array_map = new vector< jit_instruction_list_coord* >();
    instr_list_map->output_array_map = new vector< jit_instruction_list_coord* >();
    instr_list_map->constant_map = new vector< jit_instruction_list_coord* >();

    // create kernel
    jit_kernel* kernel = (jit_kernel*) malloc(sizeof(jit_kernel));
    kernel->type    = JIT_COMPILE_KERNEL;    
    kernel->il_map  = instr_list_map;
    kernel->hash    = hash;    //expr_hash(entr->expr)
    kernel->id      = id;
    kernel->compute_kernel = NULL;

    char buff[30];
    sprintf(buff,"kernel_func_%ld_%ld",hash,id);
    string kernel_name = buff;

    logcustom(cloglevel,1,"test\n");

    // how it could be
    //      
    // expr_map* data_map = build_datamap(jit_analyse_state* s, jit_expr* expr);
    // string func_text   = jitcg_create_kernelfunctionstring(jit_analyse_state* s,jitcg_settings* cgsettings,jit_expr* expr); // create the complete functiontext.
    
    // Create compute-string and fill instruction-list map.
    // create kernel function string.
    string computestring;
    string func_text;

    logcustom(cloglevel,1,"creating kernel for %ld \n", hash);

    switch(JITCG_FUNCTEXT) {         
        case JITCGFT_NoCast:
            logcustom(cloglevel,1,"nocast\n");
            computestring = expr_extract_traverser_nocast(s,entr->expr,oas,as,cs,instr_list_map,NULL,true,false);            
            func_text = create_kernel_function_travers_nocast(kernel_name.c_str(),computestring,oas,as,cs);
            break;
        default: // JITCGFT_Vanilla
            computestring = expr_extract_traverser(s,entr->expr,oas,as,cs,instr_list_map,NULL,true,false);
            func_text = create_kernel_function_travers(kernel_name.c_str(),computestring,oas,as,cs);
            break;
    }

    
    //string computestring = expr_extract_traverser_nocast(s,entr->expr,oas,as,cs,instr_list_map,NULL,true,false);
    //string func_text = create_kernel_function_travers_nocast(kernel_name.c_str(),computestring,oas,as,cs);    
    //~ string computestring = expr_extract_traverser(s,entr->expr,oas,as,cs,instr_list_map,NULL,true,false);    
    //~ string func_text = create_kernel_function_travers(kernel_name.c_str(),computestring,oas,as,cs);

    if(cloglevel[3]) {
        printf("computation string: %s\n",func_text.c_str());
        //printf("computation string: %s\n",computestring.c_str());
    }

    kernel->compute_kernel = jitc_compile_computefunction(kernel_name, func_text, JITC_COMPILE_METHOD );
    execute_kernel_out->kernel = kernel;    
    //~ build_execution_kernel(kernel,
                            //~ instr_list_map->output_array_map->size(),
                            //~ instr_list_map->array_map->size(),
                            //~ instr_list_map->constant_map->size(),
                            //~ execute_kernel_out);
    build_execution_kernel(kernel,
                            oas->size(),
                            as->size(),
                            cs->size(),
                            execute_kernel_out);
    logcustom(cloglevel,1,"test %d %d %d\n",oas->size(),as->size(),cs->size());
    logcustom(cloglevel,2,"setting computation string\n");
    logcustom(cloglevel,2,"computation string: %s\n",computestring.c_str());

    
    return 0;
}

/**
 * 
 **/
cphvb_intp build_compile_kernel(jit_analyse_state* s, jit_name_entry* entr, cphvb_intp hash, cphvb_index id,jit_execute_kernel* execute_kernel_out) {
    bool cloglevel[] = {0,0,0};    
    logcustom(cloglevel,0,"build_compile_kernel(%ld %d %p)\n",hash,id,execute_kernel_out);
    logcustom(cloglevel,2,"FunctionTextType = %s, compiler = %s \n", jit_pprint_functiontext_creator(JITCG_FUNCTEXT).c_str(), jit_pprint_compile_method(JITC_COMPILE_METHOD).c_str());
    
    
    // check that the expr is not a value.
    if ((is_array(entr->expr) || is_constant(entr->expr))) {
        return 1;
    }    

    // travers the expression and fill as and cs.    
    //~ map<cphvb_intp,cphvb_intp>* as = new map<cphvb_intp,cphvb_intp>();
    //~ map<cphvb_intp,cphvb_intp>* oas = new map<cphvb_intp,cphvb_intp>();
    //~ map<cphvb_constant*,cphvb_intp>* cs = new map<cphvb_constant*,cphvb_intp>();

    map<cphvb_array*,cphvb_intp>* as = new map<cphvb_array*,cphvb_intp>();
    map<cphvb_array*,cphvb_intp>* oas = new map<cphvb_array*,cphvb_intp>();
    map<cphvb_constant*,cphvb_intp>* cs = new map<cphvb_constant*,cphvb_intp>();

    jit_io_instruction_list_map_lists* instr_list_map = (jit_io_instruction_list_map_lists*) malloc(sizeof(jit_io_instruction_list_map_lists));
    instr_list_map->array_map = new vector< jit_instruction_list_coord* >();
    instr_list_map->output_array_map = new vector< jit_instruction_list_coord* >();
    instr_list_map->constant_map = new vector< jit_instruction_list_coord* >();

    // create kernel
    jit_kernel* kernel = (jit_kernel*) malloc(sizeof(jit_kernel));
    kernel->type    = JIT_COMPILE_KERNEL;    
    kernel->il_map  = instr_list_map;
    kernel->hash    = 1;    //expr_hash(entr->expr)
    kernel->id      = id;
    kernel->compute_kernel = NULL;

    char buff[30];
    sprintf(buff,"kernel_func_%ld_%ld",hash,id);
    string kernel_name = buff;

    logcustom(cloglevel,1,"test\n");

    // how it could be
    //      
    // expr_map* data_map = build_datamap(jit_analyse_state* s, jit_expr* expr);
    // string func_text   = jitcg_create_kernelfunctionstring(jit_analyse_state* s,jitcg_settings* cgsettings,jit_expr* expr); // create the complete functiontext.
    
    // Create compute-string and fill instruction-list map.
    // create kernel function string.
    string computestring;
    string func_text;

    switch(JITCG_FUNCTEXT) {         
        case JITCGFT_NoCast:
            logcustom(cloglevel,1,"nocast\n");
            computestring = expr_extract_traverser_nocast(s,entr->expr,oas,as,cs,instr_list_map,NULL,true,false);
            func_text = create_kernel_function_travers_nocast(kernel_name.c_str(),computestring,oas,as,cs);
            break;
        default: // JITCGFT_Vanilla
            computestring = expr_extract_traverser(s,entr->expr,oas,as,cs,instr_list_map,NULL,true,false);
            func_text = create_kernel_function_travers(kernel_name.c_str(),computestring,oas,as,cs);
            break;
    }

    
    //string computestring = expr_extract_traverser_nocast(s,entr->expr,oas,as,cs,instr_list_map,NULL,true,false);
    //string func_text = create_kernel_function_travers_nocast(kernel_name.c_str(),computestring,oas,as,cs);    
    //~ string computestring = expr_extract_traverser(s,entr->expr,oas,as,cs,instr_list_map,NULL,true,false);    
    //~ string func_text = create_kernel_function_travers(kernel_name.c_str(),computestring,oas,as,cs);

    if(K_PRINT_COMPUTESTRING) {
        //printf("computation string: %s\n",func_text.c_str());
        printf("computation string: %s\n",computestring.c_str());
    }

    kernel->compute_kernel = jitc_compile_computefunction(kernel_name, func_text, JITC_COMPILE_METHOD );
    execute_kernel_out->kernel = kernel;    
    build_execution_kernel(kernel,
                            instr_list_map->output_array_map->size(),
                            instr_list_map->array_map->size(),
                            instr_list_map->constant_map->size(),
                            execute_kernel_out);
    logcustom(cloglevel,1,"test 5\n");
    logcustom(cloglevel,1,"setting computation string\n");
    logcustom(cloglevel,1,"computation string: %s\n",computestring.c_str());

    
    return 0;
}

cphvb_error execute_from_executionlist(jit_analyse_state* s, jit_compute_functions* compute_functions,
                                    jit_expression_kernel_cache* e_kernel_cache,                                    
                                    set<cphvb_intp>* execution_list,
                                    bool cache_enabled) {
    bool cloglevel[] = {0,0,0,0};
    logcustom(cloglevel,0,"EFE execute_from_executionlist( executionlist length: %d)\n",execution_list->size());
    timespec time1, time2;
    // set id of the compund kernel.            
    jit_name_entry* entr;
    set<cphvb_intp>::iterator it;
    cphvb_intp exprhash = 0;
    int kernel_count = 0;
    bool from_cache = false;
    cphvb_error res;

    map<cphvb_array*,cphvb_intp>* as = new map<cphvb_array*,cphvb_intp>();
    map<cphvb_array*,cphvb_intp>* oas = new map<cphvb_array*,cphvb_intp>(); 
    map<cphvb_constant*,cphvb_intp>* cs = new map<cphvb_constant*,cphvb_intp>();

    if(cloglevel[2]) {
        printf("===================================\n");        
        jit_pprint_nametable(s->nametable);
        jit_pprint_nametable_dependencies(s->nametable);
        printf("executionlist");jit_pprint_set(execution_list);
        jit_pprint_base_dependency_table(s->base_usage_table);                        
    }
    
    for(it=execution_list->begin();it!=execution_list->end();it++) {    
        as->clear(); 
        oas->clear();
        cs->clear(); 
          
        entr = jita_nametable_lookup(s->nametable,*it);
        if (cloglevel[2]) {  
            printf("=================================== %ld \n",entr->expr->name);  
            jit_pprint_nametable_entry(entr);
            printf("===================================\n");
        }
        
        cphvb_intp execute_result = 1;
        if(entr->is_userfunction) {
            if (cloglevel[3]) { clock_gettime(CLOCK_REALTIME, &time1); }             
            execute_result = execute_userfunction(compute_functions,entr->instr);
            if (cloglevel[3]) {                
                clock_gettime(CLOCK_REALTIME, &time2);  
                printf("* execute cached expression_kernel %ld : %d\n",diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec)); 
            }
            
        } else {
            logcustom(cloglevel,1,"EFE name:%d  depth: %ld\n",*it, entr->expr->depth);
            exprhash = expr_hash(entr->expr);
            
            // == check expression kernel level cache
            jit_execute_kernel* execute_kernel = NULL;
            if (cache_enabled) {
                execute_kernel = jit_expression_kernel_cache_lookup(e_kernel_cache,exprhash);
            }
            
            from_cache = (execute_kernel != NULL);

            if(from_cache) { logcustom(cloglevel,1,"Got from cache: %ld  hash: %ld \n",*it, execute_kernel->kernel->hash);  }
            
            if(!from_cache) {
                execute_kernel = (jit_execute_kernel*) malloc(sizeof(jit_execute_kernel));
                if (cloglevel[3]) { clock_gettime(CLOCK_REALTIME, &time1); }   

                logcustom(cloglevel,1,"EFE %d build_compile_kernel \n",*it);
                //jit_pprint_nametable(s->nametable);
                build_compile_kernel_out_input(s,entr,exprhash,kernel_count,execute_kernel,as,oas,cs);
                
                if (cloglevel[3]) {                
                    clock_gettime(CLOCK_REALTIME, &time2);  
                    printf("build expression_kernel %ld : %d\n",diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec)); 
                }
                
            } else {
                // travers expr and bind it to the input for the cached kernel.
                if (cloglevel[3]) { clock_gettime(CLOCK_REALTIME, &time1); }
                  
                logcustom(cloglevel,1,"EFE execute_kernel from cache.\n");
                
                logcustom(cloglevel,1,"compile. Fill by travers\n");
                expr_travers_distinct(s,entr,as,cs);

                
                
                if (cloglevel[3]) {                
                    clock_gettime(CLOCK_REALTIME, &time2);  
                    printf("- extract inputs to expression_kernel %ld : %d\n",diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec)); 
                }
            }
            if (execute_kernel == NULL) {
                fprintf(stderr,"EFE Something is not right in execute_from_executionlist. Failed to build non cache entry\n");        
            }
            
            logcustom(cloglevel,1,"EFE - execution_kernel = %p, as=%d cs=%d\n",execute_kernel,as->size(),cs->size());

            //jit_pprint_execute_kernel(execute_kernel) ;
            // bind
            if (cloglevel[3]) { clock_gettime(CLOCK_REALTIME, &time1); }   
            bind_execution_kernel_to_expression(as,cs,execute_kernel);                 
            if (cloglevel[3]) {                
                clock_gettime(CLOCK_REALTIME, &time2);  
                printf("- bind cached expression_kernel %ld : %d\n",diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec)); 
            }
            // allocate
            res = allocate_for_exekernel(execute_kernel);
                logcustom(cloglevel,1,"allocation result= %d\n",res);
                
            if (res != CPHVB_SUCCESS) {
                return res;
            }

            // execute compiled kernel
            logcustom(cloglevel,0,"EFE execute: JIT_COMPILE_KERNEL - exekernel: \n");
            if (cloglevel[3]) { clock_gettime(CLOCK_REALTIME, &time1); }
            //jit_pprint_execute_kernel(execute_kernel) ;
            execute_result = execute_kernel_compiled(execute_kernel);
            if (cloglevel[3]) {                
                clock_gettime(CLOCK_REALTIME, &time2);  
                printf("- execute expression_kernel %d %ld : %d\n",execute_kernel->kernel->hash,diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec)); 
            }

            // insert into cache
            if(execute_result == 0 && !from_cache && cache_enabled) {
                if (!jit_expression_kernel_cache_insert(e_kernel_cache,exprhash,execute_kernel)) {
                    logcustom(cloglevel,1,"failed to insert kernels H: %ld into cache\n",exprhash);                
                }
            }                                
        }
        kernel_count++;                 
    }
    
    return CPHVB_SUCCESS;
}

cphvb_error execute_from_executionlist0(jit_analyse_state* s, jit_compute_functions* compute_functions,
                                    jit_expression_kernel_cache* e_kernel_cache,                                    
                                    set<cphvb_intp>* execution_list) {
    bool cloglevel[] = {0,0,0,0};
    logcustom(cloglevel,0,"EFE execute_from_executionlist( executionlist length: %d)\n",execution_list->size());
    timespec time1, time2;
    // set id of the compund kernel.            
    jit_name_entry* entr;
    set<cphvb_intp>::iterator it;
    cphvb_intp exprhash = 0;
    int kernel_count = 0;
    bool from_cache = false;
    cphvb_error res;

    map<cphvb_array*,cphvb_intp>* as = new map<cphvb_array*,cphvb_intp>();
    map<cphvb_array*,cphvb_intp>* oas = new map<cphvb_array*,cphvb_intp>(); 
    map<cphvb_constant*,cphvb_intp>* cs = new map<cphvb_constant*,cphvb_intp>();
    
    for(it=execution_list->begin();it!=execution_list->end();it++) {
        as->clear(); 
        oas->clear();
        cs->clear(); 
        
        entr = jita_nametable_lookup(s->nametable,*it);
        logcustom(cloglevel,1,"EFE name:%d  depth: %ld\n",*it, entr->expr->depth);
        exprhash = expr_hash(entr->expr);

        // == check expression kernel level cache
        jit_execute_kernel* execute_kernel = jit_expression_kernel_cache_lookup(e_kernel_cache,exprhash);
        //jit_execute_kernel* execute_kernel;
        from_cache = (execute_kernel != NULL);
        //~ if (execute_kernel != NULL) {
            //~ jit_pprint_execute_kernel(execute_kernel);
            //~ printf("operand 0: %p \n",execute_kernel->kernel->expr_kernel->instructions[0].userfunc->operand[0]);
            //~ printf("operand 1: %p \n",execute_kernel->kernel->expr_kernel->instructions[0].userfunc->operand[1]);
            //~ printf("id: %ld \n",execute_kernel->kernel->expr_kernel->instructions[0].userfunc->id);
            //~ printf("nout: %ld \n",execute_kernel->kernel->expr_kernel->instructions[0].userfunc->nout);
            //~ printf("nin: %ld \n",execute_kernel->kernel->expr_kernel->instructions[0].userfunc->nin);
            //~ printf("struct_size: %ld \n",execute_kernel->kernel->expr_kernel->instructions[0].userfunc->struct_size);
//~ 
            //~ printf("operand 0: %p \n",entr->instr->userfunc->operand[0]);
            //~ printf("operand 1: %p \n",entr->instr->userfunc->operand[1]);
            //~ printf("id: %ld \n",entr->instr->userfunc->id);
            //~ printf("nout: %ld \n",entr->instr->userfunc->nout);
            //~ printf("nin: %ld \n",entr->instr->userfunc->nin);
            //~ printf("struct_size: %ld \n",entr->instr->userfunc->struct_size);
        //~ }
        
        if(!from_cache) {
            logcustom(cloglevel,1,"EFE Building execute_kernel\n");
            // Build it
            execute_kernel = (jit_execute_kernel*) malloc(sizeof(jit_execute_kernel));
            
            if(entr->is_userfunction) {
                if (cloglevel[3]) { clock_gettime(CLOCK_REALTIME, &time1); }
                
                logcustom(cloglevel,1,"EFE %d build_userfunc_kernel \n",*it);
                                
                //build_expr_kernel_out_input(s,entr,exprhash,kernel_count,execute_kernel,as,oas,cs);

                
                jit_name_entry* tentr;
                int i = 0;
                for(i=0;i<entr->span;i++) {                                
                    tentr = jita_nametable_lookup(s->nametable,entr->expr->name+i);            
                    as->insert(pair<cphvb_array*,cphvb_intp>(tentr->arrayp,i));
                    oas->insert(pair<cphvb_array*,cphvb_intp>(tentr->arrayp,i));
                    
                    logcustom(cloglevel,1,"================ BEIMU O (%d %p) \n",i,tentr->arrayp);
                }
                
                for(uint i=0;i<entr->expr->userfunction_inputs->size();i++) {                    
                    tentr = jita_nametable_lookup(s->nametable,entr->expr->userfunction_inputs->at(i)->name);                        
                    as->insert(pair<cphvb_array*,cphvb_intp>(tentr->arrayp,i+entr->span));
                                       
                    logcustom(cloglevel,1,"================ BEIMU I (%d %p) \n",i+entr->span,tentr->arrayp);
                }
                //is->push_back(entr->instr);            
                entr->is_executed = true;

                jit_kernel* kernel = (jit_kernel*) malloc(sizeof(jit_kernel));    
                kernel->type = JIT_EXPR_KERNEL;
                kernel->il_map = NULL;    
                kernel->hash = exprhash;
                kernel->id = kernel_count;
                
                kernel->expr_kernel = (jit_expr_kernel*) malloc(sizeof(jit_expr_kernel));                            
                kernel->expr_kernel->instructions = (cphvb_instruction*) malloc(sizeof(cphvb_instruction));
                //kernel->expr_kernel->instructions[0] =
                
                
                
                kernel->expr_kernel->expr = entr->expr;    
                kernel->expr_kernel->instructions_length = 1;
                kernel->expr_kernel->instruction_num = entr->instr_num;
                kernel->expr_kernel->is_userfunc = entr->is_userfunction;
                
                // make copy of the instruction (can be viewed as the an instruction kernel).
                // Could insteed store an instr number and call the existing instruction.                
                



                cphvb_instruction *instr1 = (cphvb_instruction*) malloc(sizeof(cphvb_instruction));
                instr1->status = entr->status;
                instr1->userfunc = (cphvb_userfunc*) malloc(entr->instr->userfunc->struct_size);             
                instr1->userfunc->operand[0] = entr->instr->userfunc->operand[0];
                instr1->userfunc->operand[1] = entr->instr->userfunc->operand[1];
                instr1->userfunc->id = entr->instr->userfunc->id;
                instr1->userfunc->nout = entr->instr->userfunc->nout;
                instr1->userfunc->nin = entr->instr->userfunc->nin;
                instr1->userfunc->struct_size = entr->instr->userfunc->struct_size;
                
                
                kernel->expr_kernel->instruction = *instr1;
                //kernel->expr_kernel->instructions[0] = &(*instr1);
                //printf("--------- %p %p %p %p \n",instr1, kernel->expr_kernel->instruction, kernel->expr_kernel->instructions);
                //instruction_copy_to(entr->instr,&kernel->expr_kernel->instructions[0]);
                
                build_execution_kernel(kernel,1,as->size(),cs->size(),execute_kernel);
                if (cloglevel[3]) {                
                    clock_gettime(CLOCK_REALTIME, &time2);  
                    printf("build userfunc_kernel %ld : %d\n",diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec)); 
                }
                        
            } else {
                // handle as kernel
                if (cloglevel[3]) { clock_gettime(CLOCK_REALTIME, &time1); }   

                logcustom(cloglevel,1,"EFE %d build_compile_kernel \n",*it);
                build_compile_kernel_out_input(s,entr,exprhash,kernel_count,execute_kernel,as,oas,cs);
                
                if (cloglevel[3]) {                
                    clock_gettime(CLOCK_REALTIME, &time2);  
                    printf("build expression_kernel %ld : %d\n",diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec)); 
                }
            }
        } else {
            if (cloglevel[3]) { clock_gettime(CLOCK_REALTIME, &time1); }   
            logcustom(cloglevel,1,"EFE execute_kernel from cache.\n");
            if (entr->is_userfunction) {
                logcustom(cloglevel,1,"userfunc. Fill by travers\n");
                        

                jit_name_entry* tentr;
                for(int i=0;i<entr->span;i++) {                                
                    tentr = jita_nametable_lookup(s->nametable,entr->expr->name+i);            
                    as->insert(pair<cphvb_array*,cphvb_intp>(tentr->arrayp,i));
                    oas->insert(pair<cphvb_array*,cphvb_intp>(tentr->arrayp,i));
                }
                
                for(uint i=0;i<entr->expr->userfunction_inputs->size();i++) {                    
                    tentr = jita_nametable_lookup(s->nametable,entr->expr->userfunction_inputs->at(i)->name);                        
                    as->insert(pair<cphvb_array*,cphvb_intp>(tentr->arrayp,i+entr->span));                                                       
                }

            } else {                
                logcustom(cloglevel,1,"compile. Fill by travers\n");
                expr_travers_distinct(s,entr,as,cs);
            }
            if (cloglevel[3]) {                
                clock_gettime(CLOCK_REALTIME, &time2);  
                printf("- bind expression_kernel %ld : %d\n",diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec)); 
            }
        }
           
        
        if(execute_kernel == NULL) {
            fprintf(stderr,"EFE Something is not right in execute_from_executionlist. Failed to build non cache entry\n");        
        }
        logcustom(cloglevel,1,"EFE - execution_kernel = %p, as=%d cs=%d\n",execute_kernel,as->size(),cs->size());

       
        
       
        // bind
        //jit_pprint_execute_kernel(execute_kernel);
        bind_execution_kernel_to_expression(as,cs,execute_kernel); 
               
        //jit_pprint_execute_kernel(execute_kernel);
                
        // allocate
        res = allocate_for_exekernel(execute_kernel);
        
        
        logcustom(cloglevel,1,"allocation result= %d\n",res);
        if (res != CPHVB_SUCCESS) {
            return res;
        }
        
        //~ printf("1 ============================\n");
        //~ printf("%p\n",execute_kernel->arrays[1]);
        //~ jit_pprint_cphvb_array(execute_kernel->arrays[1],0);
        //~ printf("1 ============================\n");
        // execute it
        int execute_result = 1;
        if(execute_kernel->kernel->type == JIT_EXPR_KERNEL) {
            logcustom(cloglevel,0,"EFE execute: JIT_EXPR_KERNEL - exekernel: \n");
            //execute_result = execute_kernel_expr(compute_functions,execute_kernel);

            cphvb_instruction* instr = &execute_kernel->kernel->expr_kernel->instruction;
            //~ printf("2 %p %p\n",instr,instr->userfunc);
            //~ printf("operand 0: %p \n",instr->userfunc->operand[0]);
            //~ printf("operand 1: %p \n",instr->userfunc->operand[1]);
            //~ printf("array: %p \n",execute_kernel->arrays[0]);
            //~ printf("array: %p %d \n",execute_kernel->arrays[1],execute_kernel->arrays_length);
            for(int i=0;i<execute_kernel->arrays_length;i++) {
                int index = i; //+exekernel->outputarrays_length;
                instr->userfunc->operand[index] = execute_kernel->arrays[i];            
            }
            
            //logcustom(cloglevel,2,"EKE Userfunc instruction bounded. %p\n",instr);

            
            //~ printf("1 %p %p\n",execute_kernel->kernel->expr_kernel->instructions[0],execute_kernel->kernel->expr_kernel->instructions[0].userfunc->operand[0]);
            //~ printf("operand 0: %p \n",execute_kernel->kernel->expr_kernel->instructions[0].userfunc->operand[0]);
            //~ printf("operand 1: %p \n",execute_kernel->kernel->expr_kernel->instructions[0].userfunc->operand[1]);
            //~ printf("id: %ld \n",execute_kernel->kernel->expr_kernel->instructions[0].userfunc->id);
            //~ printf("nout: %ld \n",execute_kernel->kernel->expr_kernel->instructions[0].userfunc->nout);
            //~ printf("nin: %ld \n",execute_kernel->kernel->expr_kernel->instructions[0].userfunc->nin);
            //~ printf("struct_size: %ld \n",execute_kernel->kernel->expr_kernel->instructions[0].userfunc->struct_size);
//~ 
//~ 
            //~ printf("2 %p %p\n",entr->instr,entr->instr->userfunc);
            //~ printf("operand 0: %p \n",entr->instr->userfunc->operand[0]);
            //~ printf("operand 1: %p \n",entr->instr->userfunc->operand[1]);
            //~ printf("id: %ld \n",entr->instr->userfunc->id);
            //~ printf("nout: %ld \n",entr->instr->userfunc->nout);
            //~ printf("nin: %ld \n",entr->instr->userfunc->nin);
            //~ printf("struct_size: %ld \n",entr->instr->userfunc->struct_size);
            //~ printf("struct_size: %s \n",cphvb_opcode_text(entr->instr->opcode));            
            
            
            //execute_result = execute_userfunction(compute_functions,entr->instr);
            execute_result = execute_userfunction(compute_functions,entr->instr);
        } else if(execute_kernel->kernel->type == JIT_COMPILE_KERNEL) {
            logcustom(cloglevel,0,"EFE execute: JIT_COMPILE_KERNEL - exekernel: \n");
            execute_result = execute_kernel_compiled(execute_kernel);
        } else {
            logcustom(cloglevel,0,"EFE execute: NONE - exekernel: %d\n");            
        }

        //~ printf("1 ============================\n");
        //~ printf("%p\n",execute_kernel->arrays[0]);
        //~ jit_pprint_cphvb_array(execute_kernel->arrays[0],0);
        //~ printf("1 ============================\n");
        
        if(execute_result == 0 && !from_cache) {
            if (!jit_expression_kernel_cache_insert(e_kernel_cache,exprhash,execute_kernel)) {
                logcustom(cloglevel,1,"failed to insert kernels H: %ld into cache\n",exprhash);
                //return CPHVB_ERROR;
            }
        }
        

        //printf("execution kernel for NT: %d - %s\n",*it,executekernel_type_to_string(execute_kernel));
        //jit_pprint_il_map(execute_kernel->kernel->il_map);
        //jit_pprint_execute_kernel(execute_kernel);
        
        //printf("%d %p %d\n",(kernel_count),compoundk_out->kernels[0], sizeof(compoundk_out->kernels));
                
        kernel_count++;
    }
    return CPHVB_SUCCESS;
}

/**
 * 
 **/
cphvb_intp build_compound_kernel(jit_analyse_state* s, set<cphvb_intp>* execution_list, cphvb_index compound_id, jit_compound_kernel* compoundk_out) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"BCK build_compound_kernel( executionlist length: %d,compound ID: %ld)\n",execution_list->size(),compound_id);
        
    // set id of the compund kernel.        
    compoundk_out->id = compound_id;    
    jit_name_entry* entr;
    set<cphvb_intp>::iterator it;
    cphvb_intp exprhash = 0;
    int kernel_count = 0;

    //jit_pprint_nametable(s->nametable); printf("\n");
    
    for(it=execution_list->begin();it!=execution_list->end();it++) {
        entr = jita_nametable_lookup(s->nametable,*it);
        logcustom(cloglevel,1,"BCK name:%d  depth: %ld\n",*it, entr->expr->depth);
        exprhash = expr_hash(entr->expr);
        
        jit_execute_kernel* execute_kernel = (jit_execute_kernel*) malloc(sizeof(jit_execute_kernel));
        jit_update_expr_depth(s,entr->expr);
    
        if(entr->is_userfunction) {
            logcustom(cloglevel,1,"BCK %d build_userfunc_kernel \n",*it);
            build_expr_kernel(s,entr,exprhash,kernel_count,execute_kernel);
            entr->is_executed = true;
        } else         
        if (entr->expr->depth == -2) { // create a kernel in this case aswell!
            // a single instruction as and expression.
            logcustom(cloglevel,1,"BCK %d build_expr_kernel \n",*it);
            build_expr_kernel(s,entr,exprhash,kernel_count,execute_kernel);
            entr->is_executed = true;            
            
        } else {
            // handle as kernel
            logcustom(cloglevel,1,"BCK %d build_compile_kernel \n",*it);
            build_compile_kernel(s,entr,exprhash,kernel_count,execute_kernel);
        }

        //printf("execution kernel for NT: %d - %s\n",*it,executekernel_type_to_string(execute_kernel));
        //jit_pprint_il_map(execute_kernel->kernel->il_map);
        //jit_pprint_execute_kernel(execute_kernel);
        
        //printf("%d %p %d\n",(kernel_count),compoundk_out->kernels[0], sizeof(compoundk_out->kernels));
        
        compoundk_out->exekernels[kernel_count] = execute_kernel;
        kernel_count++;
    }
    
    return 0;
}




