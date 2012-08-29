
//jit_codegenerator.cpp

#include <stdio.h>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <string>
#include "jit_codegenerator.h"
#include "jit_analyser.h"
#include "cphvb.h"
#include <map>




using namespace std;
const char* cphvb_type_typetext(cphvb_type type);

template <typename T1>
void _jitcg_print_cphvb_array(cphvb_array* a0, cphvb_intp limit, stringstream* ss) {    
    T1* d0 = (T1*) cphvb_base_array(a0)->data;    
    
    cphvb_index j,                              
                last_dim = a0->ndim-1,
                off0,
                nelements = (limit>0) ? limit : cphvb_nelements( a0->ndim, a0->shape ),
                ec = 0;

    cphvb_index coord[CPHVB_MAXDIM];
    memset(coord, 0, CPHVB_MAXDIM * sizeof(cphvb_index));
    //*ss << "test ";
    //printf("%d %d %d %d\n",(int)ec,(int)nelements,(long)a0->ndim, a0->shape[1]);
    while( ec < nelements )
    {
        //printf(". %d %d .\n",ec,last_dim);
        off0 = a0->start;                           // Compute offset based on coord
        for( j=0; j<last_dim; ++j)
        {
            off0 += coord[j] * a0->stride[j];
        }

        for( j=0; j < a0->shape[last_dim]; j++ )    // Iterate over "last" / "innermost" dimension
        {
            //printf("%d %p\n",off0,d0);
            //*ss << "(" << off0 << ")+(" << d0 << ") -- "; 
            *ss << ((T1) *(off0+d0)) << " ";
            
            off0 += a0->stride[last_dim];
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
    
    
}

void jitcg_print_cphvb_array(cphvb_array* a0, cphvb_intp limit, stringstream* ss) {
        
    //printf("%s\n",cphvb_type_typetext(a0->type));
    
    switch(a0->type) {
        case CPHVB_BOOL:
            _jitcg_print_cphvb_array<cphvb_bool>(a0,limit,ss);         
            break;   
        case CPHVB_INT8:
            _jitcg_print_cphvb_array<cphvb_int8>(a0,limit,ss);            
            break;   
        case CPHVB_INT16:
            _jitcg_print_cphvb_array<cphvb_int16>(a0,limit,ss);            
            break;   
        case CPHVB_INT32:
            _jitcg_print_cphvb_array<cphvb_int32>(a0,limit,ss);            
            break;   
        case CPHVB_INT64:
            _jitcg_print_cphvb_array<cphvb_int64>(a0,limit,ss);            
            break;   
        case CPHVB_UINT8:
            _jitcg_print_cphvb_array<cphvb_uint8>(a0,limit,ss);            
            break;   
        case CPHVB_UINT16:
            _jitcg_print_cphvb_array<cphvb_uint16>(a0,limit,ss);            
            break;   
        case CPHVB_UINT32:
            _jitcg_print_cphvb_array<cphvb_uint32>(a0,limit,ss);            
            break;   
        case CPHVB_UINT64:
            _jitcg_print_cphvb_array<cphvb_uint64>(a0,limit,ss);            
            break;   
        case CPHVB_FLOAT16:        
            _jitcg_print_cphvb_array<cphvb_float16>(a0,limit,ss);            
            break;   
        case CPHVB_FLOAT32:        
            _jitcg_print_cphvb_array<cphvb_float32>(a0,limit,ss);            
            break;   
        case CPHVB_FLOAT64:        
            _jitcg_print_cphvb_array<cphvb_float64>(a0,limit,ss);            
            break;   
        default:            
            break;   
    }    
}





const char* cphvb_type_typetext(cphvb_type type)
{
    switch(type)
    {
    case CPHVB_BOOL:
        return "cphvb_bool";
    case CPHVB_INT8:
        return "cphvb_int8";
    case CPHVB_INT16:
        return "cphvb_int16";
    case CPHVB_INT32:
        return "cphvb_int32";
    case CPHVB_INT64:
        return "cphvb_int64";
    case CPHVB_UINT8:
        return "cphvb_uint8";
    case CPHVB_UINT16:
        return "cphvb_uint16";
    case CPHVB_UINT32:
        return "cphvb_uint32";
    case CPHVB_UINT64:
        return "cphvb_uint64";
    case CPHVB_FLOAT16:
        return "cphvb_float16";
    case CPHVB_FLOAT32:
        return "cphvb_float32";
    case CPHVB_FLOAT64:
        return "cphvb_float64";    
    default:
        return "";
    }
}

//~ void expr_val_codetext_stream(jit_expr* expr_val, cphvb_intp name, ostringstream* oss) {    
    //~ printf("expr_val_codetext_stream()\n");
    //~ if(expr_val != NULL) {
        //~ printf("has value! = %d\n", (expr_val->tag));        
        //~ if (expr_val->tag == const_val) {
            //~ *oss << "C_" << (int)name;
        //~ } else if (expr_val->tag == array_val) {
            //~ *oss << "A_" << name;
        //~ }    
    //~ }
//~ }

void jitcg_codetext_opcode_stream(cphvb_opcode opcode, stringstream* oss) { 
    //printf("jitcg_codetext_opcode_stream()\n");   
    //printf("opdoce = %s\n",cphvb_opcode_text(opcode));
    
    switch(opcode) {
        case CPHVB_ADD:      *oss << "+"; break;
        case CPHVB_SUBTRACT: *oss << "-"; break;
        case CPHVB_MULTIPLY: *oss << "*"; break;
        case CPHVB_DIVIDE:   *oss << "/"; break;
        default: break;
        
    }   
}

void jitcg_expr_travers(jit_expr* expr, jit_ssa_map* ssamap, std::map<cphvb_intp,bool>* as, std::map<cphvb_constant*,cphvb_intp>* cs, 
                        stringstream* param_ss,stringstream* init_ss,stringstream* comp_ss, stringstream* oss1, stringstream* oss2, stringstream* oss3 ) {
                            

    if(is_bin_op(expr)) {    
        *comp_ss << "(";
        jitcg_expr_travers(expr->op.expression.left,ssamap,as,cs,param_ss,init_ss,comp_ss,oss1, oss2, oss3);
        
        jitcg_codetext_opcode_stream(expr->op.expression.opcode,comp_ss);  
                                   
        jitcg_expr_travers(expr->op.expression.right,ssamap,as,cs,param_ss,init_ss,comp_ss,oss1, oss2, oss3);       
        *comp_ss << ")";
        
    } else if (is_un_op(expr)) {        
        *comp_ss << "(";
        jitcg_expr_travers(expr->op.expression.left,ssamap,as,cs,param_ss,init_ss,comp_ss,oss1, oss2, oss3);        
        jitcg_codetext_opcode_stream(expr->op.expression.opcode,comp_ss);  
        *comp_ss << ")";
        
    } else if (is_array(expr)) {   
        // add comp as offset            
        *comp_ss << " *(off_a" << expr->name << "+da_" << expr->name << ")";
                
        // if not added before
        if(as->find(expr->name) == as->end()) {
            // adding to parameter string
            //*param_ss << "cphvb_array* A_" << expr->name << ",";            
            as->insert(pair<cphvb_intp,bool>(expr->name,true));
                    
            // add init            
            // A_[name]->type* da_[name] = (A_[name]->type*) cphvb_base_array(A_[name])->data;    
            *init_ss << cphvb_type_typetext(expr->op.array->type) <<"* da_" << expr->name << " = (" << cphvb_type_typetext(expr->op.array->type) << "*) cphvb_base_array(A_" << expr->name << ")->data;\n";   
            *init_ss << "assert(da_" << expr->name << " != NULL);\n";
            *init_ss << "cphvb_index off_a" << expr->name << ";\n";
            
            // ad offset increment 1
            *oss1 << "off_a" << expr->name << " = A_" << expr->name << "->start;\n";
            
            // ad offset increment 2
            *oss2 << "off_a" << expr->name << " += coord[j] * A_" << expr->name << "->stride[j];\n";
            
            // ad offset increment 3
            *oss3 << "off_a" << expr->name << " += A_" << expr->name << "->stride[last_dim];\n";
        }
                                
    } else if (is_constant(expr)) {     
        // add comp as offset
        *comp_ss << " *dc_" << expr->op.constant ;
        if(cs->find(expr->op.constant) == cs->end()) {
            // adding to parameter string
            //*param_ss << "cphvb_array* A_" << expr->name << ",";            
            cs->insert(pair<cphvb_constant*,cphvb_intp>(expr->op.constant,1));    
                    
            *init_ss << cphvb_type_typetext(expr->op.constant->type) <<"* dc_" << expr->op.constant << " = (" << cphvb_type_typetext(expr->op.constant->type) << "*) &(C_" << expr->op.constant << "->value);\n";   
        }
        
    }
}


string traverse_function_text(char* fname, stringstream* param_ss, stringstream* init_ss, 
                                    stringstream* comp_ss, stringstream* oss1, stringstream* oss2, 
                                    stringstream* oss3) {
    stringstream txt;
    txt << "cphvb_error kernel_function_" << fname << "(" << param_ss->str() << ")\n{\n";
    txt << "cphvb_error status = CPHVB_ERROR;\n";
    
    // initialize
    txt << init_ss->str();
    txt << "" << "cphvb_index j,";
    txt << "" << "last_dim = A_out->ndim-1,";
    txt << "" << "nelements = (limit>0) ? limit : cphvb_nelements( A_out->ndim, A_out->shape ),";
    txt << "" << "ec = 0;\n";
    txt << "" << "cphvb_index coord[CPHVB_MAXDIM];";
    txt << "" << "memset(coord, 0, CPHVB_MAXDIM * sizeof(cphvb_index));\n";        
    
    txt << "if (skip>0)\n\twhile(ec<skip)\n\t\t{\n\t\tec += A_out->shape[last_dim];\n\t\tfor(j = last_dim-1; j >= 0; --j)\n\t\t{\n\t\t\tcoord[j]++;\n\t\t\tif (coord[j] < A_out->shape[j]) {\n\t\t\t\tbreak;\n\t\t\t} else {\n\t\t\t\tcoord[j] = 0;\n\t\t\t}\n\t\t}\n\t\t}";
    txt << "\n\twhile( ec < nelements )\n\t{";
    txt << "\n"; 
    txt << oss1->str();
        
    txt << "\n\t\tfor( j=0; j<last_dim; ++j)\n\t\t{";
    txt << "\n"; 
    txt << oss2->str();
        
    txt << "\n\t\t}\n\t\tfor( j=0; j < A_out->shape[last_dim]; j++ )\n\t\t{";
    txt << "\n"; 
        
    txt << "*(off_aout+da_out) = " <<  comp_ss->str() << ";"; 
    txt << "\n"; 
    txt << oss3->str();
    
    txt << "\n\t\t}\n\t\tec += A_out->shape[last_dim];\n\t\tfor(j = last_dim-1; j >= 0; --j)\n\t\t{\n\t\t\tcoord[j]++;\n\t\t\tif (coord[j] < A_out->shape[j]) {\n\t\t\t\tbreak;\n\t\t\t} else {\n\t\t\t\tcoord[j] = 0;\n\t\t\t}\n\t\t}\n\t}";    
    txt << "\n\t" << "return status;\n}";
    return txt.str();
}

/**
 * Create a kernel function from an expression.
 */
void jitcg_expr_codetext(cphvb_array* return_array, jit_expr* expr, jit_ssa_map* ssamap, char* kernel_name) {
    printf("--- jitcg_expr_codetext() --- \n");
    stringstream param_ss;
    stringstream init_ss;
    stringstream comp_ss;
    stringstream oss1;
    stringstream oss2;
    stringstream oss3;
    
    stringstream codetext_ss;
    
    std::map<cphvb_intp,bool>* as = new std::map<cphvb_intp,bool>();
    std::map<cphvb_constant*,cphvb_intp>* cs = new std::map<cphvb_constant*,cphvb_intp>();

    init_ss << cphvb_type_typetext(return_array->type) <<"* da_out = (" << cphvb_type_typetext(return_array->type) << "*) cphvb_base_array(A_out)->data;\n";   
    init_ss << "assert(da_out != NULL);\n";
    init_ss << "cphvb_index off_aout;\n";
    
    oss1 << "\n" <<"off_aout = A_out->start;" ;
    oss2 << "\n" << "off_aout += coord[j] * A_out->stride[j];";
    oss3 << "\n" << "off_aout += A_out->stride[last_dim];";
    
    
    // fill stringbuffers with the init, computation and offset strings.    
    jitcg_expr_travers(expr,ssamap,as,cs, &param_ss,&init_ss,&comp_ss,&oss1,&oss2,&oss3);
        
    // fill param_ss
    param_ss << "cphvb_array* A_out, ";
    map<cphvb_intp,bool>::iterator ita;
    for(ita=as->begin(); ita != as->end(); ita++) {
        param_ss << "cphvb_array* A_" << (*ita).first << "," ;        
    }
    
    map<cphvb_constant*,cphvb_intp>::iterator itc;
    for(itc=cs->begin(); itc != cs->end(); itc++) {
        param_ss << "cphvb_constant* C_" << (*itc).first << "," ;        
    }            
    param_ss << "cphvb_index skip, cphvb_index limit";

                                    
    string codetext = traverse_function_text(kernel_name,&param_ss,&init_ss,&comp_ss,&oss1,&oss2,&oss3);    
        
    //printf("%s",codetext.c_str());                    
    
    //const char* codetext = traverse_function_text(kernel_name,....)
    //codetext_ss << "codetext_ss\n\n" << param_ss.str() << init_ss.str() << comp_ss.str() << "\n\n" << oss1.str() <<"\n";
    //printf("%s",codetext_ss.str().c_str());
}



// add uniqe arrays and constants to a list
// list of names
// call a function with a set of parameters. (a1,a2,a3,c1,c2)



//~ // this is to become a string, in-template piece of code!! 
//~ cphvb_error traverser_expr (cphvb_array* A_o, cphvb_array* A_0,cphvb_array* A_1) {
    //~ 
    //~ //Instr opcode_func;                          // Element-wise functor-pointer
    //~ //cphvb_array *a0 = instr->operand[0],        // Array-Operands
    //~ //            *a1 = instr->operand[1];
                                                //~ // Pointers to start of data elements
                                                //~ 
        //~ 
    //~ // extraction and type casting of input     
    //~ // A_[name]->type* da_[name] = (A_[name]->type*) cphvb_base_array(A_[name])->data;    
    //~ // sprintf(strbuff,"A_%s->type* da_%s = (A_%s->type*) cphvb_base_array(A_%s)->data;",name,name,name,name);
            //~ 
    //~ A_o->type* da_o = (A_o->type*) cphvb_base_array(A_o)->data;        
    //~ A_0->type* da_1 = (A_0->type*) cphvb_base_array(A_0)->data;    
    //~ A_1->type* da_2 = (A_1->type*) cphvb_base_array(A_1)->data;    
    //~ 
    //~ // for constants (pass by value ok?)
    //~ // C_[name].type* dc_[name] = (C_[name].type*) &(C_[name].constant.value)
    //~ //sprintf(strbuff,"C_%s.type* dc_%s = (C_%s.type*) &(C_%s.constant.value)",name,name,name,name);
    //~ // T2* d2 = (T2*) &(instr->constant.value);
//~ 
    //~ // array's only
    //~ //sprintf(strbuff,"assert(da_%s != NULL);",name);
    //~ // constant
    //~ //sprintf(strbuff,"assert(dc_%s != NULL);",name);
//~ 
    //~ assert(d0 != NULL);                         // Ensure that data is allocated
    //~ assert(d1 != NULL);
//~ 
//~ 
    //~ // array's only    
    //~ // sprintf(strbuff,"cphvb_index off_%s;",name);
    //~ // last_dim = a_%s    
    //~ // sprintf(strbuff,"cphvb_index nelements = (%d>0) ? %d : cphvb_nelements( %s->ndim, %s->shape );",limit,limit,name_out,name_out);
    //~ 
    //~ cphvb_index j,                              // Traversal variables
                //~ last_dim = a0->ndim-1,
                //~ off0, off1,
                //~ nelements = (limit>0) ? limit : cphvb_nelements( a0->ndim, a0->shape ),
                //~ ec = 0;
//~ 
    //~ cphvb_index coord[CPHVB_MAXDIM];
    //~ memset(coord, 0, CPHVB_MAXDIM * sizeof(cphvb_index));
//~ 
    //~ if (skip>0) 
    //~ {                                // Create coord based on skip
        //~ while(ec<skip)
        //~ {
            //~ //sprintf(strbuff,"ec += %s->shape[last_dim];",name_out);
            //~ ec += a0->shape[last_dim];
            //~ for(j = last_dim-1; j >= 0; --j)
            //~ {
                //~ coord[j]++;
                //~ //sprintf(strbuff,"if (coord[j] < %s->shape[j]) {",name_out);
                //~ if (coord[j] < a0->shape[j]) {
                    //~ break;
                //~ } else {
                    //~ coord[j] = 0;
                //~ }
            //~ }
        //~ }
    //~ }
//~ 
    //~ while( ec < nelements )
    //~ {
        //~ // for arrays only
        //~ // sprintf(strbuff,"off_%s = %s->start",name,name);
        //~ off0 = a0->start;                           // Compute offset based on coord
        //~ off1 = a1->start;
        //~ 
        //~ for( j=0; j<last_dim; ++j)
        //~ {
            //~ 
            //~ // for arrays only
            //~ // sprintf(strbuff,"off_%s += coord[j] * %s->stride[j]",name,name);
            //~ 
            //~ off0 += coord[j] * a0->stride[j];
            //~ off1 += coord[j] * a1->stride[j];
        //~ }
//~ 
        //~ for( j=0; j < a0->shape[last_dim]; j++ )    // Iterate over "last" / "innermost" dimension
        //~ {
            //~ // expr to stringform, substitute with 
            //~ // assign(off_out+d_out, add)
            //~ // a_out = add( , p)                    
            //~ // &(d_out+off_out) = add(off_0+da_0, add( add(off_1+da_1, off_2+da_2), add(off_3+da_3,off_4+da_4)))
            //~ 
            //~ // &(d_out+off_out) =  ( ((a_0->type) &(off_0+da_0)) + ((off_1+da_1 + off_2+da_2) + (off_3+da_3 + off_4+da_4)))
                        //~ 
            //~ 
            //~ 
            //~ opcode_func( (off0+d0), (off1+d1), d2 );
//~ 
            //~ off0 += a0->stride[last_dim];
            //~ off1 += a1->stride[last_dim];
        //~ }
        //~ ec += a0->shape[last_dim];
//~ 
        //~ for(j = last_dim-1; j >= 0; --j)            // Increment coordinates for the remaining dimensions
        //~ {
            //~ coord[j]++;
            //~ if (coord[j] < a0->shape[j]) {          // Still within this dimension
                //~ break;
            //~ } else {                                // Reached the end of this dimension
                //~ coord[j] = 0;                       // Reset coordinate
            //~ }                                       // Loop then continues to increment the next dimension
        //~ }
//~ 
    //~ }
//~ 




/**
 * take and expression and a target array and generate code for this
 **/
const char* jitcg_generate_code(jit_expr* expr) {
    int loglevel = 4;
    int ERROR=1,WARNINIG=2,INFO=3,DEBUG=4;
    
    string s = "";
    
    if(loglevel >= INFO) printf("jitcg_generate_code()");    
    if(loglevel >= DEBUG) printf("Expression depth: %d", (int)expr->depth);
        
    
    return (s.c_str()) ;
}


cphvb_error kernel_function_first_kernel(cphvb_array* A_out, cphvb_array* A_0,cphvb_array* A_1,cphvb_index skip, cphvb_index limit)
{
cphvb_error status = CPHVB_ERROR;
cphvb_float32* da_out = (cphvb_float32*) cphvb_base_array(A_out)->data;
assert(da_out != NULL);
cphvb_index off_aout;
cphvb_float32* da_0 = (cphvb_float32*) cphvb_base_array(A_0)->data;
assert(da_0 != NULL);
cphvb_index off_a0;
cphvb_float32* da_1 = (cphvb_float32*) cphvb_base_array(A_1)->data;
assert(da_1 != NULL);
cphvb_index off_a1;
cphvb_index j,last_dim = A_out->ndim-1,nelements = (limit>0) ? limit : cphvb_nelements( A_out->ndim, A_out->shape ),ec = 0;
cphvb_index coord[CPHVB_MAXDIM];memset(coord, 0, CPHVB_MAXDIM * sizeof(cphvb_index));
if (skip>0)
	while(ec<skip)
    {
		ec += A_out->shape[last_dim];
		for(j = last_dim-1; j >= 0; --j)
		{
			coord[j]++;
			if (coord[j] < A_out->shape[j]) {
				break;
			} else {
				coord[j] = 0;
			}
		}
    }
	while( ec < nelements )
	{

        off_aout = A_out->start;
        off_a0 = A_0->start;
        off_a1 = A_1->start;

		for( j=0; j<last_dim; ++j)
		{

            off_aout += coord[j] * A_out->stride[j];
            off_a0 += coord[j] * A_0->stride[j];
            off_a1 += coord[j] * A_1->stride[j];

		}
		for( j=0; j < A_out->shape[last_dim]; j++ )
		{
            *(off_aout+da_out) = (*(off_a0+da_0)+*(off_a1+da_1));
            
            //printf(" %d ",(cphvb_int32)*(off_a1+da_1));
            
            off_aout += A_out->stride[last_dim];
            off_a0 += A_0->stride[last_dim];
            off_a1 += A_1->stride[last_dim];

		}
		ec += A_out->shape[last_dim];
		for(j = last_dim-1; j >= 0; --j)
		{
			coord[j]++;
			if (coord[j] < A_out->shape[j]) {
				break;
			} else {
				coord[j] = 0;
			}
		}
	}
	return status;
}



