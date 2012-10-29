// jit_kernel.cpp


#include "cphvb.h"
#include "jit_kernel.h"
#include "jit_kernel_cache.h"
#include "jit_analyser.h"
#include "jit_expr.h"
#include "jit_common.h"
#include "jit_computing.h"

//void update_expr_from_kernel_state()


bool is_expr_kernel(jit_kernel* kernel) {
    return kernel->type == JIT_EXPR_KERNEL;
}
bool is_compile_kernel(jit_kernel* kernel) {
    return kernel->type == JIT_COMPILE_KERNEL;
}


const char* executekernel_type(jit_execute_kernel* exekernel) {
    switch(exekernel->kernel->type) {
        case JIT_EXPR_KERNEL:
            return "expr-kernel";
        case JIT_COMPILE_KERNEL:
            return "compile-kernel";
        default:
            return "NONE";        
    }
}

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
cphvb_intp bind_execution_kernel(jit_execute_kernel* exekernel, cphvb_instruction* instructionlist) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"BEK bind_execution_kernel()\n");
    cphvb_index i;
    cphvb_intp operand, instr;

    logcustom(cloglevel,1,"BEK array_lenght: %d\n",exekernel->kernel->il_map->array_map->size());
    //jit_pprint_il_map2(exekernel->kernel->il_map);        
    // input arrays
    for(i = 0; i < exekernel->kernel->il_map->array_map->size(); i++) {
        instr = exekernel->kernel->il_map->array_map->at(i)->first;
        operand = exekernel->kernel->il_map->array_map->at(i)->second;
        
        exekernel->inputarrays[i] = ilmap_fetch_instruction_array(&instructionlist[instr],operand);
        logcustom(cloglevel,1,"BEK inA: %p (%ld,%ld)\n",exekernel->inputarrays[i],instr,operand);
        //jit_pprint_array_values(exekernel->inputarrays[i]);
    }

    // input constants
    for(i = 0; i < exekernel->kernel->il_map->constant_map->size(); i++) {
        instr = exekernel->kernel->il_map->constant_map->at(i)->first;
        operand = exekernel->kernel->il_map->constant_map->at(i)->second;
        exekernel->inputconstants[i] = &instructionlist[instr].constant;
        logcustom(cloglevel,1,"BEK inC: %p\n",exekernel->inputconstants[i]);
    }

    // output arrays
    for(i = 0; i < exekernel->kernel->il_map->output_array_map->size(); i++) {
        instr = exekernel->kernel->il_map->output_array_map->at(i)->first;
        operand = exekernel->kernel->il_map->output_array_map->at(i)->second;        
        exekernel->outputarrays[i] = ilmap_fetch_instruction_array(&instructionlist[instr],operand);
        //exekernel->outputarrays[i] = instructionlist[instr].operand[operand];
        logcustom(cloglevel,1,"BEK outA: %p (%ld,%ld\n",exekernel->outputarrays[i],instr,operand);

    }
    return 0;

}

cphvb_intp execute_userfunction(jit_compute_functions* compute_functions,cphvb_instruction* instr) {
    bool cloglevel[] = {0,0,0};
    logcustom(cloglevel,0,"EXU \n");

    // Allocation of target arrays
    for(int i=0;i<instr->userfunc->nout ;i++) {
        if (cphvb_base_array(instr->userfunc->operand[i])->data == NULL) {
            logcustom(cloglevel,1,"EXU Allocate array for %p\n", instr->userfunc->operand[i]);
            cphvb_data_malloc(instr->userfunc->operand[i]);
        }
        logcustom(cloglevel,2,"OA %p (%ld)\n",instr->userfunc->operand[i],i);
    }    

    if (cloglevel[2]) {    
        for(int i=0;i<instr->userfunc->nin ;i++) {
            logcustom(cloglevel,2,"IA %p (%ld) \n",instr->userfunc->operand[i+instr->userfunc->nout],i+instr->userfunc->nout);
        }       
    }
    
    cphvb_userfunc_impl ufunc = compute_functions->userfunctions->find(instr->userfunc->id)->second;
    
    if (ufunc(instr->userfunc,NULL) == CPHVB_SUCCESS) {
        logcustom(cloglevel,1," compute_apply == SUCCESS\n", instr->operand[0]);
        return 0;
    }
    logcustom(cloglevel,1," compute_apply == ERROR\n", instr->operand[0]);
    return 1;
}

cphvb_intp execute_instruction(jit_compute_functions* compute_functions,cphvb_instruction* instr) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"EXE \n");
    // Allocation of target array
    if (cphvb_base_array(instr->operand[0])->data == NULL) {
        logcustom(cloglevel,1," allocating array for %p\n", instr->operand[0]);
        cphvb_data_malloc(instr->operand[0]);
    }

    
    if (compute_functions->instr_compute(instr) == CPHVB_SUCCESS) {
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
    // just a way to pass the arguments to the expr.  

    //~ if(is_userfunc(exekernel->kernel->expr_kernel->is_userfunc)) {
        //~ cphvb_intp res = execute_userfunction(compute_functions,instr);
        //~ logcustom(cloglevel,1,"EKE U res: %ld\n", res);        
    //~ } else {
    //~ }
//~ 

    //exekernel->kernel->il_map
    cphvb_instruction* instr = &exekernel->kernel->expr_kernel->instructions[0];
    logcustom(cloglevel,0,"EKE Instr %p\n",instr);
    if(exekernel->kernel->expr_kernel->is_userfunc) {
        int i = 0;
        logcustom(cloglevel,1,"userfunc operand[]: %d %d %p\n",instr->userfunc->nin,instr->userfunc->nout,instr->userfunc->operand[0]);
        for(i=0;i<exekernel->outputarrays_length;i++) {
            instr->userfunc->operand[i] = exekernel->outputarrays[i];
            logcustom(cloglevel,2,"EKE oInstr[%d]=%d, outputarrays[%d]=%p\n",i,exekernel->kernel->il_map->output_array_map->at(i)->second,i,exekernel->outputarrays[i]);
        }

        for(i=0;i<exekernel->inputarrays_length;i++) {
            int index = i+exekernel->outputarrays_length;
            instr->userfunc->operand[index] = exekernel->inputarrays[i];
            logcustom(cloglevel,2,"EKE iInstr[%d]=%d, outputarrays[%d]=%p\n",i,exekernel->kernel->il_map->array_map->at(i)->second,i,exekernel->inputarrays[i]);
        }        
        logcustom(cloglevel,2,"EKE Userfunc instruction bounded to IL\n");
        cphvb_intp res = execute_userfunction(compute_functions,instr);
        logcustom(cloglevel,2,"EKE U res: %ld\n", res);        

    } else {
        
        instr->operand[0] = exekernel->outputarrays[0];

        for(int i=0;i<exekernel->kernel->il_map->array_map->size();i++) {            
            instr->operand[exekernel->kernel->il_map->array_map->at(i)->second] = exekernel->inputarrays[i]; 
        }

        if(exekernel->kernel->il_map->constant_map->size() > 0) {
            cphvb_index i = exekernel->kernel->il_map->constant_map->at(0)->second;
            instr->operand[i] = NULL;
            instr->constant = *(exekernel->inputconstants[0]);

            logcustom(cloglevel,1,"###  %ld %p\n",i,instr->constant);
            //constant_value_text_s(exekernel->inputconstants[0]);
        }                    
        logcustom(cloglevel,1,"EKE instruction bounded to IL\n");  
        cphvb_intp res = execute_instruction(compute_functions,instr);
        logcustom(cloglevel,1,"EKE res: %ld\n", res);        
    }
        
    return 0;
}


/**
 * 
 **/
cphvb_intp execute_kernel_compiled(jit_execute_kernel* exekernel) {
    bool cloglevel[] = {0,0,0};
    logcustom(cloglevel,0,"////// executing compiled kernel!\n");
    
    computefunc func = exekernel->kernel->compute_kernel->function;

    if (cloglevel[2]) {jit_pprint_execute_kernel(exekernel);}
    logcustom(cloglevel,1,"O:%p \n",exekernel->outputarrays[0]);
    if (cloglevel[2]) {jit_pprint_cphvb_array(exekernel->outputarrays[0],0);}
    
    for(int i=0;i<exekernel->inputarrays_length;i++) {        
        logcustom(cloglevel,1,"A:%p \n",exekernel->inputarrays[i]);
        if (cloglevel[2]) {jit_pprint_cphvb_array(exekernel->inputarrays[i],0);}
    }    
    if (exekernel->inputconstants_length >0) {        
        logcustom(cloglevel,1,"C: %p\n",exekernel->inputconstants[0]);
    }
    //jit_pprint_cphvb_array(exekernel->inputarrays[1],0);
    logcustom(cloglevel,1,"AO: %p\n",exekernel->outputarrays[0]);
    func(exekernel->outputarrays[0],exekernel->inputarrays,exekernel->inputarrays_length,exekernel->inputconstants,0,0);
    //kernel_func_4265622974_4(exekernel->outputarrays[0],exekernel->inputarrays,exekernel->inputarrays_length,exekernel->inputconstants,0,0);
    logcustom(cloglevel,1,"computation done\n");
    return 0;
}



cphvb_intp bind_compound_kernel(jit_compound_kernel* ckernel, cphvb_instruction* instruction_list, cphvb_intp id) {
    bool cloglevel[] = {0};
    for(int i=0;i<ckernel->kernels_length;i++) {
        logcustom(cloglevel,0,"BCK Bind_compound_kernel's exekernel: %d\n",i);   
        bind_execution_kernel(ckernel->exekernels[i],instruction_list);
        //jit_pprint_execute_kernel(ckernel->exekernels[i]);
    }    
}

/**
 * Allocate result arrays for an execute kernel. 
 **/
cphvb_error allocate_for_exekernel(jit_execute_kernel* ekernel) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"AFE allocate_for_exekernel(%ld)\n",ekernel->kernel->id);
    cphvb_array* tarray = NULL;
    cphvb_error res;
    logcustom(cloglevel,1,"AFE %ld \n",ekernel->outputarrays_length);
    // this will be 1. Could be extended to more if more complex kernels where to be created.
    for(int i=0;i<ekernel->outputarrays_length;i++) {
        logcustom(cloglevel,0,"outputarray* = %p\n",ekernel->kernel, ekernel->outputarrays[i]);
        if (cphvb_base_array(ekernel->outputarrays[i])->data == NULL) {
            res = jit_mcache_malloc(ekernel->outputarrays[i]);
            if (res != CPHVB_SUCCESS) {
                logcustom(cloglevel,1,"AFE Failed! allocation [%d]\n",i);                
                return res;
            }
            logcustom(cloglevel,1,"AFE Allocated output array[%d]\n",i);
        }
        logcustom(cloglevel,1,"AFE Array is bounded: %p\n",cphvb_base_array(ekernel->outputarrays[i])->data);
    }
    return CPHVB_SUCCESS;    
}

/**
 * 
 **/
cphvb_intp execute_compound_kernel(jit_compute_functions* compute_functions, jit_compound_kernel* ckernel, cphvb_instruction* instructionlist) {
    bool cloglevel[] = {0};
    cphvb_error res;
    for(int i=0;i<ckernel->kernels_length;i++) {        
        res = allocate_for_exekernel(ckernel->exekernels[i]);
        if (res != CPHVB_SUCCESS) {
            return res;
        }        
        if(ckernel->exekernels[i]->kernel->type == JIT_EXPR_KERNEL) {
            logcustom(cloglevel,0,"execute_compound_kernel: JIT_EXPR_KERNEL - exekernel: %d\n",i);
            execute_kernel_expr(compute_functions,ckernel->exekernels[i]);
        } else
        if(ckernel->exekernels[i]->kernel->type == JIT_COMPILE_KERNEL) {
            logcustom(cloglevel,0,"execute_compound_kernel: JIT_COMPILE_KERNEL - exekernel: %d\n",i);  
            execute_kernel_compiled(ckernel->exekernels[i]);
        } else {
            logcustom(cloglevel,0,"execute_compound_kernel: NONE - exekernel: %d\n",i);            
        }
      
        
    } 

    
    return 0;
}


/**
 * reads arrays and constants from left to right expression leafs. Must be placed in this order when filled!  
 **/
void extract_expr_IL_map_data(jit_analyse_state* s, jit_expr* expr,jit_io_instruction_list_map* map,cphvb_intp* acount,cphvb_intp* ccount) {
    printf("extract_expr_IL_map_data()\n");
    
    if(is_array(expr)) {                    
        jit_name_entry* e = jita_nametable_lookup(s->nametable,expr->name);
        printf("%ld (%ld,%ld)\n",*acount,e->instr_num,e->operand_num);
        
        //~ jit_instruction_list_coord2* coord = (jit_instruction_list_coord2*) malloc(sizeof(jit_instruction_list_coord2));
        //~ coord->instruction = e->instr_num;
        //~ coord->operand =e->operand_num;        
        //~ map->array_map[*acount] = *coord;

        map->array_map[*acount].instruction = e->instr_num;
        map->array_map[*acount].operand = e->operand_num;    

        //map->constant_map[*acount] = jit_instruction_list_coord2(e->instr_num,e->operand_num);    
        *acount += 1;
    }
    if (is_constant(expr)){                
        jit_name_entry* e = jita_nametable_lookup(s->nametable,expr->name);
        printf("%ld (%ld,%ld)\n",*acount,e->instr_num,e->operand_num);
        //~ jit_instruction_list_coord2* coord = (jit_instruction_list_coord2*) malloc(sizeof(jit_instruction_list_coord2));
        //~ coord->instruction = e->instr_num;
        //~ coord->operand =e->operand_num;             
        //~ map->constant_map[*ccount] = *coord;

        map->constant_map[*ccount].instruction = e->instr_num;
        map->constant_map[*ccount].operand = e->operand_num;    
        
        //map->constant_map[*ccount] = jit_instruction_list_coord2(e->instr_num,e->operand_num);    
        *ccount += 1;          
    }
    if (is_un_op(expr)){
        extract_expr_IL_map_data(s,expr->op.expression.left,map,acount,ccount);
    }
    if (is_bin_op(expr)) {
        extract_expr_IL_map_data(s,expr->op.expression.left,map,acount,ccount);
        extract_expr_IL_map_data(s,expr->op.expression.right,map,acount,ccount);
    }    
}


//~ void map_to_exe_expr(jit_execute_expr* exe_expr,cphvb_instruction* instruction_list) {
    //~ cphvb_intp instr = 0;
    //~ cphvb_intp operand = 0;
    
    //~ printf("\n\n %p\n",exe_expr->inputarrays_length);
    //~ // lookup inputarray in instruction list
    //~ for(int i=0;i<exe_expr->inputarrays_length;i++ ) {
        //~ instr =     exe_expr->il_map->array_map[i].instruction;
        //~ operand =   exe_expr->il_map->array_map[i].operand;
        //~ exe_expr->inputarrays[i] = instruction_list[instr].operand[operand];        
        //~ printf("array from instr list: %p\n",exe_expr->inputarrays[i]);
    //~ }
//~ 
    //~ // lookup inputconstants in instruction list
    //~ for(int i=0;i<exe_expr->inputconstants_length;i++ ) {
        //~ instr =     exe_expr->il_map->constant_map[i].instruction;
        //~ operand =   exe_expr->il_map->constant_map[i].operand;
        //~ exe_expr->inputconstants[i] = &instruction_list[instr].constant;
        //~ printf("array from instr list: %p\n",exe_expr->inputconstants[i]);
    //~ }
//~ 
    //~ // lookup outputarrays in instruction list
    //~ for(int i=0;i<exe_expr->outputarrays_length;i++ ) {
        //~ instr =     exe_expr->il_map->output_array_map[i].instruction;
        //~ operand =   exe_expr->il_map->output_array_map[i].operand;
        //~ printf("%ld %ld %p\n",instr,operand,exe_expr->outputarrays);
        //~ exe_expr->outputarrays[i] = instruction_list[instr].operand[operand];
        //~ printf("array from instr list: %p\n",exe_expr->outputarrays[i]);
    //~ }
//~ }


/// UNUSED!
//~ cphvb_intp build_execute_expr(jit_analyse_state* s,jit_name_entry* entr, jit_kernel* execute_expr_out) {
    //~ printf("build_execute_expr()\n");
    //~ // check that the expr is not a value.
    //~ if ((is_array(entr->expr) || is_constant(entr->expr))) {
        //~ return 1;
    //~ }
//~ 
    //~ // count the number of arrays and constants in the expr.
    //~ cphvb_intp array_count = 0;
    //~ cphvb_intp constant_count = 0;    
    //~ expr_count_operands(entr->expr, &array_count, &constant_count);    
//~ 
    //~ //printf("arrs: %ld, consts: %ld\n",array_count,constant_count);
    //~ 
    //~ if(entr->is_userfunction) {
        //~ printf("userfunc!!!!\n");
    //~ } else {
//~ 
        //~ // create input/output instructionlist map
        //~ jit_io_instruction_list_map* io_map = (jit_io_instruction_list_map*) malloc(sizeof(jit_io_instruction_list_map));        
        //~ io_map->array_map_length = array_count;
        //~ io_map->constant_map_length = constant_count;
        //~ io_map->output_array_map_length = 1;
        //~ 
        //~ io_map->array_map = (jit_instruction_list_coord2*) malloc(sizeof(jit_instruction_list_coord2)*array_count);        
        //~ io_map->output_array_map = (jit_instruction_list_coord2*) malloc(sizeof(jit_instruction_list_coord2));
        //~ if(constant_count > 0) {
            //~ io_map->constant_map = (jit_instruction_list_coord2*) malloc(sizeof(jit_instruction_list_coord2)*constant_count);
        //~ } else {
            //~ io_map->constant_map = NULL;
        //~ }
//~ 
//~ 
        //~ 
        //~ printf("creating op_extract_state()\n");
        //~ op_extract_state* es = (op_extract_state*) malloc(sizeof(op_extract_state));
        //~ es->arrays = NULL;
        //~ es->arrays_length = 0;
        //~ es->constants = NULL;
        //~ es->constants_length = 0;
        //~ es->computation_string = NULL;
        //~ 
        //~ printf("+++ jit_expr_operand_extract_s\n");
        //~ jit_expr_operand_extract(s,entr->expr,es,false);
        //~ printf("--- jit_expr_operand_extract_s\n");
        //~ printf("%p %ld %p %ld :: %s\n",es->arrays,es->arrays_length,es->constants,es->constants_length, es->computation_string);
//~ 
        //~ 
        //~ // fill io_map
        //~ cphvb_intp acount=0, ccount=0;
        //~ extract_expr_IL_map_data(s,entr->expr,io_map,&acount,&ccount);
//~ 
        //~ 
        //~ jit_instruction_list_coord2* coord = (jit_instruction_list_coord2*) malloc(sizeof(jit_instruction_list_coord2));
        //~ coord->instruction = entr->instr_num;
        //~ coord->operand =entr->operand_num;  
        //~ io_map->output_array_map[0] = *coord;
            //~ 
        //~ // create execute expression struct
        //~ execute_expr_out->il_map = io_map;        
//~ 
    //~ }
    //~ return 0;
//~ 
//~ }

void build_expr_execution_kernel_userfunc(jit_analyse_state* s,jit_name_entry* entr,
                                map<cphvb_intp,cphvb_intp>* oas,
                                map<cphvb_intp,cphvb_intp>* as) {
    if(is_userfunc(entr->expr)) {
        int i = 0;
        for(i=0;i<entr->expr->op.userfunc->nin;i++) {
            ;
        }

        for(i=0;i<entr->expr->op.userfunc->nout;i++) {
            ;
        } 
    }
}

void build_expr_il_map_userfunc(jit_analyse_state* s,jit_name_entry* entr,
                                jit_io_instruction_list_map_lists* il_map,
                                vector<cphvb_instruction*>* is) {
                                    
        bool cloglevel[] = {0,0,0};
        logcustom(cloglevel,0,"BEIMU build_expr_il_map_userfunc() \n");
        jit_name_entry* tentr;
        int i = 0;                                        
        for(i=0;i<entr->expr->userfunction_inputs->size();i++) {
            //logcustom(cloglevel,1,"BEIMU +++ %d %p %p ",entr->expr->name,entr->instr->userfunc->operand[0],entr->instr->userfunc->operand[1]);
                                                    
            tentr = jita_nametable_lookup(s->nametable,entr->expr->userfunction_inputs->at(i)->name);            
            //logcustom(cloglevel,1,"BEIMU in: (%d %d) %p (%p)\n",tentr->instr_num,tentr->operand_num,entr->instr,entr->instr->userfunc->operand[tentr->operand_num]);
            
            il_map->array_map->push_back(new jit_instruction_list_coord(tentr->instr_num,tentr->operand_num));
            
            //il_map->output_array_map[0].instruction = entr->instr_num;
            //il_map->output_array_map[0].operand = entr->operand_num;
            //;
            logcustom(cloglevel,1,"BEIMU I (%d %d) \n",tentr->instr_num,tentr->operand_num);
        }
            
        for(i=0;i<entr->span;i++) {
            //logcustom(cloglevel,1,"BEIMU --- %d %p %p ",entr->expr->name,entr->instr->userfunc->operand[0],entr->instr->userfunc->operand[1]);
            //logcustom(cloglevel,1,"BEIMU %d out: (%d %d) %p (%p)\n",i,entr->instr_num,i,entr->instr,entr->instr->userfunc->operand[i]);
            il_map->output_array_map->push_back(new jit_instruction_list_coord(entr->instr_num,i));    
            //il_map->array_map[0].instruction = entr->instr_num;
            //il_map->array_map[0].operand = entr->operand_num;"
            logcustom(cloglevel,1,"BEIMU O (%d %d) \n",entr->instr_num,i);
            
        }
        is->push_back(entr->instr);
        
        
}


//~ /**
 //~ * 
 //~ **/
//~ void build_expr_il_map(jit_analyse_state* s,jit_name_entry* entr,
                                //~ jit_io_instruction_list_map_lists* il_map) {         
    //~ // output
    //~ il_map->output_array_map->push_back(new jit_instruction_list_coord(entr->instr_num,entr->operand_num));                                    
    //~ 
    //~ //printf("un: %d, bin:%d\n",is_un_op(entr->expr),is_bin_op(entr->expr));
    //~ // input
    //~ jit_name_entry* tmp_entr;
    //~ if (is_un_op(entr->expr) || is_bin_op(entr->expr)) {        
        //~ tmp_entr = jita_nametable_lookup(s->nametable,entr->expr->op.expression.left->name);
        //~ if (is_array(entr->expr)) {
            //~ il_map->array_map->push_back(new jit_instruction_list_coord(tmp_entr->instr_num,tmp_entr->operand_num));
        //~ } else if (is_constant(entr->expr)) {
            //~ 
            //~ jit_name_entry* tentr = jita_nametable_lookup(s->nametable,entr->expr->parent->name);
            //~ if (tentr->instr->operand[1] == NULL) {
                 //~ il_map->constant_map->push_back(new jit_instruction_list_coord(tentr->instr_num,1));
            //~ } else {
                //~ //tentr->instr->operand[2] == NULL
                //~ il_map->constant_map->push_back(new jit_instruction_list_coord(tentr->instr_num,2));
            //~ }            
        //~ }
    //~ }
//~ 
    //~ if (is_bin_op(entr->expr)) {
        //~ tmp_entr = jita_nametable_lookup(s->nametable,entr->expr->op.expression.right->name);
        //~ il_map->array_map->push_back(new jit_instruction_list_coord(tmp_entr->instr_num,tmp_entr->operand_num));                    
    //~ }    
//~ }
                                    
void build_execution_kernel(jit_kernel* kernel,
                                cphvb_index oas_length,
                                cphvb_index as_length,
                                cphvb_index cs_length,
                                jit_execute_kernel* execute_kernel_out) {
    
    execute_kernel_out->kernel = kernel;
    
    // create array output-array
    execute_kernel_out->outputarrays = ((cphvb_array**) malloc(sizeof(cphvb_array*)*oas_length));
    execute_kernel_out->outputarrays_length = oas_length;
       
    // create array input-array
    execute_kernel_out->inputarrays = ((cphvb_array**) malloc(sizeof(cphvb_array*)*as_length));
    execute_kernel_out->inputarrays_length = as_length;

    // create constants input-array
    execute_kernel_out->inputconstants_length = cs_length;
    if (cs_length > 0) {        
        execute_kernel_out->inputconstants = (cphvb_constant**) malloc(sizeof(cphvb_constant*)*cs_length);
    } else {
        execute_kernel_out->inputconstants = NULL;
    }
}



/**
 * Does not set the outputs for the IL map, or fill in the output map.
 **/
string _expr_extract_traverser3(jit_analyse_state* s,jit_expr* expr,
                                        map<cphvb_intp,cphvb_intp>* as,
                                        map<cphvb_constant*,cphvb_intp>* cs,                                                                                
                                        jit_io_instruction_list_map_lists* il_map,
                                        vector<cphvb_instruction*>* is,                                        
                                        bool create_computation_string,
                                        bool create_instruction_list) {

    bool cloglevel[] = {0,0,0};    
    logcustom(cloglevel,0,"s _expr_operand_extract_traverser3()\n");
    
    stringstream ss;
    
    jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);
    logcustom(cloglevel,2,"+++ %ld .. executed= %s\n",entr->expr->name, entr->is_executed?"True":"False");
    if (is_array(expr) || entr->is_executed) {        
        cphvb_intp ia = -1;        

        // find or add to array list    
        if(as->find(expr->name) == as->end()) {                    
            ia = as->size();                        
            as->insert(pair<cphvb_intp,cphvb_intp>(expr->name,ia));

            // create IL map entry.                                    
            il_map->array_map->push_back(new jit_instruction_list_coord(entr->instr_num,entr->operand_num));                            
        } else {            
            ia = as->find(expr->name)->second;            
        }
            
        // create computation expression
        if (create_computation_string) {
            if (is_array(expr)) {
                ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(expr->op.array->type) << "*) ds[" << ia << "]->data))";                                            
            } else { // if it is a executed entr                                                
                ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(entr->arrayp->type) << "*) ds[" << ia << "]->data))";
            }            
        }       
        
    } else if (is_constant(expr)) {
        cphvb_intp ic = -1;

        //printf("=========== %s - %p - ",cs->find(expr->op.constant) == cs->end()?"NoTIn":"In",expr->op.constant);
        //constant_value_text_s(expr->op.constant);
        //printf("\n");
        
        if(cs->find(expr->op.constant) == cs->end()) {            
            ic = cs->size();
            logcustom(cloglevel,1,"insert into CS: %d - %p\n",cs->size(), expr->op.constant);            
            cs->insert(pair<cphvb_constant*,cphvb_intp>(expr->op.constant,ic));
            logcustom(cloglevel,1,"%p - %ld / %ld %d\n",entr,entr->instr_num,entr->operand_num,cs->size());
            
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
        
        
    } else if(is_bin_op(expr)) {
        if (create_instruction_list) {
            is->push_back(entr->instr);
        } 
        string s1 = _expr_extract_traverser3(s,expr->op.expression.left,as,cs,il_map,is,create_computation_string,create_instruction_list);
        string s2 = _expr_extract_traverser3(s,expr->op.expression.right,as,cs,il_map,is,create_computation_string,create_instruction_list);
        logcustom(cloglevel,2,"binary, s1,s2 created\n");
        if (create_computation_string) {            
            return build_expression_string(expr->op.expression.opcode,s1,s2);
        }        
        
    } else if (is_un_op(expr)) {
        if (create_instruction_list) {
            is->push_back(entr->instr);
        }
        string s1 = _expr_extract_traverser3(s,expr->op.expression.left,as,cs,il_map,is,create_computation_string,create_instruction_list);
        logcustom(cloglevel,2,"unary, s1 created\n");
        if (create_computation_string) {            
            return build_expression_string(expr->op.expression.opcode,s1,"NONE");        
        }        
    }
    
    logcustom(cloglevel,0,"e jitcg_expr_travers_array() -- %s\n",ss.str().c_str());
    return ss.str();  
}

/**
 * 
 **/
string expr_extract_traverser(jit_analyse_state* s,jit_expr* expr,
                                map<cphvb_intp,cphvb_intp>* oas,
                                map<cphvb_intp,cphvb_intp>* as,
                                map<cphvb_constant*,cphvb_intp>* cs,                                                                                
                                jit_io_instruction_list_map_lists* il_map,
                                vector<cphvb_instruction*>* is,                                                                    
                                bool create_computation_string,
                                bool create_instruction_list) {

    jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);    
    il_map->output_array_map->push_back(new jit_instruction_list_coord(entr->instr_num,entr->operand_num));            
    oas->insert(pair<cphvb_intp,cphvb_intp>(expr->name,0));    

    stringstream ss;
    ss << "*(((" << cphvb_type_typetext(entr->arrayp->type) << "*) doa->data) + off_oa) = ";
    ss << _expr_extract_traverser3(s,expr,as,cs,il_map,is,create_computation_string,create_instruction_list);    
    entr->is_executed = true;
    return ss.str();
}


/**
 * Does not set the outputs for the IL map, or fill in the output map.
 **/
void _expr_extract_traverser2(jit_analyse_state* s,jit_expr* expr,
                                        map<cphvb_intp,cphvb_intp>* as,
                                        map<cphvb_constant*,cphvb_intp>* cs,                                                                                
                                        jit_io_instruction_list_map_lists* il_map,
                                        vector<cphvb_instruction*>* is,
                                        stringstream* comp_ss,                                        
                                        bool create_computation_string,
                                        bool create_instruction_list,
                                        bool create_il_map,
                                        bool create_operand_map) {
    //printf("s _expr_operand_extract_traverser2()\n");
    
    jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);
    printf("+++ %ld .. executed= %s\n",entr->expr->name, entr->is_executed?"True":"False");
    if (is_array(expr) || entr->is_executed) {        
        cphvb_intp ia = -1;        
        // if not added yet        
        if(as->find(expr->name) == as->end()) {                    
            ia = as->size();
            
            if (create_operand_map) {
                as->insert(pair<cphvb_intp,cphvb_intp>(expr->name,ia));
            }

            if (create_il_map) {
                
                il_map->array_map->push_back(new jit_instruction_list_coord(entr->instr_num,entr->operand_num));    
                //il_map->array_map[ia].instruction = entr->instr_num;
                //il_map->array_map[ia].operand = entr->operand_num;
            }
        } else {
            if (create_operand_map) {
                ia = as->find(expr->name)->second;
            }
        }
        
        //printf("---------- %ld  %ld\n",expr->name,ia);
        
        // create computation expression
        if (create_computation_string) {
            if (is_array(expr)) {
                //*comp_ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(expr->op.array->type) << "*) as[" << ia << "]->data))";
                *comp_ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(expr->op.array->type) << "*) ds[" << ia << "]->data))";
            } else {
                // if it is a executed entr                
                //*comp_ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(entr->arrayp->type) << "*) as[" << ia << "]->data))";
                *comp_ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(entr->arrayp->type) << "*) ds[" << ia << "]->data))";
            }
        }       
        
    } else if (is_constant(expr)) {
        cphvb_intp ic = -1;
        // if not added yet
        //printf("=========== %s - %p - ",cs->find(expr->op.constant) == cs->end()?"NoTIn":"In",expr->op.constant);
        //constant_value_text_s(expr->op.constant);
        //printf("\n");
        
        if(cs->find(expr->op.constant) == cs->end()) {            
            ic = cs->size();        
            //printf("insert into CS: %d - %p\n",cs->size(), expr->op.constant);
            cs->insert(pair<cphvb_constant*,cphvb_intp>(expr->op.constant,ic));
                    
            //printf("%p - %ld / %ld %d\n",entr,entr->instr_num,entr->operand_num,cs->size());
            jit_name_entry* tentr = jita_nametable_lookup(s->nametable,expr->parent->name);
            if (tentr->instr->operand[1] == NULL) {
                 il_map->constant_map->push_back(new jit_instruction_list_coord(tentr->instr_num,1));
            } else { //tentr->instr->operand[2] == NULL
                il_map->constant_map->push_back(new jit_instruction_list_coord(tentr->instr_num,2));
            }        
            
        } else {            
            ic = cs->find(expr->op.constant)->second;            
        }                
        *comp_ss << " *((" << cphvb_type_typetext(expr->op.constant->type)  << "*)" << " &cs[" << ic << "]->value) ";
        
        
    } else if(is_bin_op(expr)) {
        if (create_computation_string) {
            *comp_ss << "(";
            _expr_extract_traverser2(s,expr->op.expression.left,as,cs,il_map,is,comp_ss,create_computation_string,create_instruction_list,create_il_map,create_operand_map);
            jitcg_codetext_opcode_stream(expr->op.expression.opcode,comp_ss);                                     
            _expr_extract_traverser2(s,expr->op.expression.right,as,cs,il_map,is,comp_ss,create_computation_string,create_instruction_list,create_il_map,create_operand_map);
            *comp_ss << ")";
        } else {
            _expr_extract_traverser2(s,expr->op.expression.left,as,cs,il_map,is,comp_ss,create_computation_string,create_instruction_list,create_il_map,create_operand_map);
            _expr_extract_traverser2(s,expr->op.expression.right,as,cs,il_map,is,comp_ss,create_computation_string,create_instruction_list,create_il_map,create_operand_map);
        }
        
        if (create_instruction_list) {
            is->push_back(entr->instr);
        }
    } else if (is_un_op(expr)) {
        if (create_computation_string) {
            jitcg_codetext_opcode_stream(expr->op.expression.opcode,comp_ss);  
            *comp_ss << "(";        
            _expr_extract_traverser2(s,expr->op.expression.left,as,cs,il_map,is,comp_ss,create_computation_string,create_instruction_list,create_il_map,create_operand_map);                
            *comp_ss << ")";
        } else {
            _expr_extract_traverser2(s,expr->op.expression.left,as,cs,il_map,is,comp_ss,create_computation_string,create_instruction_list,create_il_map,create_operand_map);                
        }
        if (create_instruction_list) {
            is->push_back(entr->instr);
        }
        
    }        
    logInfo("e jitcg_expr_travers_array()\n");
}


/**
 * 
 **/
void expr_extract_traverser2(jit_analyse_state* s,jit_expr* expr,
                                map<cphvb_intp,cphvb_intp>* oas,
                                map<cphvb_intp,cphvb_intp>* as,
                                map<cphvb_constant*,cphvb_intp>* cs,                                                                                
                                jit_io_instruction_list_map_lists* il_map,
                                vector<cphvb_instruction*>* is,
                                stringstream* comp_ss,                                        
                                bool create_computation_string,
                                bool create_instruction_list,
                                bool create_il_map,
                                bool create_operand_map) {

    // expression outputs.
    //   add to oas.
    //   add to il_map
    jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);

    
    if (create_il_map) {
        il_map->output_array_map->push_back(new jit_instruction_list_coord(entr->instr_num,entr->operand_num));    
    }
    if (create_operand_map) {            
        oas->insert(pair<cphvb_intp,cphvb_intp>(expr->name,0));
    }

    *comp_ss << "*(((" << cphvb_type_typetext(entr->arrayp->type) << "*) doa->data) + off_oa) = ";
    _expr_extract_traverser2(s,expr,as,cs,il_map,is,comp_ss,create_computation_string,create_instruction_list,create_il_map,create_operand_map);
    entr->is_executed = true;
}


//~ 
//~ /**
 //~ * Does not set the outputs for the IL map, or fill in the output map.
 //~ **/
//~ void _expr_extract_traverser(jit_analyse_state* s,jit_expr* expr,
                                        //~ map<cphvb_intp,cphvb_intp>* as,
                                        //~ map<cphvb_constant*,cphvb_intp>* cs,                                                                                
                                        //~ jit_io_instruction_list_map* il_map,
                                        //~ vector<cphvb_instruction*>* is,
                                        //~ stringstream* comp_ss,                                        
                                        //~ bool create_computation_string,
                                        //~ bool create_instruction_list,
                                        //~ bool create_il_map,
                                        //~ bool create_operand_map) {
    //~ //printf("s _expr_operand_extract_traverser2()\n");
    //~ 
    //~ jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);
    //~ printf("+++ %ld .. executed= %s\n",entr->expr->name, entr->is_executed?"True":"False");
    //~ if (is_array(expr) || entr->is_executed) {        
        //~ cphvb_intp ia = -1;        
        //~ // if not added yet        
        //~ if(as->find(expr->name) == as->end()) {                    
            //~ ia = as->size();
            //~ 
            //~ if (create_operand_map) {
                //~ as->insert(pair<cphvb_intp,cphvb_intp>(expr->name,ia));
            //~ }
//~ 
            //~ if (create_il_map) {
                //~ il_map->array_map[ia].instruction = entr->instr_num;
                //~ il_map->array_map[ia].operand = entr->operand_num;
            //~ }
        //~ } else {
            //~ if (create_operand_map) {
                //~ ia = as->find(expr->name)->second;
            //~ }
        //~ }
        //~ 
        //~ printf("---------- %ld  %ld\n",expr->name,ia);
        //~ 
        //~ // create computation expression
        //~ if (create_computation_string) {
            //~ if (is_array(expr)) {
                //~ //*comp_ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(expr->op.array->type) << "*) as[" << ia << "]->data))";
                //~ *comp_ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(expr->op.array->type) << "*) ds[" << ia << "]->data))";
            //~ } else {
                //~ // if it is a executed entr                
                //~ //*comp_ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(entr->arrayp->type) << "*) as[" << ia << "]->data))";
                //~ *comp_ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(entr->arrayp->type) << "*) ds[" << ia << "]->data))";
            //~ }
        //~ }       
        //~ 
    //~ } else  if (is_constant(expr)) {
        //~ cphvb_intp ic = -1;
        //~ // if not added yet
        //~ 
        //~ if(cs->find(expr->op.constant) == cs->end()) {            
            //~ ic = cs->size();
            //~ if (create_operand_map) {
                //~ cs->insert(pair<cphvb_constant*,cphvb_intp>(expr->op.constant,ic));
            //~ }
            //~ if (create_il_map) {
                //~ il_map->constant_map[ic].instruction = entr->instr_num;
                //~ il_map->constant_map[ic].operand = entr->operand_num;
            //~ }
        //~ } else {
            //~ if (create_operand_map) {
                //~ ic = cs->find(expr->op.constant)->second;
            //~ }
        //~ }
        //~ printf("+++++++++ %ld  %ld\n",expr->name,ic);
        //~ if (create_computation_string) {
            //~ *comp_ss << "*((" << cphvb_type_typetext(expr->op.constant->type)  << "*)" << " &cs[" << ic << "]->value)";
        //~ }
        //~ 
    //~ } else if(is_bin_op(expr)) {
        //~ if (create_computation_string) {
            //~ *comp_ss << "(";
            //~ _expr_extract_traverser(s,expr->op.expression.left,as,cs,il_map,is,comp_ss,create_computation_string,create_instruction_list,create_il_map,create_operand_map);
            //~ jitcg_codetext_opcode_stream(expr->op.expression.opcode,comp_ss);                                     
            //~ _expr_extract_traverser(s,expr->op.expression.right,as,cs,il_map,is,comp_ss,create_computation_string,create_instruction_list,create_il_map,create_operand_map);
            //~ *comp_ss << ")";
        //~ } else {
            //~ _expr_extract_traverser(s,expr->op.expression.left,as,cs,il_map,is,comp_ss,create_computation_string,create_instruction_list,create_il_map,create_operand_map);
            //~ _expr_extract_traverser(s,expr->op.expression.right,as,cs,il_map,is,comp_ss,create_computation_string,create_instruction_list,create_il_map,create_operand_map);
        //~ }
        //~ 
        //~ if (create_instruction_list) {
            //~ is->push_back(entr->instr);
        //~ }
    //~ } else if (is_un_op(expr)) {
        //~ if (create_computation_string) {
            //~ jitcg_codetext_opcode_stream(expr->op.expression.opcode,comp_ss);  
            //~ *comp_ss << "(";        
            //~ _expr_extract_traverser(s,expr->op.expression.left,as,cs,il_map,is,comp_ss,create_computation_string,create_instruction_list,create_il_map,create_operand_map);                
            //~ *comp_ss << ")";
        //~ } else {
            //~ _expr_extract_traverser(s,expr->op.expression.left,as,cs,il_map,is,comp_ss,create_computation_string,create_instruction_list,create_il_map,create_operand_map);                
        //~ }
        //~ if (create_instruction_list) {
            //~ is->push_back(entr->instr);
        //~ }
        //~ 
    //~ }        
    //~ logInfo("e jitcg_expr_travers_array()\n");
//~ }

//~ 
//~ /**
 //~ * 
 //~ **/
//~ void expr_extract_traverser(jit_analyse_state* s,jit_expr* expr,
                                //~ map<cphvb_intp,cphvb_intp>* oas,
                                //~ map<cphvb_intp,cphvb_intp>* as,
                                //~ map<cphvb_constant*,cphvb_intp>* cs,                                                                                
                                //~ jit_io_instruction_list_map* il_map,
                                //~ vector<cphvb_instruction*>* is,
                                //~ stringstream* comp_ss,                                        
                                //~ bool create_computation_string,
                                //~ bool create_instruction_list,
                                //~ bool create_il_map,
                                //~ bool create_operand_map) {
    //~ // expression outputs.
    //~ //   add to oas.
    //~ //   add to il_map
    //~ jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);
                      //~ 
    //~ if (create_il_map) {
        //~ il_map->output_array_map[0].instruction = entr->instr_num;
        //~ il_map->output_array_map[0].operand = entr->operand_num;        
    //~ }
    //~ if (create_operand_map) {            
        //~ oas->insert(pair<cphvb_intp,cphvb_intp>(expr->name,0));
    //~ }
//~ 
    //~ *comp_ss << "*(((" << cphvb_type_typetext(entr->arrayp->type) << "*) doa->data) + off_oa) = ";
    //~ _expr_extract_traverser(s,expr,as,cs,il_map,is,comp_ss,create_computation_string,create_instruction_list,create_il_map,create_operand_map);
    //~ entr->is_executed = true;
//~ }



void instruction_copy_to(cphvb_instruction* from, cphvb_instruction* to) {
    to->opcode = from->opcode;
    to->status = from->status;    
    to->constant = from->constant;    
    //to->operand cphvb_array* = operand[CPHVB_MAX_NO_OPERANDS];
        
    //printf("%ld %ld %p %p\n",to->status,to->opcode,&to->constant,from->userfunc);

    if (from->opcode == CPHVB_USERFUNC) {        
        to->userfunc = (cphvb_userfunc*) malloc(from->userfunc->struct_size);
        memcpy(to->userfunc,from->userfunc,from->userfunc->struct_size);        
    } else {
        to->userfunc = NULL;
    }
    //printf("FrTo: %d %d, %d %d\n",from->userfunc->nin,to->userfunc->nin,from->userfunc->nout,to->userfunc->nout);
    
}
   
/**
 * TODO: Handle multi node expressions. Can only handle "instruction" type expressions.
 * 
 **/
cphvb_intp build_expr_kernel(jit_analyse_state* s, jit_name_entry* entr, cphvb_intp hash, cphvb_index id,jit_execute_kernel* execute_kernel_out) {
    bool cloglevel[] = {0,0};    
    logcustom(cloglevel,0,"E- build_expr_kernel()\n");
    // check that the expr is not a value.
    if ((is_array(entr->expr) || is_constant(entr->expr))) {
        return 1;
    }
    
    // setup travers datastructures    
    map<cphvb_intp,cphvb_intp>* oas = new map<cphvb_intp,cphvb_intp>();
    map<cphvb_intp,cphvb_intp>* as  = new map<cphvb_intp,cphvb_intp>();
    map<cphvb_constant*,cphvb_intp>* cs  = new map<cphvb_constant*,cphvb_intp>();   
    vector<cphvb_instruction*>* is  = new vector<cphvb_instruction*>();

    jit_io_instruction_list_map_lists* instr_list_map = (jit_io_instruction_list_map_lists*) malloc(sizeof(jit_io_instruction_list_map_lists));
    instr_list_map->array_map = new vector< jit_instruction_list_coord* >();
    instr_list_map->output_array_map = new vector< jit_instruction_list_coord* >();
    instr_list_map->constant_map = new vector< jit_instruction_list_coord* >();
    
    // Travers the expression and fill as and cs.
    if (entr->is_userfunction) {
        build_expr_il_map_userfunc(s,entr,instr_list_map,is);
        logcustom(cloglevel,1,"instruction userfunc: %ld %ld \n",entr->instr->userfunc->nin,entr->instr->userfunc->nout);
    } else {
        string computestring = expr_extract_traverser(s,entr->expr,oas,as,cs,instr_list_map,is,false,true);
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
    for(int i=0; i<is->size(); i++) {
        instruction_copy_to(is->at(i),&kernel->expr_kernel->instructions[i]);
    }    
    //kernel->expr_kernel->instructions[0]->
    
    logcustom(cloglevel,1,"instruction userfunc: is.size()=%d %p\n",is->size(),kernel->expr_kernel->instructions[0].userfunc);
    //,kernel->expr_kernel->instructions[0].userfunc->nout);
    build_execution_kernel(kernel,
                            instr_list_map->output_array_map->size(),
                            instr_list_map->array_map->size(),
                            instr_list_map->constant_map->size(),
                            execute_kernel_out);
    logcustom(cloglevel,1,"E- oas:%d   as:%d \n",oas->size(), as->size());

    return 0;
}




/**
 * 
 **/
cphvb_intp build_compile_kernel(jit_analyse_state* s, jit_name_entry* entr, cphvb_intp hash, cphvb_index id,jit_execute_kernel* execute_kernel_out) {
    bool cloglevel[] = {0,0};    
    logcustom(cloglevel,0,"build_compile_kernel()\n");
    
    // check that the expr is not a value.
    if ((is_array(entr->expr) || is_constant(entr->expr))) {
        return 1;
    }    

    // travers the expression and fill as and cs.    
    map<cphvb_intp,cphvb_intp>* as = new map<cphvb_intp,cphvb_intp>();
    map<cphvb_intp,cphvb_intp>* oas = new map<cphvb_intp,cphvb_intp>();
    map<cphvb_constant*,cphvb_intp>* cs = new map<cphvb_constant*,cphvb_intp>();

    jit_io_instruction_list_map_lists* instr_list_map = (jit_io_instruction_list_map_lists*) malloc(sizeof(jit_io_instruction_list_map_lists));
    instr_list_map->array_map = new vector< jit_instruction_list_coord* >();
    instr_list_map->output_array_map = new vector< jit_instruction_list_coord* >();
    instr_list_map->constant_map = new vector< jit_instruction_list_coord* >();
    
    string computestring = expr_extract_traverser(s,entr->expr,oas,as,cs,instr_list_map,NULL,true,false);    
    
    // create kernel
    jit_kernel* kernel = (jit_kernel*) malloc(sizeof(jit_kernel));
    kernel->type    = JIT_COMPILE_KERNEL;    
    kernel->il_map  = instr_list_map;
    kernel->hash    = 1;    //expr_hash(entr->expr)
    kernel->id      = id;
    kernel->compute_kernel = NULL;
    
    // compile the kernel            
    char buff[30];
    sprintf(buff,"kernel_func_%ld_%ld",hash,id);
    string kernel_name = buff;
    string func_text = create_kernel_function_travers(kernel_name.c_str(),computestring);
    
    kernel->compute_kernel = jitc_compile_computefunction(kernel_name,func_text);             
    //printf("setting comutation string\n");
    //printf("computation string: %s\n",func_text.c_str());
    //printf("\n%d\n\n%s\n",kernel_name.c_str(),func_text.c_str());

    //kernel->compute_kernel->function = kernel_func_4265622974_0;
    
    execute_kernel_out->kernel = kernel;
    build_execution_kernel(kernel,
                            instr_list_map->output_array_map->size(),
                            instr_list_map->array_map->size(),
                            instr_list_map->constant_map->size(),
                            execute_kernel_out);
    
    logcustom(cloglevel,1,"setting computation string\n");
    logcustom(cloglevel,1,"computation string: %s\n",computestring.c_str());

    
   
    

    return 0;
}

/**
 * 
 **/
cphvb_intp build_compound_kernel(jit_analyse_state* s, set<cphvb_intp>* execution_list, cphvb_index compound_id, jit_compound_kernel* compoundk_out) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"+ build_compound_kernel( executionlist length: %d,compound ID: %ld)\n",execution_list->size(),compound_id);
        
    // set id of the compund kernel.        
    compoundk_out->id = compound_id;    
    jit_name_entry* entr;
    set<cphvb_intp>::iterator it;
    cphvb_intp exprhash = 0;
    int kernel_count = 0;

    //jit_pprint_nametable(s->nametable); printf("\n");
    
    for(it=execution_list->begin();it!=execution_list->end();it++) {
        entr = jita_nametable_lookup(s->nametable,*it);
        logcustom(cloglevel,1,"+ name:%d  depth: %ld\n",*it, entr->expr->depth);

        exprhash = expr_hash(entr->expr);
        
        jit_execute_kernel* execute_kernel = (jit_execute_kernel*) malloc(sizeof(jit_execute_kernel));

        if(entr->is_userfunction) {
            build_expr_kernel(s,entr,exprhash,kernel_count,execute_kernel);
        } else         
        if (entr->expr->depth == 1) {
            // a single instruction as and expression.
            logcustom(cloglevel,1,"+ %d build_expr_kernel \n",*it);
            build_expr_kernel(s,entr,exprhash,kernel_count,execute_kernel);
            //exekernel->kernel->id = 
            
        } else {
            // handle as kernel
            logcustom(cloglevel,1,"+ %d build_compile_kernel \n",*it);
            build_compile_kernel(s,entr,exprhash,kernel_count,execute_kernel);
        }

        //printf("execution kernel for NT: %d - %s\n",*it,executekernel_type(execute_kernel));
        //jit_pprint_il_map(execute_kernel->kernel->il_map);
        //jit_pprint_execute_kernel(execute_kernel);
        
        //printf("%d %p %d\n",(kernel_count),compoundk_out->kernels[0], sizeof(compoundk_out->kernels));
        
        compoundk_out->exekernels[kernel_count] = execute_kernel;
        kernel_count++;
    }
    
    return 0;
}




 



