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
#include "cphvb_vcache.h"

#include "jit_codegenerator.h"
#include "jit_ast.h"
#include "jit_analyser.h"
#include "jit_compile.h"
#include "jit_ssa_analyser.h"
#include "jit_logging.h"
#include "jit_common.h"
#include "jit_kernel.h"
#include "jit_kernel_cache.h"
#include "StringHasher.hpp"

#include <iostream>
#include <sstream>
#include <set>
#include <sys/time.h>

#include "cphvb_array.h"
#include "cphvb_type.h"
#include "cphvb.h"
#include <cmath>

#define KE_TIMEING 0

static cphvb_component *myself = NULL;
static cphvb_userfunc_impl reduce_impl = NULL;
static cphvb_intp reduce_impl_id = 0;
static cphvb_userfunc_impl random_impl = NULL;
static cphvb_intp random_impl_id = 0;
static cphvb_userfunc_impl matmul_impl = NULL;
static cphvb_intp matmul_impl_id = 0;

jit_ssa_map*                jitssamap;
jit_name_table*             jitnametable;
jit_base_dependency_table*  jitbasedeptable;
jit_analyse_state*          jitanalysestate;

jit_kernel_cache*           jitkernelcache;
jit_compute_functions*      jitcomputefunctions;
jit_execute_list*           jitexecutelist;

cphvb_instruction*          jit_prev_instruction;
cphvb_index                 jitcompound_kernel_count;
cphvb_index                 jitinstr_list_count;
cphvb_index                 last_nametable_size;

cphvb_index cache_hit = 0;  
cphvb_index cache_miss = 0;



using namespace std;

/**
 * count the number of operations (binary / unary)
 * ignores executed expression (and following subexpression)
 * ignores identity
 */
 
cphvb_intp expr_opcount(jit_analyse_state* s, jit_expr* expr) {
    cphvb_intp opcount = 0;
    if(is_bin_op(expr)) {
        opcount++;
        if (!jita_nametable_lookup(s->nametable, expr->op.expression.left->name)->is_executed) {
            opcount += expr_opcount(s,expr->op.expression.left);
        }
        if (!jita_nametable_lookup(s->nametable, expr->op.expression.right->name)->is_executed) {
            opcount += expr_opcount(s,expr->op.expression.right);
        }
        
    } else if (is_un_op(expr)) {
        opcount++;
        if (! (jita_nametable_lookup(s->nametable, expr->op.expression.left->name)->is_executed)) {
            opcount += expr_opcount(s,expr->op.expression.left);
        }
    }    
    return opcount;
}

/**
 * Calculates a number to compare expression complexity.
 * num_of_operations * array size. 
 **/
cphvb_intp expr_computational_complexity(jit_analyse_state* s, jit_expr* expr) {
    // number of operations in the expression.
    
    cphvb_intp opcount = expr_opcount(s,expr);
    jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);    
    // if the target array for the expression, does not have a view (is a base),
    // the number of

    cphvb_intp accessed_elements = cphvb_nelements(entr->arrayp->ndim,entr->arrayp->shape);
    //printf("acc.elems: %ld \n",accessed_elements);
    //printf("opcount  : %ld \n",opcount);

    
    return accessed_elements * opcount;    
}

/**
 * function which finds dependency violations in and expr, and splits it into a list sub expr
 **/
void expr_dependecy_travers3(jit_analyse_state* s, jit_expr* expr, set<cphvb_intp>* execute_list, set<cphvb_intp>* grand_parent_DON) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"expr_dependecy_travers3(%ld)\n",expr->name);
    if (is_array(expr)) {
        logcustom(cloglevel,1,"expr_dependecy_travers3(Array:%ld)\n",expr->name);
         return;
    }
    
    jit_expr* echild;
    jit_name_entry* entr_child;
    jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);

    entr->dep_trav_visited = true;
    /* comments
    // when not to break a branch in the expression tree? BA = BoundArray
    // If the parent has no DONs, childs will always be attached.
    // If childs are arrays or constants
    // If childs have no DON
    // if P->DONs != C->DONs, depend-on BA different between parent and child
    // if P->DONs == C

    // when to break branch
    // if P->DONs == C->DONs && P->DONs->names > C->DONs->names, same depend-on BA and C->DON name are smaller then P->DON name. 
    // if C->DTO > 1 , if C in two or more P->DONs, it
    // if C->DTO == 1 , and C-DTO < P->DON, and      
    */
    if (is_userfunc(expr)) {
        logcustom(cloglevel,1,"EDT3 is userfun  (%d). returning.\n",expr->name);
        execute_list->insert(expr->name);
        return;
    }

    // if the entry is not discarded in this instructionlist, it must be used in someway in the following and must thus be executed.
    // it is not possible to know how it is used. If it is a root node, it will automatically be added.
    // The entr must not be and bounded array, as these are not discarded before the end of the program.
    if (entr->discarded_at == -1 && entr->expr->parent != NULL && cphvb_base_array(entr->arrayp)->data == NULL) {
        logcustom(cloglevel,1,"EDT3 is not discarded in this batch (%d). returning.\n",expr->name);
        execute_list->insert(expr->name);        
        
    } else if (entr->discarded_at == -1 && cphvb_base_array(entr->arrayp)->data != NULL) {
        // if a bounded array is the last one written to, and not discarded it must be executed.
        cphvb_intp lastusage = jita_base_usage_table_get_usage(s->base_usage_table,cphvb_base_array(entr->arrayp))->back();            
                
        if (lastusage == entr->expr->name) {
            logcustom(cloglevel,1,"EDT3 is not discarded in this batch, is bounded and last use (%d) lastusage %d. returning.\n",entr->expr->name,lastusage);
            execute_list->insert(expr->name);            
        }
    }
    
    if (is_un_op(expr) || is_bin_op(expr)) {
        int children_num = is_bin_op(expr) ? 2 : 1;    
        jit_expr* children[2] = {expr->op.expression.left, expr->op.expression.right};
        logcustom(cloglevel,1,"children_num: %d \n",children_num);

        // do for both left and right expression. Left first.
        for(int i=0;i<children_num;i++) {                                                        
            echild = children[i];                        
            entr_child = jita_nametable_lookup(s->nametable,echild->name);

            //printf("Child is expression\n",echild->name);
            //printf( (is_un_op(echild) || is_bin_op(echild)) ? "exp ": "val ");
            //print_ast_node(echild); 
            //printf("after copy %ld\n",echild->name);
            if( (is_un_op(echild) || is_bin_op(echild))  ) {                
                if (entr_child->tdto->size() > 1) {                
                    // if child is depended on by more then one.
                    execute_list->insert(echild->name);            
                    logcustom(cloglevel,1,"Adding to execute:1: %ld\n",echild->name);
                    expr_dependecy_travers3(s,echild,execute_list, grand_parent_DON);
                    return;                
                        
                } else {                    
                    //vector<cphvb_intp>* dep_table = jita_base_usage_table_get_usage(s->base_usage_table,cphvb_base_array(entr_child->arrayp));
                    //printf("%p %ld\n",dep_table, echild->name);
                     
                    if (cphvb_base_array(entr_child->arrayp)->data != NULL && echild->name == jita_base_usage_table_get_usage(s->base_usage_table,cphvb_base_array(entr_child->arrayp))->back()) {
                        // child is the last update to the BoundedArray.
                        execute_list->insert(echild->name);     
                        logcustom(cloglevel,1,"Adding to execute:2: %ld\n",echild->name);
                        expr_dependecy_travers3(s,echild,execute_list, grand_parent_DON);
                        return;
                    }                    
                }
                
                set<cphvb_intp>::iterator itp,itc;
                //copy parent to grandparent
                for (itp=entr->tdon->begin(); itp!=entr->tdon->end(); itp++) {                    
                    grand_parent_DON->insert(*itp);
                }
                
                // get dependency 
                if(entr_child->tdon->size() > 0 ) {                                 
                    if (entr->tdon->size() > 0) {                                                                
                        // check parent and child dependencies for overlaps
                        for (itc=entr_child->tdon->begin(); itc!=entr_child->tdon->end(); itc++) {                    
                            for (itp=entr->tdon->begin(); itp!=entr->tdon->end(); itp++) {                    
                                                            
                                if (*itp == *itc) {
                                    // depend on same                            
                                    continue;
                                }
                                if (*itp == echild->name) {
                                    // echild DTO.size == 1 and dto == parent
                                    grand_parent_DON->erase(*itp);
                                    continue;
                                }
                                
                                if (cphvb_base_array(jita_nametable_lookup(s->nametable,*itp)->arrayp) == cphvb_base_array(jita_nametable_lookup(s->nametable,*itc)->arrayp)) {
                                    if (*itp > *itc) {
                                        // parent is dependent on a higher version of the BoundedArray.
                                        // cutting closest to expression dependency conflict
                                        execute_list->insert(echild->name);
                                        logcustom(cloglevel,1,"Adding to execute:3: %ld\n",echild->name);
                                        grand_parent_DON->erase(*itp);                                                                                    
                                    }                            
                                }
                            }
                        } // end double forloop.                        
                    }                                                        
                    
                    if (grand_parent_DON->size() > 0) {
                        // check vs. grandparents DON
                        for (itc=entr_child->tdon->begin(); itc!=entr_child->tdon->end(); itc++) {            
                            for (itp=grand_parent_DON->begin(); itp!=grand_parent_DON->end(); itp++) {
                                if (*itp == *itc) {
                                    // depend on same
                                    continue;
                                }
                                if (*itp == echild->name) {
                                    // echild DTO.size == 1 and dto == grandparent
                                    // grand parent DON == child name
                                    grand_parent_DON->erase(*itp);
                                    continue;
                                }
                                
                                if (cphvb_base_array(jita_nametable_lookup(s->nametable,*itp)->arrayp) == cphvb_base_array(jita_nametable_lookup(s->nametable,*itc)->arrayp)) {
                                    if (*itp > *itc) {
                                        // a grand parent is dependent on a higher version of the BoundedArray.
                                        // cutting closest to expression dependency conflict
                                        execute_list->insert(echild->name);
                                        logcustom(cloglevel,1,"Adding to execute:4: %ld",echild->name);
                                        grand_parent_DON->erase(*itp);                              
                                    }                            
                                }
                            }
                        }
                    }
                }
            } // end expression check
            logcustom(cloglevel,1,"Investigating: %ld\n",echild->name);
            expr_dependecy_travers3(s,echild,execute_list, grand_parent_DON);        
        } // end for loop        
    } // if array or constant the traveresal stops.
}



/**
 * this functions takes an array and from this determines which expressions must be
 * executed and in what order. a list of expressions are generated based on the
 * analyse_state tables. This will contain a copy of the existing structure, but point
 * to the existing data.
 **/
set<cphvb_intp>* generate_execution_list(jit_analyse_state* s) {
    bool cloglevel[] = {0,0};
    logcustom(cloglevel,0,"Generate_execution_list()\n");
    
    set<cphvb_intp>* exelist = new set<cphvb_intp>();
    for(int i=s->nametable->size()-1;i>=0;i--) {        
        if (!s->nametable->at(i)->dep_trav_visited && !is_array(s->nametable->at(i)->expr)) {
            //printf("DepTravers: %d\n",i);
            expr_dependecy_travers3(s,s->nametable->at(i)->expr,exelist,new set<cphvb_intp>());
            logcustom(cloglevel,1,"GEL add to executelist (%d)\n",i);
            exelist->insert(i);
        }        
    }    
    return exelist;
}


/**
 * Parsing each element of the instructionlist.
 * Using jita_handle-functions to handle the different types of functions.
 **/            
void jit_analyse_instruction_list(
        jit_analyse_state* s,        
        cphvb_instruction* instruction_list,
        cphvb_intp instruction_count) {

    int count;
    cphvb_instruction* inst;        

    jit_base_dependency_table::iterator it;
    std::vector<cphvb_intp>::iterator itvec;        
            
    for(count=0; count < instruction_count; count++) { 
        inst = &instruction_list[count];                
        switch (inst->opcode) {                     // Dispatch instruction

            case CPHVB_NONE:                        // NOOP.
            case CPHVB_DISCARD:
                jita_handle_discard_instruction(s,&instruction_list[count],count);
                break;
            case CPHVB_SYNC:                
                // will always be the last element in a list
                // This could be catched and used in a SYNC-as-execution-base approach to kernelcreation and exectution.
                break;
            case CPHVB_FREE:                        // Store data-pointer in malloc-cache
                jita_handle_free_instruction(s,&instruction_list[count],count);
                break;
            case CPHVB_USERFUNC:                
                jita_handle_userfunc_instruction(s,&instruction_list[count],count);                
                break;                
            default:                
                jita_handle_arithmetic_instruction3(s,&instruction_list[count],count);                
                break;
        }        
    }    
}


/**
 * ensure that free'd arrays are freed.
 **/
void cleanup_free(cphvb_instruction* list, cphvb_index instr_count) {
    for(int i=0;i<instr_count;i++) {
        if(list[i].opcode == CPHVB_FREE) {            
            jit_vcache_free(list[i].operand[0]);
        }
    }    
}

/**
 * Initialize the JIT VE.
 * setting up state which must be present cross instructionlist.
 **/
cphvb_error cphvb_ve_jit_init(cphvb_component *self) {
    myself = self;
    cphvb_vcache_init( 10 );
     
    jitssamap = new jit_ssa_map();
    jitnametable = new jit_name_table();    
    jitbasedeptable = new jit_base_dependency_table();
    jitanalysestate = jita_make_jit_analyser_state(jitnametable,jitssamap,jitbasedeptable);

    jitcomputefunctions = new jit_compute_functions();
    jitcomputefunctions->instr_compute = cphvb_compute_apply;
    jitcomputefunctions->userfunctions = new map<cphvb_intp,cphvb_userfunc_impl>();
    //jitexecutelist = new jit_execute_list(); 
    jitkernelcache = new jit_kernel_cache();
    jitcompound_kernel_count = 0;
    
    jitinstr_list_count = 0;
    last_nametable_size = 0;
        
    jit_prev_instruction = NULL;
    
    return CPHVB_SUCCESS;
}


/**
 * Function called to exectue this VE. 
 **/
cphvb_error cphvb_ve_jit_execute( cphvb_intp instruction_count, cphvb_instruction* instruction_list ) {
    
    //cphvb_pprint_instr_list(instruction_list,instruction_count,"Testing!");
    //cphvb_pprint_instr_list_small(instruction_list,instruction_count,"Testing!");
    bool cloglevel[] = {0,0,0};
    //bool clean_up_list = false; // true if the instruction list holds no arithmetic instructions. (old.nametable.size() == new.nametable.size())
    //bool put_in_cache = false;
    
    // true if the list is split by a fixed number. If the last element is not a SYNC operation it could be split 
    bool connected_instruction_list = false;//(instruction_list[instruction_count-1].opcode != CPHVB_SYNC); 
    if(connected_instruction_list) {
       /// todo: might want to handle this case (or just restrict the VEM to never to split up instructionlists.)
       printf("* * * * Connected Instruction list detected. Nametable should not be reset.\n");
       return 0;  
    } // else 

    timespec time1, time2;
    if (KE_TIMEING > 0) { 
        clock_gettime(CLOCK_REALTIME, &time1);    
    }    
    jit_compound_kernel* compound_kernel = NULL;        
    // cache lookup.
    cphvb_intp instr_list_hash = instructionlist_hash(instruction_list,instruction_count);
    compound_kernel = jit_kernel_cache_lookup(jitkernelcache,instr_list_hash);            
    set<cphvb_intp>* execution_list = NULL;

    (compound_kernel != NULL) ? cache_hit++ : cache_miss++ ;    
    //printf("Time taken is: %ld.%ld\n ",tS.tv_sec, tS.tv_nsec);

    
    logcustom(cloglevel,0,"=== Kernel from cache: %p  H: %ld\n",compound_kernel, instr_list_hash);            
    // cache miss
    if (compound_kernel == NULL) {        
        jitssamap = new jit_ssa_map();
        jitnametable = new jit_name_table();    
        jitbasedeptable = new jit_base_dependency_table();
        jitanalysestate = jita_make_jit_analyser_state(jitnametable,jitssamap,jitbasedeptable);
        
        // setup                                
        /// do analysis - check on free/discard only isntruction list.
        jit_analyse_instruction_list(jitanalysestate,instruction_list,instruction_count);            
        logcustom(cloglevel,0,"== Instruction-list initial analysis done\n");
                    
        //logcustom(cloglevel,2,"last_nametable_size %ld\n",last_nametable_size);
        /// do dependency analysis
        jita_perform_dependecy_analysis(jitanalysestate,0);
        logcustom(cloglevel,0,"== Dependency analysis done\n");

        if (cloglevel[1]) {
            jit_pprint_base_dependency_table(jitanalysestate->base_usage_table);            
            jit_pprint_nametable_dependencies(jitanalysestate->nametable);
            jit_pprint_nametable(jitanalysestate->nametable); printf("\n");

        }
        /// build execution list
        execution_list = generate_execution_list(jitanalysestate);
        logcustom(cloglevel,0,"== Executionlist created %p, size()=%ld\n",execution_list,execution_list->size());
        if(cloglevel[1]) {jit_pprint_set(execution_list);}
        
        if (execution_list->size() > 0) {
                    
            // in the creation of the compoundkernel and executionkernels the executionkernels are bounded to the data of the
            // insturctionlist used to create the kernels. This is and uptimization step and as it is needed to visit the
            // input arrays and constants to determin their position in the instructionlist revisiting the same operands again
            // would be inefficient.
            /// create compund kernel with creation execution kernels.
            compound_kernel = (jit_compound_kernel*) malloc(sizeof(jit_compound_kernel));                            
            compound_kernel->exekernels = (jit_execute_kernel**) malloc(sizeof(jit_execute_kernel*)*execution_list->size());
            compound_kernel->kernels_length = execution_list->size();                    
            cphvb_intp buildresult = build_compound_kernel(jitanalysestate, execution_list, jitcompound_kernel_count, compound_kernel);
            
            logcustom(cloglevel,0,"== Compund kernel created. res: %ld  P: %p, \n", buildresult, compound_kernel);
            if(cloglevel[2]) {
                printf("===================================\n");
                printf("executionlist");jit_pprint_set(execution_list);
                jit_pprint_nametable(jitanalysestate->nametable);
                jit_pprint_nametable_dependencies(jitanalysestate->nametable);
                jit_pprint_base_dependency_table(jitanalysestate->base_usage_table);                        
            }
            // increment the compound_kernel_id.
            jitcompound_kernel_count++;
        }
        
    }

    if (KE_TIMEING > 0) {            
        clock_gettime(CLOCK_REALTIME, &time2);  
        printf("start to bind    %ld : %d\n",diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec)); 
    }
    // execute the 
    if (compound_kernel != NULL) {
        /// execute ckernel
        logcustom(cloglevel,0,"== Bind\n");
        // bind execute_kernels with instruction list
        bind_compound_kernel(compound_kernel,instruction_list,jitinstr_list_count);
        logcustom(cloglevel,0,"== Execute\n");
        // execute execute kernels
        if (KE_TIMEING > 0) {
            timespec time3;
            clock_gettime(CLOCK_REALTIME, &time3);  
            printf("start to execute %ld : %d  (%ld : %d) \n",diff(time1,time2).tv_sec, (diff(time1,time2).tv_nsec),diff(time2,time3).tv_sec, (diff(time2,time3).tv_nsec) ); 
        }
        execute_compound_kernel(jitcomputefunctions,compound_kernel,instruction_list);
        logcustom(cloglevel,0,"== Cache Add\n");
        // if execution happened without errors, the compund_kernel is stored in the cache
        jit_kernel_cache_insert(jitkernelcache,instr_list_hash,compound_kernel);
        
    } else if(execution_list != NULL && execution_list->size() > 0){
        if(cloglevel[1]) {jit_pprint_set(execution_list);}
        // return error!! ckernel == NULL and is_error
        printf("something horrible happened! No compound kernel created for instruction list\n");    
    } 
    
    /// perform cleanup
    // go through the used arrays and ensure that discarded and free'd temp arrays are marked as unused.
    cleanup_free(instruction_list,instruction_count);

    /// Before return
    jitinstr_list_count++;
    last_nametable_size = instruction_count;        
    
    //printf("Running Analyser\n");
    //jit_analyse_instruction_list(jitssamap,jitnametable,jitbasedeptable,instruction_count,  instruction_list); 
                    
    //printf("Executing with simple\n\n\n\n");
    //execute_simple(instruction_count,instruction_list);
    //cphvb_intp hash = instructionlist_hash(instruction_list,instruction_count);
    //printf("hsah of the instructionlist: %ld\n",hash);
            
    return CPHVB_SUCCESS;    
}

/**
 **/
cphvb_error cphvb_ve_jit_shutdown( void )
{
    // De-allocate the malloc-cache
    cphvb_vcache_clear();
    cphvb_vcache_delete();

    // De-allocate state
    
    //printf("Instruction list computed: %ld\n", jitinstr_list_count );
    //printf("Kernel cache: hits %ld , misses: %ld\n", cache_hit, cache_miss);        
    
    return CPHVB_SUCCESS;
}


/**
 **/
cphvb_error cphvb_ve_jit_reg_func(char *fun, cphvb_intp *id) {
    
    map<cphvb_intp,cphvb_userfunc_impl>::iterator it;
    cphvb_userfunc_impl userfunc_impl;

    //printf("%ld\n",*id);
    it = jitcomputefunctions->userfunctions->find(*id);                
    if(it == jitcomputefunctions->userfunctions->end()) {
        // function not registered            
        cphvb_component_get_func(myself, fun, &userfunc_impl);
        if (userfunc_impl == NULL)
            return CPHVB_USERFUNC_NOT_SUPPORTED;
        if (jitcomputefunctions->userfunctions->insert(pair<cphvb_intp,cphvb_userfunc_impl>(*id,userfunc_impl)).second) {
            return CPHVB_SUCCESS;                
        }
        return CPHVB_ERROR;
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
