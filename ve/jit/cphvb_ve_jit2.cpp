/*
This file is part of cphVB and copyright (c) 2012 the cphVB team:
http://cphvb.bitbucket.org

cphVB is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 
of the License, or (at your option) any later version.

cphVB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the 
GNU Lesser General Public License along with cphVB. 

If not, see <http://www.gnu.org/licenses/>.
*/
#include <cphvb.h>
#include "cphvb_ve_jit2.h"
#include "cphvb_mcache.h"

#include "jit_codegenerator.h"
#include "jit_ast.h"
#include "jit_analyser.h"
#include "jit_compile.h"
#include "jit_ssa_analyser.h"
#include "jit_logging.h"
#include "jit_common.h"

#include <iostream>
#include <sstream>

static cphvb_component *myself = NULL;
static cphvb_userfunc_impl reduce_impl = NULL;
static cphvb_intp reduce_impl_id = 0;
static cphvb_userfunc_impl random_impl = NULL;
static cphvb_intp random_impl_id = 0;
static cphvb_userfunc_impl matmul_impl = NULL;
static cphvb_intp matmul_impl_id = 0;

jit_ssa_map* jitssamap;
jit_name_table* jitnametable;
jit_execute_list* jitexecutelist;
cphvb_instruction* jit_prev_instruction;

cphvb_error cphvb_ve_jit_init(cphvb_component *self)
{
    myself = self;
    cphvb_mcache_init( 10 );        
    jitssamap = new jit_ssa_map();
    jitnametable = new jit_name_table();
    jitexecutelist = new jit_execute_list(); 
    jit_prev_instruction = NULL;
    
    return CPHVB_SUCCESS;
}

cphvb_error execute_simple(cphvb_intp instruction_count, cphvb_instruction* instruction_list) {
    cphvb_intp count;
    cphvb_instruction* inst;
    cphvb_error res;

    for (count=0; count < instruction_count; count++) {

        inst = &instruction_list[count];
        if (inst->status == CPHVB_SUCCESS) {        // SKIP instruction
            continue;
        }

        res = cphvb_mcache_malloc( inst );          // Allocate memory for operands
        if ( res != CPHVB_SUCCESS ) {
            printf("Unhandled error returned by cphvb_mcache_malloc() called from cphvb_ve_simple_execute()\n");
            return res;
        }
                                                    
        switch (inst->opcode) {                     // Dispatch instruction

            case CPHVB_NONE:                        // NOOP.
            case CPHVB_DISCARD:
            case CPHVB_SYNC:
                inst->status = CPHVB_SUCCESS;
                break;
            case CPHVB_FREE:                        // Store data-pointer in malloc-cache
                inst->status = cphvb_mcache_free( inst );
                break;

            case CPHVB_USERFUNC:                    // External libraries

                if (inst->userfunc->id == reduce_impl_id) {

                    inst->status = reduce_impl(inst->userfunc, NULL);

                } else if(inst->userfunc->id == random_impl_id) {

                    inst->status = random_impl(inst->userfunc, NULL);

                } else if(inst->userfunc->id == matmul_impl_id) {

                    inst->status = matmul_impl(inst->userfunc, NULL);

                } else {                            // Unsupported userfunc
                
                    inst->status = CPHVB_USERFUNC_NOT_SUPPORTED;
                }

                break;

            default:                            // Built-in operations                                        
                inst->status = cphvb_compute_apply( inst );

        }

        if (inst->status != CPHVB_SUCCESS) {    // Instruction failed
            break;
        }
    }

    if (count == instruction_count) {
        return CPHVB_SUCCESS;
    } else {
        return CPHVB_PARTIAL_SUCCESS;
    }
}



cphvb_error jit_analyse_instruction_list(jit_ssa_map* jitssamap, 
        jit_name_table* jitnametable, cphvb_intp instruction_count, 
        cphvb_instruction* instruction_list) {
    
    int count;
    cphvb_instruction* inst;
    for(count=0; count < instruction_count; count++) { 
        inst = &instruction_list[count];                
        switch (inst->opcode) {                     // Dispatch instruction

            case CPHVB_NONE:                        // NOOP.
            case CPHVB_DISCARD:
            case CPHVB_SYNC:
            case CPHVB_FREE:                        // Store data-pointer in malloc-cache
            case CPHVB_USERFUNC:
                break;
                
            default:
                printf("tets");
                jita_handle_arithmetic_instruction(jitnametable,jitssamap,&instruction_list[count]);
                break;
        }        
    }
    
    printf("%s",jit_pprint_nametable(jitnametable).c_str());
}


cphvb_error cphvb_ve_jit_execute( cphvb_intp instruction_count, cphvb_instruction* instruction_list )
{
    bool output_testing = true;
    // Debug - print the execution list
    
    
    if (output_testing) {
        cphvb_pprint_instr_list(instruction_list,instruction_count,"Testing!"); 
        printf("Running Analyser\n");
        jit_analyse_instruction_list(jitssamap,jitnametable,instruction_count,  instruction_list); 
        printf("Executing with simple\n");
        execute_simple(instruction_count,instruction_list);
        return CPHVB_SUCCESS;
    }
    
    cphvb_intp count;
    cphvb_instruction* inst;
    
       
    for(count=0; count < instruction_count; count++)
    {   
        inst = &instruction_list[count];
        if (inst->status == CPHVB_SUCCESS) {        // SKIP instruction
            continue;
        }
             
        
        printf("++ %s\n",cphvb_opcode_text(inst->opcode));
        // is controll instruction
        if (inst->opcode == CPHVB_SYNC) {
        
            // execute             
            cphvb_intp name = jita_ssamap_version_lookup(jitssamap,inst->operand[0],-1);
            jit_name_entry* ne = jita_nametable_lookup(jitnametable,name);          
            
            if (CPHVB_SUCCESS != cphvb_data_malloc(ne->arrayp)) {
                printf("ERROR: Failed to allocate return array.");
                return CPHVB_ERROR;
            }                     
                                                                            
            test_computation(ne->arrayp,ne->expr,jitssamap,jitnametable);            
            
            stringstream pss; 
            printf("++++++++++++++++\nresult:\n");
            jitcg_print_cphvb_array( ne->arrayp,5, &pss);
            printf("%s\n\n",pss.str().c_str());
                        
            inst->status = CPHVB_SUCCESS;
            
        } else if (inst->opcode == CPHVB_FREE) {                    
            // add the array to the list of allocations to be free'd (only makes sense if they have been allocated in the first place)
            // inst->status = cphvb_mcache_free( inst );
        } else if (inst->opcode == CPHVB_DISCARD) {            
        
            
        // instruction is userfunction
        } else if (inst->opcode == CPHVB_USERFUNC){        
            
            if (inst->userfunc->id == reduce_impl_id) {
                inst->status = reduce_impl(inst->userfunc, NULL);
            } else if(inst->userfunc->id == random_impl_id) {
                inst->status = random_impl(inst->userfunc, NULL);
            } else if(inst->userfunc->id == matmul_impl_id) {
                inst->status = matmul_impl(inst->userfunc, NULL);
            } else {                            // Unsupported userfunc             
                inst->status = CPHVB_USERFUNC_NOT_SUPPORTED;
            }
        
        // is math operation          
        } else {                    
                                
        } 
        
        printf("setting prev_instr\n");       
        jit_prev_instruction = &instruction_list[count];
        
        
        
        
        
        
        

        
                           
    }
    
    return CPHVB_SUCCESS;   
    //return execute_simple(instruction_count,instruction_list);    
}

cphvb_error cphvb_ve_jit_shutdown( void )
{
    // De-allocate the malloc-cache
    cphvb_mcache_clear();
    cphvb_mcache_delete();

    return CPHVB_SUCCESS;
}

cphvb_error cphvb_ve_jit_reg_func(char *fun, cphvb_intp *id) 
{
    if(strcmp("cphvb_reduce", fun) == 0)
    {
    	if (reduce_impl == NULL)
    	{
			cphvb_component_get_func(myself, fun, &reduce_impl);
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
			cphvb_component_get_func(myself, fun, &random_impl);
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
            cphvb_component_get_func(myself, fun, &matmul_impl);
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
