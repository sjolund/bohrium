
//jit_codegenerator.cpp

#include "cphvb.h"
#include "jit_codegenerator.h"
#include "jit_analyser.h"
#include "jit_logging.h"
#include "jit_compile.h"
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <string>
#include <map>
#include "StringHasher.hpp"

#define COMPUTE_SIGN (cphvb_array*,vector<cphvb_array*>*,vector<cphvb_constant*>*,cphvb_index,cphvb_index*) 

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
    while( ec < nelements ) {
        //printf(". %d %d .\n",ec,last_dim);
        off0 = a0->start;                           // Compute offset based on coord
        for( j=0; j<last_dim; ++j) {
            off0 += coord[j] * a0->stride[j];
        }

        for( j=0; j < a0->shape[last_dim]; j++ ) {    // Iterate over "last" / "innermost" dimension        
            *ss << ((T1) *(off0+d0)) << " ";            
            off0 += a0->stride[last_dim];
        }
        ec += a0->shape[last_dim];

        for(j = last_dim-1; j >= 0; --j) {           // Increment coordinates for the remaining dimensions      
            coord[j]++;
            if (coord[j] < a0->shape[j]) {          // Still within this dimension
                break;
            } else {                                // Reached the end of this dimension
                coord[j] = 0;                       // Reset coordinate
            }                                       // Loop then continues to increment the next dimension
        }
    }

}


//jit_comp_kernel* jitcg_create_kernel

void jitcg_expr_traverse2(jit_expr* expr, jit_analyse_state* s, std::map<cphvb_intp,cphvb_intp>* as, std::map<cphvb_constant*,cphvb_intp>* cs, stringstream* comp_ss );
string create_kernel_function_travers(string name,string computation_string);
string create_kernel_function(string computation_string);

void jitcg_create_kernel_code(jitcg_state* cgs, jit_analyse_state* s, jit_expr* expr) {
    //printf("jitcg_create_kernel_code()\n");
    // get the nametable entry for the expr.
    jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);
    
    // stringstream to build the math expression. 
    stringstream comp_ss;
    
    // left hand side of the math expression    
    comp_ss << "*(((" << cphvb_type_typetext(entr->arrayp->type) << "*) oa->data) + off_oa) = ";

    // lists to hold input variables, Arrays and Constants, from traversal of expr.
    std::map<cphvb_intp,cphvb_intp>* as = new std::map<cphvb_intp,cphvb_intp>();
    std::map<cphvb_constant*,cphvb_intp>* cs = new std::map<cphvb_constant*,cphvb_intp>();

    // travers the expr, building the math computation and extracting input variables.
    jitcg_expr_traverse2(expr, s, as, cs, &comp_ss);

    // convert maps into arrays with key==index        
    cphvb_array** vasa      = (cphvb_array**)malloc(as->size() * sizeof(cphvb_array*));
    cphvb_constant** vcsa   = (cphvb_constant**)malloc(cs->size() * sizeof(cphvb_constant*)); 
        
    std::map<cphvb_intp,cphvb_intp>::iterator ita;    
    for(ita = as->begin();ita != as->end();ita++) {
        cphvb_array* ainput = cphvb_base_array(jita_nametable_lookup(s->nametable,(*ita).first)->arrayp);
        //cphvb_array* ainput = cphvb_base_array(jita_nametable_lookup(s->nametable,(*ita).first)->arrayp);
        //printf("+++++ %d %p\n",(*ita).second,ainput);
        
        vasa[(*ita).second] = ainput;
    }
    std::map<cphvb_constant*,cphvb_intp>::iterator itc;    
    for(itc = cs->begin();itc != cs->end();itc++) {                
        vcsa[(*itc).second] = (*itc).first;
    }

    cgs->array_inputs = vasa;
    cgs->array_inputs_len = as->size();

    cgs->constant_inputs = vcsa;
    cgs->constant_inputs_len = cs->size();

    //string kernel_name = "kernel_func_001";
    //printf("----------- %s\n",cgs->kernel_name.c_str());
    string func_text = create_kernel_function_travers(cgs->kernel_name.c_str(),comp_ss.str());
    //printf("----------- %s\n",func_text.c_str());
    cgs->source_function = func_text;
    cgs->source_math = comp_ss.str();




        
    //~ 
    //~ size_t hash = string_hasher(comp_ss.str());
    //~ char buff[30];
    //~ sprintf(buff,"kernel_func_%d",hash);
    //~ string kernel_name = buff;
    //~ // create the text function    
    //~ //string func_text = compute_func_text(comp_ss.str());
    //~ //string func_text = create_kernel_function(comp_ss.str());
    //~ string func_text = create_kernel_function_travers(kernel_name,comp_ss.str());    
        //~ 
    //~ // compile the computation function
    //~ jit_comp_kernel* kernel = jitc_compile_computefunction(kernel_name,func_text);
    //~ if (kernel == NULL) {
        //~ logInfo("kernel == null\n");
        //~ return;
    //~ } 
        //~ logInfo("kernel->function == %p\n",kernel->function);
            //~ 
    //~ // compile the comp_ss.str() into a compute function    
    //~ //void (*compute_fun) () = 
        //~ 
    //~ 
    //~ kernel->function(cphvb_base_array(output_array), vasa, as->size() ,vcsa, 0, 0);
    //~ //traverse_kernel_str(cphvb_base_array(output_array), vasa, as->size() ,vcsa, 0, 0);
    //~ 
 //~ 
    //~ 
    //~ stringstream s;
    //~ s << hash;
    //~ 
    //~ printf(" === hash %s ===",s.str().c_str());
    
    //traverse_kernel(cphvb_base_array(output_array), vasa, as->size() ,vcsa, 0, 0, kernel->function);
    logInfo("e test_computation()\n");


    
    
}


void jitcg_print_cphvb_array(cphvb_array* a0, cphvb_intp limit, stringstream* ss) {
    //logInfo("jitcg_print_cphvb_array()\n");        
    //logDebug("Array: %p\n",a0);
    if (a0 == NULL)
        return;           
    //logDebug("ArrayType: %s\n",cphvb_type_typetext(a0->type)) ;
    
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


const char* cphvb_type_typetext(cphvb_type type) {
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


void jitcg_codetext_opcode_stream(cphvb_opcode opcode, stringstream* oss) { 
    //printf("jitcg_codetext_opcode_stream()\n");   
    //printf("opdoce = %s\n",cphvb_opcode_text(opcode));
    
    switch(opcode) {
        case CPHVB_ADD:      *oss << " + "; break;
        case CPHVB_SUBTRACT: *oss << " - "; break;
        case CPHVB_MULTIPLY: *oss << " * "; break;
        case CPHVB_DIVIDE:   *oss << " / "; break;
        default: break;
        
    }   
}


void jitcg_expr_traverse2(jit_expr* expr, jit_analyse_state* s, std::map<cphvb_intp,cphvb_intp>* as, std::map<cphvb_constant*,cphvb_intp>* cs, stringstream* comp_ss ) {
    logInfo("s jitcg_expr_travers_array2()\n");
    
    jit_name_entry* entr = jita_nametable_lookup(s->nametable,expr->name);
    
    if (is_array(expr) || entr->is_executed) {       
        cphvb_intp ia = -1;        
        // if not added yet        
        if(as->find(expr->name) == as->end()) {                    
            ia = as->size();
            //printf("---------- %ld  %ld\n",expr->name,ia);
            as->insert(pair<cphvb_intp,cphvb_intp>(expr->name,ia));             
        } else {
           ia = as->find(expr->name)->second;
        }        
        if (ia > -1) {;
            if (is_array(expr)) {
                *comp_ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(expr->op.array->type) << "*) as[" << ia << "]->data))";
            } else {                
                *comp_ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(entr->arrayp->type) << "*) as[" << ia << "]->data))";
            }
        }       
        
    } else if(is_bin_op(expr)) {    
        *comp_ss << "(";
        jitcg_expr_traverse2(expr->op.expression.left,s,as,cs,comp_ss);
        jitcg_codetext_opcode_stream(expr->op.expression.opcode,comp_ss);                                     
        jitcg_expr_traverse2(expr->op.expression.right,s,as,cs,comp_ss);
        *comp_ss << ")";
        
    } else if (is_un_op(expr)) {        
        *comp_ss << "(";
        jitcg_expr_traverse2(expr->op.expression.left,s,as,cs,comp_ss);        
        jitcg_codetext_opcode_stream(expr->op.expression.opcode,comp_ss);  
        *comp_ss << ")";
        
    } else  if (is_constant(expr)) {     
        cphvb_intp ic = -1;
        // if not added yet
        if(cs->find(expr->op.constant) == cs->end()) {            
            ic = cs->size();
            cs->insert(pair<cphvb_constant*,cphvb_intp>(expr->op.constant,ic));                                
        } else {
            ic = cs->find(expr->op.constant)->second;
        }                
        
        *comp_ss << "*((" << cphvb_type_typetext(expr->op.constant->type)  << "*)" << " &cs[" << ic << "]->value)";         
    }        
    logInfo("e jitcg_expr_travers_array()\n");
}






void jitcg_expr_traverse(jit_expr* expr, jit_ssa_map* ssamap, std::map<cphvb_intp,cphvb_intp>* as, std::map<cphvb_constant*,cphvb_intp>* cs, stringstream* comp_ss ) {
    logInfo("s jitcg_expr_travers_array()\n");
    if(is_bin_op(expr)) {    
        *comp_ss << "(";
        jitcg_expr_traverse(expr->op.expression.left,ssamap,as,cs,comp_ss);
        jitcg_codetext_opcode_stream(expr->op.expression.opcode,comp_ss);                                     
        jitcg_expr_traverse(expr->op.expression.right,ssamap,as,cs,comp_ss);
        *comp_ss << ")";
        
    } else if (is_un_op(expr)) {        
        *comp_ss << "(";
        jitcg_expr_traverse(expr->op.expression.left,ssamap,as,cs,comp_ss);        
        jitcg_codetext_opcode_stream(expr->op.expression.opcode,comp_ss);  
        *comp_ss << ")";
        
    } else if (is_array(expr)) {       
        cphvb_intp ia = -1;        
        // if not added yet        
        if(as->find(expr->name) == as->end()) {                    
            ia = as->size();
            as->insert(pair<cphvb_intp,cphvb_intp>(expr->name,ia));             
        } else {
           ia = as->find(expr->name)->second;
        }        
        if (ia > -1) {;
            *comp_ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(expr->op.array->type) << "*) as[" << ia << "]->data))";
        }       
        
    } else if (is_constant(expr)) {     
        cphvb_intp ic = -1;
        // if not added yet
        if(cs->find(expr->op.constant) == cs->end()) {            
            ic = cs->size();
            cs->insert(pair<cphvb_constant*,cphvb_intp>(expr->op.constant,ic));                                
        } else {
            ic = cs->find(expr->op.constant)->second;
        }                
        
        *comp_ss << "*((" << cphvb_type_typetext(expr->op.constant->type)  << "*)" << " &cs[" << ic << "]->value)";         
    }        
    logInfo("e jitcg_expr_travers_array()\n");
}




string traverse_function_text_2(const char* fname, stringstream* codetext_ss, stringstream* comp_ss) {    
    *codetext_ss << "cphvb_error traverse_" << fname << "(cphvb_array* oa, vector<cphvb_array*>* as, vector<cphvb_constant*>* cs, cphvb_index skip, cphvb_index limit ) {\n";
    *codetext_ss <<       
"    // validate that output array is initialized!\n"
"    assert(oa->data != NULL);\n"   
"    cphvb_index i, j,last_dim = oa->ndim-1,nelements = (limit>0) ? limit : cphvb_nelements( oa->ndim, oa->shape ),ec = 0;\n"
"    cphvb_index num_as = as->size();"
"    cphvb_index offs[num_as];\n"
"    cphvb_index off_oa = 0;\n"
"    cphvb_index coord[CPHVB_MAXDIM];\n"
"    memset(coord, 0, CPHVB_MAXDIM * sizeof(cphvb_index));\n"
"    if (skip>0) {                                // Create coord based on skip\n"
"        while(ec<skip) {\n"
"            ec += oa->shape[last_dim];\n"
"            for(j = last_dim-1; j >= 0; --j) {\n"
"                coord[j]++;\n"
"                if (coord[j] < oa->shape[j]) {\n"
"                    break;\n"
"                } else {\n"
"                    coord[j] = 0;\n"
"                }\n"
"            }\n"
"        }\n"
"    }\n"    
"    while( ec < nelements ) {\n"
"        for(i=0;i<num_as;i++) {\n"
"            offs[i] = as->at(i)->start;\n"
"        }\n"
"        // Compute offset based on coord\n"
"        for( j=0; j<last_dim; ++j) {\n"
"            for(i=0;i<num_as;i++) {\n"
"                offs[i] += coord[j] * as->at(i)->stride[j];\n"
"            }\n"
"        }\n"
"        for( j=0; j < oa->shape[last_dim]; j++ ) {\n"
"            // computation code\n";
    *codetext_ss << "\t\t\t" << comp_ss->str() << ";\n";
    *codetext_ss <<
"            for(i=0;i<num_as;i++) {\n"
"               offs[i] += as->at(i)->stride[last_dim];\n"
"            }\n"
"        }\n"
"        ec += oa->shape[last_dim];\n"
"        for(j = last_dim-1; j >= 0; --j) {\n"
"            coord[j]++;\n"
"            if (coord[j] < oa->shape[j]) {\n"
"                break;\n"
"            } else {\n"
"                coord[j] = 0;\n"
"            }\n"
"        }\n"
"    }\n"
"    return CPHVB_SUCCESS;\n"
"}\n";
    return (*codetext_ss).str();
}


string traverse_function_text(const char* fname, stringstream* codetext_ss, stringstream* param_ss, stringstream* init_ss, 
                                    stringstream* comp_ss, stringstream* oss1, stringstream* oss2, 
                                    stringstream* oss3) {
    
    
    *codetext_ss << "cphvb_error kernel_function_" << fname << "(" << param_ss->str() << "){\n";
    *codetext_ss << "cphvb_error status = CPHVB_ERROR;\n";
    
    // initialize
    *codetext_ss << init_ss->str();
    *codetext_ss << "" << "cphvb_index j,";
    *codetext_ss << "" << "last_dim = A_out->ndim-1,";
    *codetext_ss << "" << "nelements = (limit>0) ? limit : cphvb_nelements( A_out->ndim, A_out->shape ),";
    *codetext_ss << "" << "ec = 0;\n";
    *codetext_ss << "" << "cphvb_index coord[CPHVB_MAXDIM];";
    *codetext_ss << "" << "memset(coord, 0, CPHVB_MAXDIM * sizeof(cphvb_index));\n";        
    
    *codetext_ss << "if (skip>0)\n\twhile(ec<skip)\n\t\t{\n\t\tec += A_out->shape[last_dim];\n\t\tfor(j = last_dim-1; j >= 0; --j)\n\t\t{\n\t\t\tcoord[j]++;\n\t\t\tif (coord[j] < A_out->shape[j]) {\n\t\t\t\tbreak;\n\t\t\t} else {\n\t\t\t\tcoord[j] = 0;\n\t\t\t}\n\t\t}\n\t\t}";
    *codetext_ss << "\n\twhile( ec < nelements )\n\t{";
    *codetext_ss << "\n"; 
    *codetext_ss << oss1->str();
        
    *codetext_ss << "\n\t\tfor( j=0; j<last_dim; ++j)\n\t\t{";
    *codetext_ss << "\n"; 
    *codetext_ss << oss2->str();
        
    *codetext_ss << "\n\t\t}\n\t\tfor( j=0; j < A_out->shape[last_dim]; j++ )\n\t\t{";
    *codetext_ss << "\n"; 
        
    *codetext_ss << "*(off_aout+da_out) = " <<  comp_ss->str() << ";"; 
    *codetext_ss << "\n"; 
    *codetext_ss << oss3->str();
    
    *codetext_ss << "\n\t\t}\n\t\tec += A_out->shape[last_dim];\n\t\tfor(j = last_dim-1; j >= 0; --j)\n\t\t{\n\t\t\tcoord[j]++;\n\t\t\tif (coord[j] < A_out->shape[j]) {\n\t\t\t\tbreak;\n\t\t\t} else {\n\t\t\t\tcoord[j] = 0;\n\t\t\t}\n\t\t}\n\t}";    
    *codetext_ss << "\n\t" << "return status;\n}";
    return codetext_ss->str();
}


string compute_func_text(string computation) {
    stringstream ss;    
    ss << "#include \"cphvb_array.h\"\n";
    ss << "#include \"cphvb_type.h\"\n";
    ss << "#include \"cphvb.h\"\n";
    ss << "void compute_func(cphvb_array* oa, cphvb_array** as, cphvb_constant** cs, cphvb_index off_oa, cphvb_index* offs) {\n";
    ss << "\t" << computation << ";\n}";    
    //ss << "\t" << "return;\n}";    
    return ss.str();
}

void compute_func_test(cphvb_array* oa, cphvb_array** as, cphvb_constant** cs, cphvb_index off_oa, cphvb_index* offs) {
    
    *(off_oa+(cphvb_float32*) oa->data) = (*(offs[0]+((cphvb_float32*) as[0]->data)) + *(offs[1]+((cphvb_float32*) as[1]->data)));
    
    //stringstream ss;
    //jitcg_print_cphvb_array(oa,0,&ss);
    //jitcg_print_cphvb_array(as[0],0,&ss);
    //*(off_oa+(cphvb_float32*) oa->data) = (*(offs[0]+((cphvb_float32*) as[0]->data)) + *(offs[1]+((cphvb_float32*) as[1]->data)));
    //*(off_oa+(cphvb_float32*) oa->data) = (*(offs[0]+((cphvb_float32*) as[0]->data)) + *(offs[0]+((cphvb_float32*) as[0]->data)));
    
    
    //jitcg_print_cphvb_array(as[1],0,&ss);
    
    //printf("%p %s\n",as[0],ss.str().c_str());
    //printf("offs: %d,%d\n",offs[0]);
    //printf("{%d ",((cphvb_int32)*(off_oa+ ((cphvb_int32*)oa))) );
    
    //logInfo("val: %d + %d\n",*(offs[0]+((cphvb_int32*) as[0]->data)), (cs[0]->value));
    
    //cphvb_int32 a = (*(offs[0]+((cphvb_float32*) as[0]->data)) + *((cphvb_float32*) &cs[0]->value));  
    //*((cphvb_float32*)&cs[0]->value)
    
    //*(off_oa+ (cphvb_int32*) oa->data) = (*(offs[0]+((cphvb_float32*) as[0]->data)) + *((cphvb_float32*) &cs[0]->value));
    //*(off_oa+ (cphvb_float32*) oa->data) = (*(offs[0]+((cphvb_float32*) as[0]->data)) + *((cphvb_float32*) &cs[0]->value));  
    //*(off_oa+(cphvb_float32*) oa->data) =   (*(offs[0]+((cphvb_float32*) as[0]->data)) + *(offs[1]+((cphvb_float32*) as[1]->data)) );
    //*(off_oa+(cphvb_float32*) oa->data) = (*(offs[0]+((cphvb_float32*) as[0]->data)) + *(offs[1]+((cphvb_float32*) as[1]->data)));
    //*(off_oa + ((cphvb_int32*) oa->data)) = *(offs[0]+((cphvb_int32*) as[0]->data)) + *(offs[1]+((cphvb_int32*) as[1]->data));
    //printf("[%d] \n",a);
}



cphvb_error traverse_kernel(cphvb_array* oa, cphvb_array* as[],cphvb_index num_as, cphvb_constant* cs[], cphvb_index skip, cphvb_index limit,
                                computefunc2 compute_func) {
    // validate that output array is initialized!
    logInfo("s traverse_kernel()\n");    
    logInfo("computefunc: %p\n",compute_func);    
    logInfo("num_as:%d\n",num_as);
    
    //computefunc compute = compute_func_test;
    computefunc2 compute = compute_func;
    
    cphvb_pprint_array(oa);
    cphvb_pprint_array(as[0]);
        
    assert(oa->data != NULL);
    cphvb_index last_dim = oa->ndim-1,nelements = (limit>0) ? limit : cphvb_nelements( oa->ndim, oa->shape ),ec = 0;    
    cphvb_index offs[num_as];
    cphvb_index off_oa = 0;
    cphvb_index coord[CPHVB_MAXDIM];
    memset(coord, 0, CPHVB_MAXDIM * sizeof(cphvb_index));
        
    logInfo("datap %p\n",oa->data);
    for(int i=0;i<num_as;i++) {
        logInfo("datap %p\n",as[i]->data);
    }
    
    if (skip>0) {                                // Create coord based on skip
        while(ec<skip) {
            ec += oa->shape[last_dim];
            for(int j = last_dim-1; j >= 0; --j) {
                coord[j]++;
                if (coord[j] < oa->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }
    }
    
    logDebug("- skip determined\n");
    while( ec < nelements ) {
        for(int i=0;i<num_as;i++) {
            offs[i] = as[i]->start;
        }
        // Compute offset based on coord
        for(int j=0; j<last_dim; ++j) {
            for(int i=0;i<num_as;i++) {
                offs[i] += coord[j] * as[i]->stride[j];
            }
        }
        for(int j=0; j < oa->shape[last_dim]; j++ ) {
            // computation code
            logInfo("pre-compute\n");
            //cphvb_pprint_array(oa);
            
            compute(oa, as, cs, off_oa, offs);  
                      			
            for(int i=0;i<num_as;i++) {
               offs[i] += as[i]->stride[last_dim];
            }
            off_oa += oa->stride[last_dim];
        }
        ec += oa->shape[last_dim];
        for(int j = last_dim-1; j >= 0; --j) {
            coord[j]++;
            if (coord[j] < oa->shape[j]) {
                break;
            } else {
                coord[j] = 0;
            }
        }
    }
    logInfo("e traverse_first_kernel()\n");
    return CPHVB_SUCCESS;
}

string create_kernel_function_travers(string name,string computation_string) {
    logDebug("create_kernel_function_travers(%s,comp)",name.c_str());
    
    stringstream ss;
    ss << "#include \"cphvb_array.h\"\n";
    ss << "#include \"cphvb_type.h\"\n";
    ss << "#include \"cphvb.h\"\n";    
    ss << "void " << name.c_str() << "(cphvb_array* oa, cphvb_array** as, cphvb_index num_as, cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {\n"
"        cphvb_index last_dim = oa->ndim-1;\n"
"        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( oa->ndim, oa->shape );\n"
"        cphvb_index ec = 0;\n"
"        cphvb_index off_oa = 0;\n"
"        cphvb_index* coord;\n"        
"        coord = (cphvb_index*) malloc(oa->ndim * sizeof(cphvb_index));\n"
"        memset(coord, 0, oa->ndim * sizeof(cphvb_index));\n"  
"        cphvb_index* offs;\n"
"        offs = (cphvb_index*) malloc(num_as * sizeof(cphvb_index));\n"
"        int j=0, i=0;\n"
"        if (skip>0) {                                // Create coord based on skip\n"
"            while(ec<skip) {\n"
"                ec += oa->shape[last_dim];\n"
"                for(j = (last_dim-1); j >= 0; --j) {\n"
"                    coord[j]++;\n"
"                }\n"
"            }\n"
"        }\n"
"        while( ec < nelements ) {\n"
"            for(i=0;i<num_as;i++) {\n"
"                offs[i] = as[i]->start;\n"
"            }\n"
"            // Compute offset based on coord\n"
"            for(j=0; j<last_dim; ++j) {\n"
"                for(i=0;i<num_as;i++) {\n"
"                    offs[i] += coord[j] * as[i]->stride[j];\n"
"                }\n"
"            }\n"  
"            for(j=0; j < oa->shape[last_dim]; j++ ) {\n"
"                // computation code\n";
    ss << "                " << computation_string << ";\n";
    ss << "                for(i=0;i<num_as;i++) {\n"
"                   offs[i] += as[i]->stride[last_dim];\n"
"                }\n"
"                off_oa += oa->stride[last_dim];\n"
"            }\n"
"            ec += oa->shape[last_dim];\n"
"            for(j = last_dim-1; j >= 0; --j) {\n"
"                coord[j]++;\n"
"                if (coord[j] < oa->shape[j]) {\n"
"                    break;\n"
"                } else {\n"
"                    coord[j] = 0;\n"
"                }\n"
"            }\n"   
"        }\n"
"        free(coord);\n"
"        free(offs);\n"
"    }\n";
    return ss.str();
}

string create_kernel_function(string computation_string) {
    
    stringstream ss;
    
    ss << "#include \"cphvb_array.h\"\n";
    ss << "#include \"cphvb_type.h\"\n";
    ss << "#include \"cphvb.h\"\n";    
    ss << "void traverse_kernel(cphvb_array* oa, cphvb_array* as[],cphvb_index num_as, cphvb_constant* cs[], cphvb_index skip, cphvb_index limit) {\n"
"        cphvb_pprint_array(oa);\n"
"        cphvb_pprint_array(as[0]);\n"           
"        cphvb_index last_dim = oa->ndim-1,nelements = (limit>0) ? limit : cphvb_nelements( oa->ndim, oa->shape ),ec = 0;\n"
"        cphvb_index offs[num_as];\n"
"        cphvb_index off_oa = 0;\n"
"        cphvb_index coord[CPHVB_MAXDIM];\n"
"        memset(coord, 0, CPHVB_MAXDIM * sizeof(cphvb_index));\n"
"        if (skip>0) {                                // Create coord based on skip\n"
"            while(ec<skip) {\n"
"                ec += oa->shape[last_dim];\n"
"                for(int j = last_dim-1; j >= 0; --j) {\n"
"                    coord[j]++;\n"
"                    if (coord[j] < oa->shape[j]) {\n"
"                        break;\n"
"                    } else {\n"
"                        coord[j] = 0;\n"
"                    }\n"
"                }\n"
"            }\n"
"        }\n"     
"        while( ec < nelements ) {\n"
"            for(int i=0;i<num_as;i++) {\n"
"                offs[i] = as[i]->start;\n"
"            }\n"
"            // Compute offset based on coord\n"
"            for(int j=0; j<last_dim; ++j) {\n"
"                for(int i=0;i<num_as;i++) {\n"
"                    offs[i] += coord[j] * as[i]->stride[j];\n"
"                }\n"
"            }\n"
"            for(int j=0; j < oa->shape[last_dim]; j++ ) {\n"
"                // computation code\n";
    ss << "                " << computation_string << ";\n";
    ss << "                for(int i=0;i<num_as;i++) {\n"
"                   offs[i] += as[i]->stride[last_dim];\n"
"                }\n"
"                off_oa += oa->stride[last_dim];\n"
"            }\n"
"            ec += oa->shape[last_dim];\n"
"            for(int j = last_dim-1; j >= 0; --j) {\n"
"                coord[j]++;\n"
"                if (coord[j] < oa->shape[j]) {\n"
"                    break;\n"
"                } else {\n"
"                    coord[j] = 0;\n"
"                }\n"
"            }\n"
"        }\n"
"    }\n";

    return ss.str();
}

void traverse_kernel_str(cphvb_array* oa, cphvb_array* as[],cphvb_index num_as, cphvb_constant* cs[], cphvb_index skip, cphvb_index limit);

void test_computation(cphvb_array* output_array, jit_expr* expr,jit_ssa_map* ssamap,jit_name_table* nametable) {
    logInfo("s test_computation()\n");
    // create maps and stringstream
    stringstream comp_ss;    
    std::map<cphvb_intp,cphvb_intp>* as = new std::map<cphvb_intp,cphvb_intp>();
    std::map<cphvb_constant*,cphvb_intp>* cs = new std::map<cphvb_constant*,cphvb_intp>();
        
    comp_ss << "*(((" << cphvb_type_typetext(output_array->type) << "*) oa->data) + off_oa) = ";
    
    // traverse the expr, creating array and constant vectors and computation code        
    //jitcg_expr_travers_a(expr, ssamap, as, cs, &comp_ss);
    
    jitcg_expr_traverse(expr, ssamap, as, cs, &comp_ss);
    
    // convert maps into vectors with key==index
    vector<cphvb_array*>* vas = new vector<cphvb_array*>(as->size());
    vector<cphvb_constant*>* vcs = new vector<cphvb_constant*>(cs->size());
    
    cphvb_array* vasa[as->size()];
    cphvb_constant* vcsa[cs->size()];    
    
    
    std::map<cphvb_intp,cphvb_intp>::iterator ita;    
    for(ita = as->begin();ita != as->end();ita++) {        
        //vas->at((*ita).second) =  cphvb_base_array(jita_nametable_lookup(nametable,(*ita).first)->arrayp);
        
        vasa[(*ita).second] = cphvb_base_array(jita_nametable_lookup(nametable,(*ita).first)->arrayp);
    }
    std::map<cphvb_constant*,cphvb_intp>::iterator itc;    
    for(itc = cs->begin();itc != cs->end();itc++) {
        //vcs->at((*itc).second) = (*itc).first;
        
        vcsa[(*itc).second] = (*itc).first;
    }
    
    size_t hash = string_hasher(comp_ss.str());
    char buff[30];
    sprintf(buff,"kernel_func_%d",hash);
    string kernel_name = buff;
    // create the text function    
    //string func_text = compute_func_text(comp_ss.str());
    //string func_text = create_kernel_function(comp_ss.str());
    string func_text = create_kernel_function_travers(kernel_name,comp_ss.str());    
        
    // compile the computation function
    jit_comp_kernel* kernel = jitc_compile_computefunction(kernel_name,func_text);
    if (kernel == NULL) {
        logInfo("kernel == null\n");
        return;
    } 
        logInfo("kernel->function == %p\n",kernel->function);
            
    // compile the comp_ss.str() into a compute function    
    //void (*compute_fun) () = 
        
    
    kernel->function(cphvb_base_array(output_array), vasa, as->size() ,vcsa, 0, 0);
    //traverse_kernel_str(cphvb_base_array(output_array), vasa, as->size() ,vcsa, 0, 0);
    
 
    
    stringstream s;
    s << hash;
    
    printf(" === hash %s ===",s.str().c_str());
    
    //traverse_kernel(cphvb_base_array(output_array), vasa, as->size() ,vcsa, 0, 0, kernel->function);
    logInfo("e test_computation()\n");
}


void traverse_kernel_str(cphvb_array* oa, cphvb_array* as[],cphvb_index num_as, cphvb_constant* cs[], cphvb_index skip, cphvb_index limit) {
    cphvb_pprint_array(oa);
    cphvb_pprint_array(as[0]);        assert(oa->data != NULL);
    cphvb_index last_dim = oa->ndim-1,nelements = (limit>0) ? limit : cphvb_nelements( oa->ndim, oa->shape ),ec = 0;
    cphvb_index offs[num_as];
    cphvb_index off_oa = 0;
    cphvb_index coord[CPHVB_MAXDIM];
    memset(coord, 0, CPHVB_MAXDIM * sizeof(cphvb_index));
    if (skip>0) {                                // Create coord based on skip
        while(ec<skip) {
            ec += oa->shape[last_dim];
            for(int j = last_dim-1; j >= 0; --j) {
                coord[j]++;
                if (coord[j] < oa->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }
    }
    while( ec < nelements ) {
        for(int i=0;i<num_as;i++) {
            offs[i] = as[i]->start;
        }
        // Compute offset based on coord
        for(int j=0; j<last_dim; ++j) {
            for(int i=0;i<num_as;i++) {
                offs[i] += coord[j] * as[i]->stride[j];
            }
        }
        for(int j=0; j < oa->shape[last_dim]; j++ ) {
            // computation code
            *(((cphvb_int32*) oa->data) + off_oa) = *(offs[0]+((cphvb_int32*) as[0]->data));
            for(int i=0;i<num_as;i++) {
               offs[i] += as[i]->stride[last_dim];
            }
            off_oa += oa->stride[last_dim];
        }
        ec += oa->shape[last_dim];
        for(int j = last_dim-1; j >= 0; --j) {
            coord[j]++;
            if (coord[j] < oa->shape[j]) {
                break;
            } else {
                coord[j] = 0;
            }
        }
    }
}


void kernel_func_001(cphvb_array* oa, cphvb_array** as, cphvb_index num_as, cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
        cphvb_intp last_dim = (oa->ndim-1);
        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( oa->ndim, oa->shape );
        cphvb_index ec = 0;
        cphvb_index off_oa = 0;
        cphvb_index coord[oa->ndim];
        //~ cphvb_index* coord;
        //~ coord = (cphvb_index*) malloc( (sizeof(cphvb_index) * (int)oa->ndim) );
        memset(coord, 0, oa->ndim * sizeof(cphvb_index));
        cphvb_index offs[num_as];
        //~ cphvb_index* offs;
        //~ offs = (cphvb_index*) malloc(num_as * sizeof(cphvb_index));
        //~ cphvb_array* test;
        

        int j=0, i=0;
        if (skip>0) {                                // Create coord based on skip
            while(ec<skip) {
                ec += oa->shape[last_dim];
                for(j = (last_dim-1); j >= 0; --j) {
                    coord[j]++;
                }
            }
        }
        
        while( ec < nelements ) {
            
            for(i=0;i<num_as;i++) {                                                
                offs[i] = as[i]->start;
                //printf("a %d %p,%d\n",i,as[i],offs[i]);            
            }
            printf("test1\n");
            // Compute offset based on coord
            for(j=0; j<last_dim; ++j) {
                for(i=0;i<num_as;i++) {
                    offs[i] += coord[j] * as[i]->stride[j];
                }
            }
            printf("test2\n");
            for(j=0; j < oa->shape[last_dim]; j++ ) {
                // computation code
                //printf("out = %p, in %p %p\n",oa->data,as[0]->data,as[1]->data);
                *(((cphvb_float32*) oa->data) + off_oa) = (*(offs[0]+((cphvb_float32*) as[0]->data)) + *(offs[1]+((cphvb_float32*) as[1]->data)));
                //printf("1\n");
                for(i=0;i<num_as;i++) {
                    //printf("%ld, %ld\n",offs[i], as[i]->stride[last_dim]);
                    //printf("ldim %ld\n", last_dim);
                    offs[i] = offs[i] + (as[i]->stride[last_dim]);
                    //printf("%ld\n",offs[i]);
                }                
                off_oa += oa->stride[last_dim];
            }
            
            ec += oa->shape[last_dim];
            for(j = last_dim-1; j >= 0; --j) {
                coord[j]++;
                if (coord[j] < oa->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }
        printf("test3\n");
        //free(coord);
        //free(offs);
    }
//~ 
