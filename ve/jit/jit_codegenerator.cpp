//jit_codegenerator.cpp



#include "jit_codegenerator.h"
#include <string>

// this is to become a string, in-template piece of code!! 
cphvb_error traverser_expr (cphvb_array* A_o, cphvb_array* A_0,cphvb_array* A_1) {
    
    //Instr opcode_func;                          // Element-wise functor-pointer
    //cphvb_array *a0 = instr->operand[0],        // Array-Operands
    //            *a1 = instr->operand[1];
                                                // Pointers to start of data elements
                                                
    
    
    // extraction and type casting of input     
    // A_[name]->type* da_[name] = (A_[name]->type*) cphvb_base_array(A_[name])->data;    
    // sprintf(strbuff,"A_%s->type* da_%s = (A_%s->type*) cphvb_base_array(A_%s)->data;",name,name,name,name);
            
    A_o->type* da_o = (A_o->type*) cphvb_base_array(A_o)->data;        
    A_0->type* da_1 = (A_0->type*) cphvb_base_array(A_0)->data;    
    A_1->type* da_2 = (A_1->type*) cphvb_base_array(A_1)->data;    
    
    // for constants (pass by value ok?)
    // C_[name].type* dc_[name] = (C_[name].type*) &(C_[name].constant.value)
    //sprintf(strbuff,"C_%s.type* dc_%s = (C_%s.type*) &(C_%s.constant.value)",name,name,name,name);
    // T2* d2 = (T2*) &(instr->constant.value);

    // array's only
    //sprintf(strbuff,"assert(da_%s != NULL);",name);
    // constant
    //sprintf(strbuff,"assert(dc_%s != NULL);",name);

    assert(d0 != NULL);                         // Ensure that data is allocated
    assert(d1 != NULL);


    // array's only    
    // sprintf(strbuff,"cphvb_index off_%s;",name);
    // last_dim = a_%s    
    // sprintf(strbuff,"cphvb_index nelements = (%d>0) ? %d : cphvb_nelements( %s->ndim, %s->shape );",limit,limit,name,name);
    
    cphvb_index j,                              // Traversal variables
                last_dim = a0->ndim-1,
                off0, off1,
                nelements = (limit>0) ? limit : cphvb_nelements( a0->ndim, a0->shape ),
                ec = 0;

    cphvb_index coord[CPHVB_MAXDIM];
    memset(coord, 0, CPHVB_MAXDIM * sizeof(cphvb_index));

    if (skip>0)                                 // Create coord based on skip
        while(ec<skip)
        {
            ec += a0->shape[last_dim];
            for(j = last_dim-1; j >= 0; --j)
            {
                coord[j]++;
                if (coord[j] < a0->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }

    while( ec < nelements )
    {
        off0 = a0->start;                           // Compute offset based on coord
        off1 = a1->start;
        for( j=0; j<last_dim; ++j)
        {
            off0 += coord[j] * a0->stride[j];
            off1 += coord[j] * a1->stride[j];
        }

        for( j=0; j < a0->shape[last_dim]; j++ )    // Iterate over "last" / "innermost" dimension
        {
            opcode_func( (off0+d0), (off1+d1), d2 );

            off0 += a0->stride[last_dim];
            off1 += a1->stride[last_dim];
        }
        ec += a0->shape[last_dim];

        for(j = last_dim-1; j >= 0; --j)            // Increment coordinates for the remaining dimensions
        {
            coord[j]++;
            if (coord[j] < a0->shape[j]) {          // Still within this dimension
                break;
            } else {                                // Reached the end of this dimension
                coord[j] = 0;                       // Reset coordinate
            }                                       // Loop then continues to increment the next dimension
        }

    }

    return CPHVB_SUCCESS;







jit_expr* compute(jit_expr* e1, jit_expr* e2, cphvb_opcode) {
    
    // case the const and array status of e1 and e2
    
    // call type generic function with the variables
    
    
    
    
} 


char* jitcg_generate_inner_func_text(jit_expr* expr, std::vector<jit_expr_*> list, cphvb_intp* Acount, cphvb_intp* Ccount) {
    
    if(expr->tag == const_val) {
        
        
        
        (*Ccount)++; //*Ccount = (*cCount)+1;
    }
    
    if(expr->tag == array_val) {
        (*Acount)++
    }
    
    
    
}



traverse_loop(loop_var_name, )



/**
 * take and expression and a target array and generate code for this
 **/
char* jitcg_generate_code(jit_expr* expr) {
    int loglevel = 4;
    int ERROR=1,WARNINIG=2,INFO=3,DEBUG=4;
    
    char* buff = "";
    
    if(loglevel >= INFO) printf("jitcg_generate_code()");    
    if(loglevel >= DEBUG) printf("Expression depth: %d",expr->depth);
    
    
    
    
    return buff ;
}








