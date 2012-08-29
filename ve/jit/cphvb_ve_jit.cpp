/*
 * Copyright 2011 Simon A. F. Lund <safl@safl.dk>
 *
 * This file is part of cphVB.
 *
 * cphVB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cphVB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cphVB. If not, see <http://www.gnu.org/licenses/>.
 * 
 * modified by Johannes Lund. Based on the cphvb_ve_simple.cpp
 */

#ifndef _JIT_LOG_LEVEL
#define _JIT_LOG_LEVEL 4 // 0 NO LOGGING, 1 ERROR, 2 WARNING, 3 INFO, 4 DEBUG
#endif

#include <stdio.h>
#include <iostream>
#include <cphvb.h>
#include "cphvb_ve_jit.h"
#include <cphvb_compute.h>
#include "jit_ast.h"
#include "jit_ssa_analyser.h"
#include "jit_analyser.h"
#include <list>
#include "jit_codegenerator.h"
#include <iostream>
#include <sstream>

using namespace std;

static cphvb_component *myself = NULL;
static cphvb_userfunc_impl reduce_impl = NULL;
static cphvb_intp reduce_impl_id = 0;
static cphvb_userfunc_impl random_impl = NULL;
static cphvb_intp random_impl_id = 0;
static cphvb_userfunc_impl matmul_impl = NULL;
static cphvb_intp matmul_impl_id = 0;

static cphvb_userfunc_impl jacstenc_impl = NULL;
static cphvb_intp jacstenc_impl_id = 0;

std::map<cphvb_array*,ssavt*> _ssalist;
std::map<cphvb_array*,ast*> _attable;
ssalist_t _ssaua_table;

template <typename Type> cphvb_error do_jacstenc(cphvb_array *out, 
    cphvb_array *cells, cphvb_array *u, cphvb_array *l, cphvb_array *r, 
    cphvb_array *d, cphvb_float64 cons ) 
    {

    if(cphvb_data_malloc(out) != CPHVB_SUCCESS)
        return CPHVB_OUT_OF_MEMORY;    

    Type* out_data;
    Type* cells_data;
    Type* u_data;
    Type* l_data;
    Type* r_data;
    Type* d_data;    

    cphvb_data_get(out, (cphvb_data_ptr*) &out_data);
    cphvb_data_get(cells, (cphvb_data_ptr*) &cells_data);
    cphvb_data_get(u, (cphvb_data_ptr*) &u_data);
    cphvb_data_get(l, (cphvb_data_ptr*) &l_data);
    cphvb_data_get(r, (cphvb_data_ptr*) &r_data);
    cphvb_data_get(d, (cphvb_data_ptr*) &d_data);
    
    cphvb_intp i_out,i_cells,i_u,i_l,i_r,i_d;

    cphvb_intp M = cells->shape[0];
    cphvb_intp N = cells->shape[1];        
    
    for(cphvb_intp i = 0; i < M; i++){
        for(cphvb_intp j = 0; j < N; j++){
            
            i_out = out->start + i*out->stride[0]+j*out->stride[1];
            i_cells = cells->start + i*cells->stride[0]+j*cells->stride[1];
            i_u = u->start + i*u->stride[0]+j*u->stride[1];
            i_l = l->start + i*l->stride[0]+j*l->stride[1];
            i_r = r->start + i*r->stride[0]+j*r->stride[1];
            i_d = d->start + i*d->stride[0]+j*d->stride[1];
                        
            out_data[i_out] = (cells_data[i_cells] + u_data[i_u] + \
                    l_data[i_l] + r_data[i_r] + d_data[i_d])*cons;               
        }	
    }
    
    return CPHVB_SUCCESS;
}


/**
 * cphvb_compute_jacstenc
 *
 * Implementation of the user-defined funtion "matmul".
 * Note that we follow the function signature defined by cphvb_userfunc_impl.
 * 
 * work = (up + left + right + down) * 0.2
 * diff = abs(cells - work)
 * delta = addreduce,addreduce
 * cells = work
 * 
 */
cphvb_error cphvb_compute_jacstenc(cphvb_userfunc *arg, void* ve_arg)
{
    //printf("compute jacstenc: data extracted\n");
    // cells,up,left,right,down,constant,out 
    cphvb_jacstenc_type *m_arg = (cphvb_jacstenc_type *) arg;
    
    cphvb_array     *out    = m_arg->operand[0];
    cphvb_array     *cells  = m_arg->operand[1];
    cphvb_array     *up     = m_arg->operand[2];
    cphvb_array     *left   = m_arg->operand[3];
    cphvb_array     *right  = m_arg->operand[4];
    cphvb_array     *down   = m_arg->operand[5];    
    
    cphvb_float64   cons    = m_arg->mult;
    
        
    //Make sure that the arrays memory are allocated.
    if(cphvb_data_malloc(out) != CPHVB_SUCCESS)
        return CPHVB_OUT_OF_MEMORY; 
    if(cphvb_data_malloc(cells) != CPHVB_SUCCESS)
        return CPHVB_OUT_OF_MEMORY; 
    if(cphvb_data_malloc(up) != CPHVB_SUCCESS)
        return CPHVB_OUT_OF_MEMORY; 
    if(cphvb_data_malloc(left) != CPHVB_SUCCESS)
        return CPHVB_OUT_OF_MEMORY; 
    if(cphvb_data_malloc(right) != CPHVB_SUCCESS)
        return CPHVB_OUT_OF_MEMORY;         
    if(cphvb_data_malloc(down) != CPHVB_SUCCESS)
        return CPHVB_OUT_OF_MEMORY;                

    switch (out->type)
    {
    	case CPHVB_INT8:
		    return do_jacstenc<cphvb_int8>(out,cells,up,left,right,down,cons);
    	case CPHVB_INT16:
		    return do_jacstenc<cphvb_int16>(out,cells,up,left,right,down,cons);
    	case CPHVB_INT32:
		    return do_jacstenc<cphvb_int32>(out,cells,up,left,right,down,cons);
    	case CPHVB_INT64:
		    return do_jacstenc<cphvb_int64>(out,cells,up,left,right,down,cons);
        case CPHVB_UINT8:
		    return do_jacstenc<cphvb_uint8>(out,cells,up,left,right,down,cons);
    	case CPHVB_UINT16:
		    return do_jacstenc<cphvb_uint16>(out,cells,up,left,right,down,cons);
    	case CPHVB_UINT32:
	        return do_jacstenc<cphvb_uint32>(out,cells,up,left,right,down,cons);
    	case CPHVB_UINT64:
		    return do_jacstenc<cphvb_uint64>(out,cells,up,left,right,down,cons);
    	case CPHVB_FLOAT32:
		    return do_jacstenc<cphvb_float32>(out,cells,up,left,right,down,cons);
    	case CPHVB_FLOAT64:
		    return do_jacstenc<cphvb_float64>(out,cells,up,left,right,down,cons);
        default:
            return CPHVB_TYPE_NOT_SUPPORTED;

	}
}

void print_instruction(cphvb_instruction *inst) {
    cphvb_pprint_instr(inst);
    //printf(inst->opcode);    
}

cphvb_error instruction_handler_simple(cphvb_instruction *inst) {
    cphvb_intp nops, i;    
    cphvb_error ret;    
    
    if(inst->status != CPHVB_INST_DONE)     // SKIP instruction
    {                
        nops = cphvb_operands(inst->opcode);    // Allocate memory for operands
        for(i=0; i<nops; i++)
        {
            if (!cphvb_is_constant(inst->operand[i]))
            {
                if (cphvb_data_malloc(inst->operand[i]) != CPHVB_SUCCESS)
                {
                    return CPHVB_OUT_OF_MEMORY; // EXIT
                }
            }
        }

        switch(inst->opcode)                    // Dispatch instruction
        {
            case CPHVB_NONE:                    // NOOP.
            case CPHVB_DISCARD:
            case CPHVB_SYNC:
                inst->status = CPHVB_INST_DONE;
                break;

            case CPHVB_USERFUNC:                // External libraries

                if(inst->userfunc->id == reduce_impl_id)
                {
                    ret = reduce_impl(inst->userfunc, NULL);
                    inst->status = (ret == CPHVB_SUCCESS) ? CPHVB_INST_DONE : ret;
                }
                else if(inst->userfunc->id == random_impl_id)
                {
                    ret = random_impl(inst->userfunc, NULL);
                    inst->status = (ret == CPHVB_SUCCESS) ? CPHVB_INST_DONE : ret;
                }
                else if(inst->userfunc->id == matmul_impl_id)
                {
                    ret = matmul_impl(inst->userfunc, NULL);
                    inst->status = (ret == CPHVB_SUCCESS) ? CPHVB_INST_DONE : ret;
                }
                else if(inst->userfunc->id == jacstenc_impl_id)
                {
                    ret = jacstenc_impl(inst->userfunc, NULL);
                    inst->status = (ret == CPHVB_SUCCESS) ? CPHVB_INST_DONE : ret;
                }
                
                else                            // Unsupported userfunc
                {
                    inst->status = CPHVB_USERFUNC_NOT_SUPPORTED;
                }

                break;

            default:                            // Built-in operations
                ret = cphvb_compute_apply( inst );
                inst->status = (ret == CPHVB_SUCCESS) ? CPHVB_INST_DONE : ret;
        }
        
        if (inst->status != CPHVB_INST_DONE)    // Instruction failed
        {
            return CPHVB_ERROR;
        }
    }
    
    return CPHVB_SUCCESS;
}

cphvb_error instruction_handler_print_simple(cphvb_instruction *inst) {
    // print the instruction and pipe them down to the simple
    // intepreter;
    
    print_instruction(inst);    
    return instruction_handler_simple(inst);        
}


cphvb_error instruction_handler_tree_builder(cphvb_instruction *inst) {
    // print the instruction and pipe them down to the simple
    // intepreter;
    
    print_instruction(inst); 
        
    ast* out_tree = new ast;
    cphvb_error result = create_ast_from_instruction(inst, out_tree);    
    //result = print_ast_node(out_tree);
    
    //std::cout << "tag:" << expr_type_to_string(inst_as_tree->tag) << "\n";
    //std::cout << "tag:" << inst_as_tree->id << "\n";
    //result = ssa_add(&_ssalist, cphvb_base_array(inst->operand[0]), &inst_as_tree);
       
    return instruction_handler_simple(inst);        
}



cphvb_error instruction_handler(cphvb_instruction *inst) {
    // the inst->status must be set on exit
    
    //return instruction_handler_tree_builder(inst);
    return instruction_handler_print_simple(inst);
}



 
cphvb_error cphvb_ve_jit_init(cphvb_component *self)
{
    // create a datastructure to handle the JIT stages.
    // set a  global variable with some stuff and remember to deallocate it
        
    _ssalist = std::map<cphvb_array*,ssavt*>();
    _attable = std::map<cphvb_array*,ast*>();

    // original
    myself = self;
    return CPHVB_SUCCESS;
    
    
}

/*
cphvb_error cphvb_ve_jit_execute( cphvb_intp instruction_count, cphvb_instruction* instruction_list )
{
    cphvb_intp count, nops, i;
    cphvb_instruction* inst;
    cphvb_error ret;

    for(count=0; count < instruction_count; count++)
    {
        inst = &instruction_list[count];        
        cphvb_opcode_text( inst->opcode);
        
    }   
    //cphvb_pprint_instr_list(instruction_list,instruction_count,".");
    
    return CPHVB_SUCCESS;

}
*/


cphvb_instruction create_customfunc_instr(cphvb_array* OUT, cphvb_array* 
            A,cphvb_array* B,cphvb_array* C,cphvb_array* D,cphvb_array* 
            E,cphvb_float64 cons, cphvb_intp ufunc_id) 
{
    
    cphvb_instruction inst;
    cphvb_jacstenc_type *rinst;
        
    void *mem = malloc(sizeof(cphvb_jacstenc_type));        
    if(rinst == NULL)
    {
        // not good
    }
    rinst = (cphvb_jacstenc_type *) mem;
        
    rinst->id = ufunc_id;
    rinst->nout        = 1;
    rinst->nin         = 5;
    rinst->struct_size = sizeof(cphvb_matmul_type);
    rinst->operand[0]  = OUT;
    rinst->operand[1]  = A;
    rinst->operand[2]  = B;
    rinst->operand[3]  = C;
    rinst->operand[4]  = D;
    rinst->operand[5]  = E;
    rinst->mult  = cons;
    
    inst.opcode        = CPHVB_USERFUNC;
    inst.userfunc      = (cphvb_userfunc *) rinst;
    
    return inst;
}


void testing_stuff() {
    
    //printf("Testing offset calculations:\n");
   
    
    //~ 
       //~ 
    //~ int itest[10];
    //~ memset(itest, 0, 10 * sizeof(int));
    //~ 
    //~ float ftest[10];
    //~ memset(ftest, 0, 10 * sizeof(float));
    //~ 
    //~ double dtest[10];
    //~ memset(dtest, 0, 10 * sizeof(double));
    //~ 
    //~ 
    //~ for(int i=0;i<10;i++) {
        //~ dtest[i] = (double) i+10;
        //~ itest[i] = (int) i+5;
        //~ ftest[i] = (float) i;
    //~ }
    //~ 
    //~ float* d0 = (float*) dtest; 
    //~ int* d1 = (int*) itest;
    //~ 
    //~ printf("double: %f , int: %d, float: %f\n",dtest[1],itest[2],ftest[3    ]);
    //~ //printf("%f\n", (double) *(d0+1sizeof(double)) );
    //~ printf("%f -- %d\n", (float) *(d0+1), (int) *(d1+3));
    //~ 
    //~ // &(d_out+off_out) = add(off_0+da_0, add( add(off_1+da_1, off_2+da_2), add(off_3+da_3,off_4+da_4)))    
    //~ //test_constant_to_string();
}


std::list<ast*>* instruction_list_to_ast(cphvb_instruction* instructionlist) {
    
    ast* result = new ast();
    int count,instruction_count;
    for(count=0; count < instruction_count; count++)
    {   
        
        
        // it makes no sense to call this function on instructions which are not
        // possible to build AST with.
        printf("-------- %d\n",cphvb_operands(instructionlist[count].opcode));
        if ( cphvb_operands(instructionlist[count].opcode) > 1) {
            create_ast_from_instruction(&instructionlist[count], result);
        }         
        
        // create statement ( arr = Exp )
        // lookup 
        
        
    }
}



cphvb_error cphvb_ve_jit_execute_split( cphvb_intp instruction_count, cphvb_instruction* instruction_list ) {        
    bool do_test = false;;
    cphvb_intp count;
    cphvb_error status;    
    
    
    //return CPHVB_SUCCESS;
    printf("\n\n------- Instr count = %d --------\n", instruction_count);
    cphvb_pprint_instr_list(instruction_list,instruction_count,"");
    printf("-------------------------------------\n\n");    
    ast_log("t",LOG_DEBUG);
    
    if (do_test) {
        printf("DOING TEST!!! \n");
        std::list<ast*>* expression_list = new std::list<ast*>();    
        std::map<cphvb_array*,ast*>* nametable = new std::map<cphvb_array*,ast*>();
        std::map<cphvb_array*,ssavt*>* ssa_nametable = new std::map<cphvb_array*,ssavt*>();
        
        for(count=0; count < instruction_count; count++)
        {
            //print_instruction(&instruction_list[count]);
            printf(">>> %d \n",count);        
            
            //printf("-------- %d\n",cphvb_operands(instruction_list[count].opcode));
            
            // it makes no sense to use the AST_HAndle function on instructions not suitable for building a AST.        
            if ( cphvb_operands(instruction_list[count].opcode) > 1) {    
                //ast_handle_instruction( expression_list, nametable, &instruction_list[count]);            
                ast_handle_instruction_ssa(expression_list, ssa_nametable, &instruction_list[count]);            
                //ast_handle_instruction_ssa_ua(_ssaua_table ,expression_list, ssa_nametable, &instruction_list[count]);            
                
            }        
        }
        
        printf("done with ast_handle_instruction for the whole instruction list\n");
        
        //printf("nametable size: %d\n", (int) (*ssa_nametable).size());        
                
        //std::list<ast*> astreelist = instruction_list_to_ast(instruction_list);
        
        return CPHVB_SUCCESS;
    }
    
    // =================================================================
    // =================================================================
    
        
    jit_ssa_map* jitssamap = new jit_ssa_map();
    jit_name_table* jitnametable = new jit_name_table();
    jit_execute_list* jitexecutelist = new jit_execute_list();    
    for(count=0; count < instruction_count; count++)
    {
        if(jita_is_controll(&instruction_list[count])) {
            
            jita_handle_controll_instruction(jitnametable,jitssamap,jitexecutelist,&instruction_list[count]);            
        } else {
            jita_handle_arithmetic_instruction(jitnametable,jitssamap,&instruction_list[count]);
        }                
    }
    
    
    
    printf("***  ssamap ***\n");
    printf("size = %d\n",(int)jitssamap->size());
    
    printf("***  nametable ***\n");
    printf("size = %d\n",(int)jitnametable->size());
    
    jit_name_entry* ent;
    for (int i=0; i < (int)jitnametable->size(); i++) {
        ent = jitnametable->at(i);
        printf("entry:[%d]  %p\n version %d\n expr* %p\n status %d\n ",i,ent->arrayp,ent->version,ent->expr,ent->status);
        _print_used_at(ent->used_at);
    }
    
    printf("\n*** execution list (%d)***\n[",(int)jitexecutelist->size());
    for(int i=0;i<jitexecutelist->size();i++) {
        printf("%d,",jitexecutelist->at(i));
    }    
    printf("]\n");
    
    jit_expr* e = jitnametable->at(jitexecutelist->at(0))->expr;
    cphvb_array* output_array = jitnametable->at(jitexecutelist->at(0))->arrayp;      
        
    jitcg_expr_codetext(output_array, e, jitssamap , "first_kernel");
    printf("\n\n");
    
    if (CPHVB_SUCCESS != cphvb_data_malloc(output_array)) {
        printf("ERROR: Failed to allocate return array.");
        return -1;
    }    
    
    kernel_function_first_kernel(output_array, e->op.expression.left->op.array, e->op.expression.right->op.array, 0, 0);    
    
    stringstream pss;                 
    jitcg_print_cphvb_array( (e->op.expression.left->op.array),5, &pss);    
    printf("%s\n",pss.str().c_str());
    pss.seekp(0);

    
    jitcg_print_cphvb_array( (e->op.expression.right->op.array),5, &pss);    
    printf("%s\n",pss.str().c_str());
    pss.seekp(0);
    
    jitcg_print_cphvb_array( output_array,5, &pss);
    printf("%s\n",pss.str().c_str());
    
    
    // =================================================================
    // =================================================================
        
    //~ cphvb_array*                arrayp;
    //~ cphvb_intp                  version;
    //~ jit_expr*                   expr;
    //~ jit_entry_status            status;
    //~ std::vector<cphvb_intp>*    used_at;
    
    
    
            
        
    for(count=0; count < instruction_count; count++)
    {        
        status = instruction_handler(&instruction_list[count]);
        if (status == CPHVB_OUT_OF_MEMORY) {
            return CPHVB_OUT_OF_MEMORY;
        } else if (status == CPHVB_ERROR) {
            break;
        }
    }
    
    
    
    //jita_run_tests();
    
    //testing_stuff();
    
    
    if (count == instruction_count) {
        return CPHVB_SUCCESS;
    } else {
        return CPHVB_PARTIAL_SUCCESS;
    }
}

cphvb_error cphvb_ve_jit_execute_org( cphvb_intp instruction_count, cphvb_instruction* instruction_list )
{
    cphvb_intp count, nops, i;
    cphvb_instruction* inst;
    cphvb_error ret;

    //cphvb_pprint_instr_list(instruction_list,instruction_count,".");    
    
    // match a patter of ADD,ADD,ADD,ADD,MULT
    //cphvb_instruction* temp_instr_list[5];    
    cphvb_opcode pattern[5] = {CPHVB_ADD,CPHVB_ADD,CPHVB_ADD,CPHVB_ADD,CPHVB_MULTIPLY};
    cphvb_intp step = 0;
    cphvb_intp misses = 0;
    
    
    for(count=0; count < instruction_count; count++)
    {
        inst = &instruction_list[count];
        
        //cphvb_opcode_text( inst->opcode);
        //cphvb_pprint_instr(inst);
        
        // possible to print the isntructions ???
                
        //printf("-- %s", cphvb_opcode_text( inst->opcode));
        
        
        //printf("misses %d  -  step: %d \n",misses,step);
        
        if(inst->opcode == pattern[step] && misses == 0) 
        {
            //printf(".");
            step++;
            if (step == 5) {
                // do_pattern_match_userfunc
                // extract the operands from the holded instructions
                // and call the userfunc
                //print_str(" Customfunc\n");
                
                /*
                cphvb_array* O = inst->operand[0];
                cphvb_array* A = ;
                cphvb_array* B = ;
                cphvb_array* C = ;
                cphvb_array* D = ;
                cphvb_array* E = ;
                cphvb_float64 cons = ;
                inst = create_customfunc_instr(O,A,B,C,D,E,con,jacstenc_impl_id);                                        
                */
                
                if (jacstenc_impl_id == 0) {                    
                    jacstenc_impl_id = 5;                    
                }
                
                
                void* rinsta = malloc(sizeof(cphvb_jacstenc_type));
                if(rinsta == NULL)
                {
                    printf("memory allocation error!!!");
                    return -1;
                }
                
                cphvb_jacstenc_type* rinst = (cphvb_jacstenc_type*) rinsta; 
                
                rinst->id = jacstenc_impl_id;
                rinst->nout        = 1;
                rinst->nin         = 5;
                rinst->struct_size = sizeof(cphvb_jacstenc_type);
                                
                cphvb_array* O = (&instruction_list[count-4])->operand[0];
                cphvb_array* A = (&instruction_list[count-4])->operand[1];
                cphvb_array* B = (&instruction_list[count-4])->operand[2];
                cphvb_array* C = (&instruction_list[count-3])->operand[2];
                cphvb_array* D = (&instruction_list[count-2])->operand[2];
                cphvb_array* E = (&instruction_list[count-1])->operand[2];
                
                cphvb_float64 cons = 0.2;//(cphvb_float64) ((&instruction_list[count])->constant.value);
                
                rinst->operand[0]  = O;
                rinst->operand[1]  = A;
                rinst->operand[2]  = B;
                rinst->operand[3]  = C;
                rinst->operand[4]  = D;
                rinst->operand[5]  = E;
                rinst->mult  =  cons;
                                
                cphvb_error output;                
                output = cphvb_compute_jacstenc((cphvb_userfunc*) rinst, NULL);
                
                if (output) {
                    // all operations status to
                    (&instruction_list[count-4])->status = CPHVB_INST_DONE;
                    (&instruction_list[count-3])->status = CPHVB_INST_DONE;
                    (&instruction_list[count-2])->status = CPHVB_INST_DONE;
                    (&instruction_list[count-1])->status = CPHVB_INST_DONE;
                    (&instruction_list[count-0])->status = CPHVB_INST_DONE;
                    
                } else {
                    // destroy world
                }
                
                step = 0;                
            }            
            continue;
        } else {
            
            // a mis in the pattern, reset
            if (step != 0 && misses == 0) 
            {                                
                //printf(" miss at step: %d\n", step);                                                                        
                //printf("c %d   s %d   m %d\n",count,step+1,step);
                
                // undo patternfit == reset count
                misses = step;
                count -= (step+1);
                step = 0;
                continue;
            }            
        }
        
        if (misses != 0) {
            misses--;
        }
        
        //printf("Ex: %s\n", cphvb_opcode_text(inst->opcode));


        if(inst->status == CPHVB_INST_DONE)     // SKIP instruction
        {
            continue;
        }

        nops = cphvb_operands(inst->opcode);    // Allocate memory for operands
        for(i=0; i<nops; i++)
        {
            if (!cphvb_is_constant(inst->operand[i]))
            {
                if (cphvb_data_malloc(inst->operand[i]) != CPHVB_SUCCESS)
                {
                    return CPHVB_OUT_OF_MEMORY; // EXIT
                }
            }

        }

        switch(inst->opcode)                    // Dispatch instruction
        {
            case CPHVB_NONE:                    // NOOP.
            case CPHVB_DISCARD:
            case CPHVB_SYNC:
                inst->status = CPHVB_INST_DONE;
                break;

            case CPHVB_USERFUNC:                // External libraries

                if(inst->userfunc->id == reduce_impl_id)
                {
                    ret = reduce_impl(inst->userfunc, NULL);
                    inst->status = (ret == CPHVB_SUCCESS) ? CPHVB_INST_DONE : ret;
                }
                else if(inst->userfunc->id == random_impl_id)
                {
                    ret = random_impl(inst->userfunc, NULL);
                    inst->status = (ret == CPHVB_SUCCESS) ? CPHVB_INST_DONE : ret;
                }
                else if(inst->userfunc->id == matmul_impl_id)
                {
                    ret = matmul_impl(inst->userfunc, NULL);
                    inst->status = (ret == CPHVB_SUCCESS) ? CPHVB_INST_DONE : ret;
                }
                else if(inst->userfunc->id == jacstenc_impl_id)
                {
                    ret = jacstenc_impl(inst->userfunc, NULL);
                    inst->status = (ret == CPHVB_SUCCESS) ? CPHVB_INST_DONE : ret;
                }
                
                else                            // Unsupported userfunc
                {
                    inst->status = CPHVB_USERFUNC_NOT_SUPPORTED;
                }

                break;

            default:                            // Built-in operations
                ret = cphvb_compute_apply( inst );
                inst->status = (ret == CPHVB_SUCCESS) ? CPHVB_INST_DONE : ret;
        }
        
        if (inst->status != CPHVB_INST_DONE)    // Instruction failed
        {
            break;
        }

    }

    if (count == instruction_count) {
        return CPHVB_SUCCESS;
    } else {
        return CPHVB_PARTIAL_SUCCESS;
    }

}

cphvb_error cphvb_ve_jit_execute( cphvb_intp instruction_count, cphvb_instruction* instruction_list ) {
    
    return cphvb_ve_jit_execute_split(instruction_count,instruction_list);
}


cphvb_error cphvb_ve_jit_shutdown( void )
{
    return CPHVB_SUCCESS;
}

cphvb_error cphvb_ve_jit_reg_func(char *lib, char *fun, cphvb_intp *id) 
{
    if(strcmp("cphvb_reduce", fun) == 0)
    {
    	if (reduce_impl == NULL)
    	{
			cphvb_component_get_func(myself, lib, fun, &reduce_impl);
			if (reduce_impl == NULL)
				return CPHVB_USERFUNC_NOT_SUPPORTED;

			reduce_impl_id = *id;
			return CPHVB_SUCCESS;			
        }
        else
        {
        	*id = reduce_impl_id;
        	return CPHVB_SUCCESS;
        }
    }
    else if(strcmp("cphvb_random", fun) == 0)
    {
    	if (random_impl == NULL)
    	{
			cphvb_component_get_func(myself, lib, fun, &random_impl);
			if (random_impl == NULL)
				return CPHVB_USERFUNC_NOT_SUPPORTED;

			random_impl_id = *id;
			return CPHVB_SUCCESS;			
        }
        else
        {
        	*id = random_impl_id;
        	return CPHVB_SUCCESS;
        }
    }
    else if(strcmp("cphvb_matmul", fun) == 0)
    {
    	if (matmul_impl == NULL)
    	{
            cphvb_component_get_func(myself, lib, fun, &matmul_impl);
            if (matmul_impl == NULL)
                return CPHVB_USERFUNC_NOT_SUPPORTED;
            
            matmul_impl_id = *id;
            return CPHVB_SUCCESS;			
        }
        else
        {
        	*id = matmul_impl_id;
        	return CPHVB_SUCCESS;
        }
    } 
    /*
    // Demo to test the maximum effect the a JIT grouping can achieve
    else if(strcmp("cphvb_jacstenc",fun) == 0) 
    {
        if (jacstenc_impl == NULL) 
        {
            cphvb_component_get_func(myself, lib, fun, &jacstenc_impl);
            if (jacstenc_impl == NULL)
                return CPHVB_USERFUNC_NOT_SUPPORTED;
        
            jacstenc_impl_id = *id;
            return CPHVB_SUCCESS;			
        }
        else
        {
            *id = jacstenc_impl_id;
            return CPHVB_SUCCESS;
        }        
    }
    */
    return CPHVB_USERFUNC_NOT_SUPPORTED;
}

cphvb_error cphvb_reduce( cphvb_userfunc *arg, void* ve_arg)
{
    return cphvb_compute_reduce( arg, ve_arg );
}

cphvb_error cphvb_random( cphvb_userfunc *arg, void* ve_arg)
{
    return cphvb_compute_random( arg, ve_arg );
}

cphvb_error cphvb_matmul( cphvb_userfunc *arg, void* ve_arg)
{
    return cphvb_compute_matmul( arg, ve_arg );    
}
/*
cphvb_error cphvb_jacstenc( cphvb_userfunc *arg, void* ve_arg)
{
    return cphvb_compute_jacstenc( arg, ve_arg );    
}
*/
