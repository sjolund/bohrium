// jit_common
// could hold commonly used functions and data structures. 


// print functions

#include "jit_common.h"

#include "MurmurHash3.h"

#define PPRINT_BUF_STRIDE_SIZE 50
#define PPRINT_BUF_SHAPE_SIZE 50
#define PPRINT_BUF_OPSTR_SIZE 512
#define PPRINT_BUF_SIZE PPRINT_BUF_OPSTR_SIZE*4

template <typename T1>
void _jit_pprint_cphvb_array(cphvb_array* a0, cphvb_intp limit, stringstream* ss) {    
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
        *ss << "[";                
        for( j=0; j < a0->shape[last_dim]; j++ ) {    // Iterate over "last" / "innermost" dimension                    
            *ss << *(off0+d0) << ",";
            off0 += a0->stride[last_dim];
        }
        *ss << "]\n";
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

void jit_pprint_cphvb_array_stream(cphvb_array* a0, cphvb_intp limit, stringstream* ss) {
    //logInfo("jitcg_print_cphvb_array()\n");        
    //logDebug("Array: %p\n",a0);
    if (a0 == NULL)
        return;           
    //logDebug("ArrayType: %s\n",cphvb_type_typetext(a0->type)) ;
    
    switch(a0->type) {
        case CPHVB_BOOL:
            _jit_pprint_cphvb_array<cphvb_bool>(a0,limit,ss);         
            break;   
        case CPHVB_INT8:
            _jit_pprint_cphvb_array<cphvb_int8>(a0,limit,ss);            
            break;   
        case CPHVB_INT16:
            _jit_pprint_cphvb_array<cphvb_int16>(a0,limit,ss);            
            break;   
        case CPHVB_INT32:
            _jit_pprint_cphvb_array<cphvb_int32>(a0,limit,ss);            
            break;   
        case CPHVB_INT64:
            _jit_pprint_cphvb_array<cphvb_int64>(a0,limit,ss);            
            break;   
        case CPHVB_UINT8:
            _jit_pprint_cphvb_array<cphvb_uint8>(a0,limit,ss);            
            break;   
        case CPHVB_UINT16:
            _jit_pprint_cphvb_array<cphvb_uint16>(a0,limit,ss);            
            break;   
        case CPHVB_UINT32:
            _jit_pprint_cphvb_array<cphvb_uint32>(a0,limit,ss);            
            break;   
        case CPHVB_UINT64:
            _jit_pprint_cphvb_array<cphvb_uint64>(a0,limit,ss);            
            break;   
        case CPHVB_FLOAT16:        
            _jit_pprint_cphvb_array<cphvb_float16>(a0,limit,ss);            
            break;   
        case CPHVB_FLOAT32:        
            _jit_pprint_cphvb_array<cphvb_float32>(a0,limit,ss);            
            break;   
        case CPHVB_FLOAT64:        
            _jit_pprint_cphvb_array<cphvb_float64>(a0,limit,ss);            
            break;   
        default:            
            break;   
    }    
}

void jit_pprint_cphvb_array(cphvb_array* a0, cphvb_intp limit) {
    stringstream ss;
    jit_pprint_cphvb_array_stream(a0,limit,&ss);
    printf("%s",ss.str().c_str());
}



string jit_pprint_true_false(bool stm) {
    if(stm)
        return string("true");
    else
        return string("true");
}

void expr_travers_for_hashing(jit_expr* expr, vector<cphvb_intp>* chain) {
    if (is_array(expr)) {
        chain->push_back(expr->name^expr->tag);
        
    } else if (is_constant(expr)) {
        chain->push_back(expr->name^expr->tag);
        
    } else if(is_bin_op(expr)) {
        chain->push_back(expr->name^expr->depth);
        expr_travers_for_hashing(expr->op.expression.left,chain);
        expr_travers_for_hashing(expr->op.expression.right,chain);
        
    } else if (is_un_op(expr)) {
        chain->push_back(expr->name^expr->depth);
        expr_travers_for_hashing(expr->op.expression.left,chain);
    }
}

timespec diff(timespec start, timespec end) {
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

cphvb_intp expr_hash(jit_expr* expr) {
    vector<cphvb_intp> hashinputvector;
    expr_travers_for_hashing(expr,&hashinputvector);
    cphvb_intp hashinput[hashinputvector.size()];
    
    for(uint i=0;i<hashinputvector.size();i++) {
        hashinput[i] = hashinputvector[i];              
    }
    uint32_t result = 0;
    uint32_t seed = 42;
    cphvb_intp hash_val = 0;
    MurmurHash3_x86_32(&hashinput,(int)hashinputvector.size()*2,seed,&result);
    hash_val = result;
    
    return hash_val;
}

// compare two cphvb_intp array.
cphvb_intp instructionlist_hash(cphvb_instruction* instruction_list, cphvb_intp instruction_count) {
    cphvb_intp hash_val = 0;

    // lookup length in a bit map. no entry with length, it is resonable to skip.
    cphvb_intp opcodes[instruction_count];    
    
    for(cphvb_index i=0;i<instruction_count;i++) {
        opcodes[i] = ((cphvb_intp)instruction_list[i].opcode);              
    }

    uint32_t result = 0;
    uint32_t seed = 42;
    
    MurmurHash3_x86_32(&opcodes,(int)instruction_count*2,seed,&result);
    hash_val = result;
    
    return hash_val;
}

static void cphvb_sprint_instr_small( cphvb_instruction *instr, char buf[] ) {

    char op_str[PPRINT_BUF_OPSTR_SIZE];
    char tmp[PPRINT_BUF_OPSTR_SIZE];
    int op_count = cphvb_operands(instr->opcode);
    int i;
    sprintf(buf, "%s \tOPS=%d{ ", cphvb_opcode_text( instr->opcode), op_count );
    for(i=0; i < op_count; i++) {

        if (!cphvb_is_constant(instr->operand[i])) {
        
            sprintf(op_str,"A:%p",instr->operand[i]);
            
            //cphvb_sprint_array( instr->operand[i], op_str );
        } else {
            sprintf(op_str, "CONSTANT");
        }   
        sprintf(tmp, "  OP%d %s", i, op_str);
        strcat(buf, tmp);
        
    }
    strcat(buf, " ");
    if (instr->opcode == CPHVB_USERFUNC)
    {
        cphvb_userfunc* userfunc = instr->userfunc;
        for(i=0; i < userfunc->nout; i++) {
            //cphvb_sprint_array( userfunc->operand[i], op_str );
            sprintf(op_str,"A:%p",instr->operand[i]);
            sprintf(tmp, "  OUT%d %s\n", i, op_str);
            strcat(buf, tmp);
            if (userfunc->operand[i]->base != NULL) {
                sprintf(op_str,"A:%p",instr->operand[i]->base);
                //cphvb_sprint_array( userfunc->operand[i]->base, op_str );
                sprintf(tmp, "      %s\n", op_str);
                strcat(buf, tmp);
            }
        }
        for(i=userfunc->nout; i < userfunc->nout + userfunc->nin; i++) {
            //cphvb_sprint_array( userfunc->operand[i], op_str );
            sprintf(op_str,"A:%p",instr->operand[i]);
            sprintf(tmp, "  IN%d %s\n", i, op_str);
            strcat(buf, tmp);
            if (userfunc->operand[i]->base != NULL) {
                sprintf(op_str,"A:%p",instr->operand[i]->base);
                //cphvb_sprint_array( userfunc->operand[i]->base, op_str );
                sprintf(tmp, "      %s\n", op_str);
                strcat(buf, tmp);
            }
        }
    }
    strcat(buf, "}");
}

void cphvb_pprint_instr_small( cphvb_instruction *instr ) {

    char buf[PPRINT_BUF_SIZE];
    cphvb_sprint_instr_small( instr, buf );
    puts( buf );
}

    

void jit_pprint_epxr_childs_names(){
}

string jit_pprint_functiontext_creator(jitcg_functiontext_creator functext) {    
    switch(functext) {
        case JITCGFT_NoCast:
            return string("NoCast");
        case JITCGFT_Vanilla:
            return string("Vanilla");
        default:
            return string("none");
    }
}

string jit_pprint_compile_method(jit_compile_method method) {
    switch(method) {
        case COMPILE_METHOD_TCC:
            return string("TCC");
        case COMPILE_METHOD_GCC:
            return string("GCC");
        case COMPILE_METHOD_LLVM:
            return string("LLVM");
        default:
            return string("none");
    }
    
}

void cphvb_pprint_instr_list_small( cphvb_instruction* instruction_list, cphvb_intp instruction_count, const char* txt )
{
    cphvb_intp count;
    printf("%s %d {\n", txt, (int)instruction_count);
    for(count=0; count < instruction_count; count++) {
        cphvb_pprint_instr_small( &instruction_list[count] );
    }
    printf("}\n");
}
void _jit_expr_tag_short_text_stream(jit_expr_tag tag,stringstream* ss) {
    switch(tag) {
        case bin_op:
            *ss << "B";
            break;
        case un_op:
            *ss << "U";
            break;
        case const_val:
            *ss << "C";
            break;
        case array_val:
            *ss << "A";
            break;
        default:
            *ss << "-";
    }
}
void _jit_expr_tag_text_stream(jit_expr_tag tag,stringstream* ss) {
    switch(tag) {
        case bin_op:
            *ss << "Bin";
            break;
        case un_op:
            *ss << "Una";
            break;
        case const_val:
            *ss << "Con";
            break;
        case array_val:
            *ss << "Arr";
            break;
        default:
            *
            ss << "-";
    }
}

void jit_pprint_expr_short_ss(jit_expr* expr,stringstream* ss) {
    if (expr->tag == array_val || expr->tag == const_val) {
        _jit_expr_tag_short_text_stream(expr->tag, ss);
        return;
    }
    
    *ss << " ";
    _jit_expr_tag_short_text_stream(expr->tag, ss);

    if (expr->tag == bin_op) {
        *ss << "  |  ";
        _jit_expr_tag_short_text_stream(expr->tag, ss);        
    }
    
}

void jit_pprint_nametable_entry(jit_name_entry* entry) {
    stringstream ss;    
    
    ss << entry->expr->name << " :[ ";
        
    //ss << cphvb_opcode_text(entry->expr->op.expression.opcode);    
    ss << " ] " <<  entry->arrayp << " . ";
    jit_pprint_expr_short_ss(entry->expr,&ss);
    ss << "\n";
    
    printf("%s",ss.str().c_str());
}

void nametable_entry_text(jit_name_entry* ne, stringstream* ss) {        
    *ss <<  " e:" << ne->expr << " a:" << ne->arrayp <<  " b:" << ne->arrayp->base << "\n";    
}

void jit_pprint_ssamap(jit_ssa_map* ssamap) {    
    stringstream ss;
    jit_ssa_map::iterator it;
    uint i;
    for(it=ssamap->begin();it!=ssamap->end();it++) {
        ss << (*it).first << " v's: [";
        for(i=0; i<(*it).second->size();i++) {
             ss << (*it).second->at(i) << "," ;
        } 
        ss << "]\n";
    }
    printf("%s",ss.str().c_str())    ;
}

void jit_pprint_dependency_entry_values(jit_name_table* table) {

    stringstream ss;
    jit_name_table::iterator it;
    set<cphvb_intp>* nte;
    set<cphvb_intp>::iterator nit;
    ss << "Nametable: [\n";  
    for (it=table->begin();it !=table->end();it++) {


        
        nte = (*it)->tdon;
        for(nit=nte->begin();nit!=nte->end();nit++) {
            ss << (*nit) << ",";
        }        
        
        nte = (*it)->tdto;
        ss << "]\nDTO [";
        for(nit=nte->begin();nit!=nte->end();nit++) {
            ss << (*nit) << ",";
        }        
        ss << "]\n\n";

        
        //nametable_entry_text((*it),&ss);
        if ((*it)->expr->depth > 0) {
            ss << "Array: " << (*it)->arrayp << " =\n";
        }
        print_ast_recursive_stream(0,(*it)->expr,&ss);
                
    }    
    ss << "]";
    printf("%s",ss.str().c_str());            
}


void jit_pprint_name_expr(jit_expr* expr) {
    stringstream ss;
    print_ast_name_recursive_stream(0,expr,&ss);
    printf(ss.str().c_str());
}

void jit_pprint_expr(jit_expr* expr) {
    stringstream ss;
    print_ast_recursive_stream(0,expr,&ss);
    printf(ss.str().c_str());
}

void jit_pprint_base_dependency_list(vector<cphvb_intp>* ep_list) {
    stringstream ss;
    vector<cphvb_intp>::iterator itvec;     
    ss << ": [";
    for(itvec = ep_list->begin(); itvec != ep_list->end();itvec++) {
        ss << (*itvec) << ",";
    } 
    ss << "]\n";
    
    
    printf(ss.str().c_str());
    ss.clear();   
}

void jit_stream_text_set(set<cphvb_intp>* iset, stringstream* ss) {
    
    set<cphvb_intp>::iterator it;     
    *ss << ": [";
    for(it = iset->begin(); it != iset->end();it++) {
        (*ss) << (*it) << ",";
    } 
    (*ss) << "]";
}

void jit_pprint_set(set<cphvb_intp>* iset) {
    stringstream ss;
    set<cphvb_intp>::iterator it;     
    ss << ": [";
    for(it = iset->begin(); it != iset->end();it++) {
        ss << (*it) << ",";
    } 
    ss << "]\n";
    
    
    printf(ss.str().c_str());
    ss.clear();   
}



void jit_pprint_base_dependency_table(jit_base_dependency_table* base) {
    stringstream ss;    
    jit_base_dependency_table::iterator it;
    vector<cphvb_intp>::iterator itvec;
    for(it=base->begin();it!=base->end();it++) {
        ss << "base " << it->first << ": [";    
        for(itvec = it->second->begin(); itvec != it->second->end();itvec++) {
            ss << (*itvec) << ",";
        } 
        ss << "]\n";
    }
    
    printf(ss.str().c_str());
    ss.clear();    
}

void jit_pprint_list_of_expr(vector<jit_expr*>* expr_list) {
    stringstream ss;    
    vector<jit_expr*>::iterator it;
    ss << " list [";   
    for(it=expr_list->begin();it!=expr_list->end();it++) {
        ss << (*it)->name << ",";        
    }
    ss << "]\n";
    
    printf(ss.str().c_str());
    ss.clear(); 
}

void jit_pprint_nametable_dependencies(jit_name_table* nt) {
    stringstream ss;
    jit_name_table::iterator it;  
    ss << "Nametable dep's: [\n";  
    for (it=nt->begin();it != nt->end();it++) {
        ss << (*it)->expr->name << ": " ;
        ss << "tdon: ";
        jit_stream_text_set((*it)->tdon,&ss);
        ss << "tdto: ";
        jit_stream_text_set((*it)->tdto,&ss);
        ss << "\n";      
    }    
    ss << "]";
    printf(ss.str().c_str());    
}

string jit_pprint_nametable(jit_name_table* nt) {
    stringstream ss;
    jit_name_table::iterator it;  
    ss << "Nametable: [\n";  
    for (it=nt->begin();it != nt->end();it++) {
        if ((*it)->is_userfunction) {
            ss << (*it)->expr->name << ":Userfunc. "<< (*it)->arrayp << "|"<< (*it)->expr <<" In: "<< (*it)->expr->userfunction_inputs->size() <<"   Out: " << (*it)->span << "\n";
        } else {
            ss << (*it)->expr->name << ":" ;
            //nametable_entry_text((*it),&ss);
            ss << "Array: " << (*it)->arrayp << ""<< " DTV = " << (((*it)->dep_trav_visited) ? "T" : "F") << " ,  FreedAt: " << (*it)->freed_at << "  DiscardedAt: " << (*it)->discarded_at << "(" << (*it)->instr_num << "," << (*it)->operand_num << ")\n";
            if ((*it)->expr->depth > 0) {
               print_ast_name_recursive_stream(0,(*it)->expr,&ss);   
            }
        }        
        //print_ast_recursive_stream(0,(*it)->expr,&ss);                         
    }    
    ss << "]";
    printf(ss.str().c_str());
    return ss.str();
}


void jit_pprint_il_map2(jit_io_instruction_list_map_lists* il_map) {
    stringstream ss;
    int i=0;
    cphvb_index instr,operand;

    ss << "IL map2\n";
    ss << "Array map:\n";    
    for(i = 0; i < il_map->array_map->size(); i++) {
        instr = il_map->array_map->at(i)->first;
        operand = il_map->array_map->at(i)->second;
        ss << "(" << instr << "," << operand << "), ";
    }
    ss << "\nConstant map:\n";
    // input constants
    for(i = 0; i < il_map->constant_map->size(); i++) {
        instr = il_map->constant_map->at(i)->first;
        operand = il_map->constant_map->at(i)->second;
        ss << "(" << instr << "," << operand << "), ";
    }

    ss << "\nOutput map:\n";
    // output arrays
    for(i = 0; i < il_map->output_array_map->size(); i++) {
        instr = il_map->output_array_map->at(i)->first;
        operand = il_map->output_array_map->at(i)->second;
        ss << "(" << instr << "," << operand << "), ";        
    }
    ss << "\n";
    printf("%s",ss.str().c_str());
}

void jit_pprint_il_map(jit_io_instruction_list_map* il_map) {
    stringstream ss;
    int i=0;
    cphvb_index instr,operand;

    ss << "IL map\n";
    ss << "Array map:\n";    
    for(i = 0; i < il_map->array_map_length; i++) {
        instr = il_map->array_map[i].instruction;
        operand = il_map->array_map[i].operand;
        ss << "(" << instr << "," << operand << "), ";
    }
    ss << "\nConstant map:\n";
    // input constants
    for(i = 0; i < il_map->constant_map_length; i++) {
        instr = il_map->constant_map[i].instruction;
        operand = il_map->constant_map[i].operand;
        ss << "(" << instr << "," << operand << "), ";
    }

    ss << "\nOutput map:\n";
    // output arrays
    for(i = 0; i < il_map->output_array_map_length; i++) {
        instr = il_map->output_array_map[i].instruction;
        operand = il_map->output_array_map[i].operand;
        ss << "(" << instr << "," << operand << "), ";        
    }
    ss << "\n";
    printf("%s",ss.str().c_str());
}

void jit_pprint_execute_kernel(jit_execute_kernel* exekernel) {
    stringstream ss;

    //~ cphvb_array**       outputarrays;
    //~ cphvb_index         outputarrays_length;
    //~ 
    //~ // distinct array input count
    //~ cphvb_array**       inputarrays;
    //~ cphvb_index         inputarrays_length;
        //~ 
    //~ // constant input count 
    //~ cphvb_constant**    inputconstants;
    //~ cphvb_index         inputconstants_length;
    
    ss << "\n+Kernel:" << exekernel->kernel << "  " << executekernel_type_to_string(exekernel) << "  ";
    if (exekernel->kernel->type == JIT_COMPILE_KERNEL) {
        ss << "computekernel pointer:" << exekernel->kernel->compute_kernel << "\n";; 
    } else {    
        ss << "name: " << exekernel->kernel->expr_kernel->expr->name << "\n";;        
    }
    
    ss << "output arrays [" << exekernel->outputarrays_length << "] ";
    for(int i=0;i<exekernel->outputarrays_length;i++) {
        ss << "{" << exekernel->outputarrays[i] << "}"; 
    }
    ss << "\ninput  arrays [" << exekernel->arrays_length << "] ";
    for(int i=0;i<exekernel->arrays_length;i++) {
        ss << "{" << exekernel->arrays[i] << "}"; 
    }
    ss << "\nconstants     [" << exekernel->inputconstants_length << "] ";
    for(int i=0;i<exekernel->inputconstants_length;i++) {
        ss << "{" << exekernel->inputconstants[i] << "}"; 
    }
    ss << "\n";
    printf("%s",ss.str().c_str());

    if (exekernel->kernel->type == JIT_EXPR_KERNEL) {    
        printf("expr instruction list\n");
        cphvb_index i, il = exekernel->kernel->expr_kernel->instructions_length;
        cphvb_instruction* tinstr;
        for(i=0;i<il;i++) {
            tinstr = &exekernel->kernel->expr_kernel->instructions[i];
            if (tinstr != NULL) {
                printf("instr: %p   T %p\n",tinstr,tinstr->operand[0]);
            }
        }        
    }
    
    jit_pprint_il_map2(exekernel->kernel->il_map);
}

int timeval_subtract (timeval* result, timeval* x, timeval* y) {     
    
       /* Perform the carry for the later subtraction by updating y. */
       if (x->tv_usec < y->tv_usec) {
         int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
         y->tv_usec -= 1000000 * nsec;
         y->tv_sec += nsec;
       }
       if (x->tv_usec - y->tv_usec > 1000000) {
         int nsec = (x->tv_usec - y->tv_usec) / 1000000;
         y->tv_usec += 1000000 * nsec;
         y->tv_sec -= nsec;
       }
     
       /* Compute the time remaining to wait.
          tv_usec is certainly positive. */
       result->tv_sec = x->tv_sec - y->tv_sec;
       result->tv_usec = x->tv_usec - y->tv_usec;
     
       /* Return 1 if result is negative. */
       return x->tv_sec < y->tv_sec;
     }



