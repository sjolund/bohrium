
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

string kernel_define_header() {
    stringstream ss;
    ss << "#ifndef M_PI\n";
    ss << "#define M_PI 3.14159265358979323846\n";
    ss << "#endif\n";
    ss << "#if _WIN32\n";
    ss << "#include <float.h>\n";
    ss << "#define cphvb_isnan(x) (_isnan(x))\n";
    ss << "#define cphvb_isinf(x) (!_isnan(x) || !_finite(x))\n";
    ss << "#else\n";
    ss << "#define cphvb_isnan(x) (std::isnan(x))\n";
    ss << "#define cphvb_isinf(x) (std::isinf(x))\n";
    ss << "#endif\n";
    ss << "#define DEG_CIR 360.0\n";
    ss << "#define DEG_RAD (M_PI / (DEG_CIR / 2.0))\n";
    ss << "#define RAD_DEG ((DEG_CIR / 2.0) / M_PI)\n";
    ss << "\n";    

    return ss.str();
}
string jitcg_nameing_array_offset(string prefix,cphvb_index i) {
    stringstream ss;
    ss << prefix << "OffA" << i;
    return ss.str();
}

string jitcg_nameing_array_datapointer(string prefix,cphvb_index i) {
    stringstream ss;
    ss << prefix << "DA" << i;
    return ss.str();
}

string jitcg_nameing_constant(cphvb_index i) {
    stringstream ss;
    ss << "C" << i;
    return ss.str();
}

string jitcg_nameing_constant_datapointer(cphvb_index i) {
    stringstream ss;
    ss << "DC" << i;
    return ss.str();
}

string jitcg_nameing_array(cphvb_index i) {
    stringstream ss;
    ss << "A" << i;
    return ss.str();
}



//jit_comp_kernel* jitcg_create_kernel

void jitcg_expr_traverse2(jit_expr* expr, jit_analyse_state* s, std::map<cphvb_intp,cphvb_intp>* as, std::map<cphvb_constant*,cphvb_intp>* cs, stringstream* comp_ss );


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
        return " ";
    }
}

string build_expression_string(cphvb_opcode c, string s1, string s2) {
    stringstream ss;
    //printf("build_expression_string(%s)\n", cphvb_opcode_text(c));
    switch(c) {
        case CPHVB_ADD:		// Add arguments element-wise.        
            ss << "(" << s1 << " + " << s2 << ")";        
            break;
        
        case CPHVB_SUBTRACT:		    // Subtract arguments, element-wise.        
            ss << "(" << s1 << " - " << s2 << ")";        
            break;        

        case CPHVB_MULTIPLY:		    // Multiply arguments element-wise.
            ss << "(" << s1 << " * " << s2 << ")";            
            break;
        
        case CPHVB_DIVIDE:		    // Divide arguments element-wise.        
            ss << "(" << s1 << " / " << s2 << ")";                
            break;
        
        case CPHVB_POWER:		    // First array elements raised to powers from second array, element-wise.
            ss << " pow(" << s1 << "," << s2 << ")";        
            break;
        
        case CPHVB_ABSOLUTE:		// Calculate the absolute value element-wise.        
            ss << "(" << s1 << " < 0.0 ? -" << s1 << " : " << s1 << ")";        
            break;
        
        case CPHVB_GREATER:		    // Return the truth value of (x1 > x2) element-wise.        
            ss << "(" << s1 << " > " << s2 << ")";        
            break;

        case CPHVB_GREATER_EQUAL:	// Return the truth value of (x1 >= x2) element-wise.        
            ss << "(" << s1 << " >= " << s2 << ")";
            break;

        case CPHVB_LESS:		        // Return the truth value of (x1 < x2) element-wise.
            ss << "(" << s1 << " < " << s2 << ")";  
            break;

        case CPHVB_LESS_EQUAL:		// Return the truth value of (x1 =< x2) element-wise.
            ss << "(" << s1 << " <= " << s2 << ")";  
            break;

        case CPHVB_EQUAL:		    // Return (x1 == x2) element-wise.
            ss << "(" << s1 << " == " << s2 << ")";          
            break;

        case CPHVB_NOT_EQUAL:		// Return (x1 != x2) element-wise.
            ss << "(" << s1 << " != " << s2 << ")";   
            break;

        case CPHVB_LOGICAL_AND:		// Compute the truth value of x1 AND x2 elementwise.
            ss << "(" << s1 << " && " << s2 << ")";   
            break;

        case CPHVB_LOGICAL_OR:		// Compute the truth value of x1 OR x2 elementwise.
            ss << "(" << s1 << " || " << s2 << ")";   
            break;

        case CPHVB_LOGICAL_XOR:		// Compute the truth value of x1 XOR x2, element-wise.
            ss << "(!" << s1 << " != !" << s2 << ")";       
            break;
        
        case CPHVB_LOGICAL_NOT:		// Compute the truth value of NOT x elementwise.
            ss << "(!" << s1 << ")";   
            break;

        case CPHVB_MAXIMUM:		    // Element-wise maximum of array elements.
            ss << "(" << s1 << " < " << s2 << " ? " << s2 << ":" << s1 << ")";   
            break;
        
        case CPHVB_MINIMUM:		    // Element-wise minimum of array elements.
            ss << "(" << s1 << " < " << s2 << " ? " << s1 << ":" << s2 << ")";   
            break;


        case CPHVB_BITWISE_AND:		// Compute the bit-wise AND of two arrays element-wise.
            ss << "(" << s1 << " & " << s2 << ")";
            break;                
        
        case CPHVB_BITWISE_OR:		// Compute the bit-wise OR of two arrays element-wise.
            ss << "(" << s1 << " | " << s2 << ")";            
            break;
        
        case CPHVB_BITWISE_XOR:		// Compute the bit-wise XOR of two arrays element-wise.
            ss << "(" << s1 << " ^ " << s2 << ")";
            break;
        
        case CPHVB_INVERT:		    // Compute bit-wise inversion, or bit-wise NOT, element-wise.
            ss << "( ~" << s1 << ")";                        
            break;
        
        case CPHVB_LEFT_SHIFT:		// Shift the bits of an integer to the left.
            ss << "(" << s1 << " << " << s2 << ")";
            break;
        
        case CPHVB_RIGHT_SHIFT:		// Shift the bits of an integer to the right.
            ss << "(" << s1 << " >> " << s2 << ")";            
            break;
        
        case CPHVB_COS:		        // Cosine elementwise.
            ss << " cos(" << s1 << ")";            
            break;
        
        case CPHVB_SIN:		        // Trigonometric sine, element-wise.            
            ss << " sin(" << s1 << ")";            
            break;
        
        case CPHVB_TAN:		        // Compute tangent element-wise.            
            ss << " tan(" << s1 << ")";             
            break;
        
        case CPHVB_COSH:		        // Hyperbolic cosine, element-wise.        
            ss << " cosh(" << s1 << ")";             
            break;
        
        case CPHVB_SINH:		        // Hyperbolic sine, element-wise.        
            ss << " sinh(" << s1 << ")";                     
            break;
        
        case CPHVB_TANH:		        // Compute hyperbolic tangent element-wise.            
            ss << " tanh(" << s1 << ")";                         
            break;
        
        case CPHVB_ARCSIN:		    // Inverse sine, element-wise.
            ss << " asin(" << s1 << ")";                                     
            break;
            
        case CPHVB_ARCCOS:		    // Trigonometric inverse cosine, element-wise.            
            ss << " acas(" << s1 << ")";                                     
            break;
            
        case CPHVB_ARCTAN:		    // Trigonometric inverse tangent, element-wise.
            ss << " atan(" << s1 << ")";                                                         
            break;
            
        case CPHVB_ARCSINH:		    // Inverse hyperbolic sine elementwise.
            //asinh( *op2 );
            ss << " asinh(" << s1 << ")";                                     
            break;
            
        case CPHVB_ARCCOSH:		    // Inverse hyperbolic cosine, elementwise.        
            //acosh( *op2 );
            ss << " acosh(" << s1 << ")";                                     
            break;
            
        case CPHVB_ARCTANH:		    // Inverse hyperbolic tangent elementwise.            
            ss << " atanh(" << s1 << ")";              
            break;
            
        case CPHVB_ARCTAN2:		    // Element-wise arc tangent of ``x1/x2`` choosing the quadrant correctly.            
            ss << " atan2(" << s1 << "," << s2 << ")";                  
            break;
        
        case CPHVB_EXP:		        // Calculate the exponential of all elements in the input array.            
            ss << " exp(" << s1 << ")";                  
            break;
            
        case CPHVB_EXP2:		        // Calculate `2**p` for all `p` in the input array.
            //pow( 2, *op2 );
            ss << " pow( 2, " << s1 << ")";              
            break;
        
        case CPHVB_EXPM1:		    // Calculate ``exp(x) - 1`` for all elements in the array.        
            ss << " expm1(" << s1 << ")";              
            break;

        case CPHVB_LOG:		        // Natural logarithm: element-wise.            
            ss << " log(" << s1 << ")";                  
            break;
            
        case CPHVB_LOG2:		        // Base-2 logarithm of `x`.
            ss << " log2(" << s1 << ")";                  
            break;

        case CPHVB_LOG10:		    // Return the base 10 logarithm of the input array, element-wise.            
            ss << " log10(" << s1 << ")";                  
            break;
            
        case CPHVB_LOG1P:		    // Return the natural logarithm of one plus the input array, element-wise.        
            ss << " log1p(" << s1 << ")";      
            break;

        case CPHVB_SQRT:		        // Return the positive square-root of an array, element-wise.        
            ss << " sqrt(" << s1 << ")";      
            break;
        
        case CPHVB_CEIL:		        // Return the ceiling of the input, element-wise.            
            ss << " ceil(" << s1 << ")";      
            break;
        
        case CPHVB_TRUNC:		    // Return the truncated value of the input, element-wise.            
            ss << " trunc(" << s1 << ")";        
            break;
        
        case CPHVB_FLOOR:		    // Return the floor of the input, element-wise.            
            ss << " floor(" << s1 << ")";              
            break;
        
        case CPHVB_RINT:		        // Round elements of the array to the nearest integer.
            //(*op2 > 0.0) ? floor(*op2 + 0.5) : ceil(*op2 - 0.5);
            ss << "(" << s1 << " > 0.0) ? floor(" << s1 << "+0.5) : ceil(" << s1 << " - 0.5)";          
            break;
        
        case CPHVB_MOD:		        // Return the element-wise remainder of division.
            //*op1 = *op2 - floor(*op2 / *op3) * *op3;
            ss << "(" << s1 << " - floor(" << s1 << " / " <<  s2 << " ) * " << s2 << ")";
            break;
            
        case CPHVB_ISNAN:		    // Test for NaN values.
            //*op1 = cphvb_isnan(*op2);
            ss << " cphvb_isnan(" << s1 << ")";
            break;
            
        case CPHVB_ISINF:		    // Test for infinity values.
            //*op1 = cphvb_isinf(*op2);
            ss << " cphvb_isinf(" << s1 << ")";        
            break;
        /*
        CPHVB_IDENTITY:		    // The identity function that returns the input value converted to the output data type.        
        return s2;
        break;
            
        CPHVB_DISCARD:		    // System instruction that informs the child component to forget the array and release any metadata allocated.
        return ss.str();
        break;
        CPHVB_FREE:		        // System instruction that informs the child component to deallocate the data storage associated with the array.
        return ss.str();
        break;
        CPHVB_SYNC:		        // System instruction that informs the child component to make data synchronized and available.
        return ss.str();
        break;
        CPHVB_NONE:		        // A opcode that should be ignored.
        return ss.str();
        break;        
        CPHVB_USERFUNC:		    // System instruction that represents a user-defined function registered by the Bridge.
        return ss.str();
        break;
*/
        case CPHVB_IDENTITY:		    // The identity function that returns the input value converted to the output data type.        
            ss << s1;
            break;   
        default:
            ss << "";
    }
    //rintf("%s : %s %s\n",ss.str().c_str(),s1.c_str(),s2.c_str());
    return ss.str();
}

string _etrav(jit_expr* e, int* ai, int* ci) {
    stringstream ss;
    if(is_array(e)) {
        *ai += 1;        
        ss << "as[" << *ai << "]";
        return ss.str();
    } else
    if (is_userfunc(e)) {
        return (string)"userfunc";
    } else
    if (is_constant(e)) {        
        *ci += 1;
        ss << "cs[" << *ci << "]";
        return ss.str();
    } else  
    
    if (is_bin_op(e)) {
        printf("binary: \n");
        string s1 = _etrav(e->op.expression.left,ai,ci);
        string s2 = _etrav(e->op.expression.right,ai,ci);
        printf("binary: s1: %s s2: %s\n",s1.c_str(),s2.c_str());
        return build_expression_string(e->op.expression.opcode,s1,s2);
    } else
    
    if (is_un_op(e)) {
        string s1 = _etrav(e->op.expression.left,ai,ci);
        return build_expression_string(e->op.expression.opcode,s1,"NONE");
    }
}
string etrav(jit_expr* e) {
    
    int array_count = -1;
    int constant_count = -1;

    printf("etrav %p %p\n",&array_count,&constant_count);
    return _etrav(e,&array_count,&constant_count);
}

string _build_computestring_etrav(jit_expr* e, map<cphvb_array*,cphvb_index>* as, map<cphvb_constant*,cphvb_index>* cs) {
    stringstream ss;
    if(is_array(e)) {
        cphvb_index ai = -1;
        map<cphvb_array*,cphvb_index>::iterator it = as->find(e->op.array);
        if (it == as->end()) {
            ai = as->size()+1;
            as->insert(pair<cphvb_array*,cphvb_index>(e->op.array,ai));            
        } else {
            ai = it->second;
        }
        
        ss << "as[" << ai << "]";
        //ss << "*(offs[" << ia << "]+((" << cphvb_type_typetext(expr->op.array->type) << "*) as[" << ia << "]->data))";
        return ss.str();
    } else
    if (is_userfunc(e)) {
        return (string)"USERFUNC";
    } else
    if (is_constant(e)) {
        cphvb_index ci = -1;
        map<cphvb_constant*,cphvb_index>::iterator it = cs->find(e->op.constant);
        if (it == cs->end()) {
            ci = cs->size()+1;
            cs->insert(pair<cphvb_constant*,cphvb_index>(e->op.constant,ci));            
        } else {
            ci = it->second;
        }
        
        ss << "cs[" << ci << "]";
        //ss << "*((" << cphvb_type_typetext(expr->op.constant->type)  << "*)" << " &cs[" << ic << "]->value)";
        return ss.str();
    } else  
    
    if (is_bin_op(e)) {
        //printf("binary: \n");
        string s1 = _build_computestring_etrav(e->op.expression.left,as,cs);
        string s2 = _build_computestring_etrav(e->op.expression.right,as,cs);
        //printf("binary: s1: %s s2: %s\n",s1.c_str(),s2.c_str());
        return build_expression_string(e->op.expression.opcode,s1,s2);
    } else
    
    if (is_un_op(e)) {
        string s1 = _build_computestring_etrav(e->op.expression.left,as,cs);
        return build_expression_string(e->op.expression.opcode,s1,"NONE");
    }
}

string build_computestring_etrav(jit_expr* e) {
    map<cphvb_array*,cphvb_index>* as = new map<cphvb_array*,cphvb_index>();
    map<cphvb_constant*,cphvb_index>* cs = new map<cphvb_constant*,cphvb_index>();    

    string computation_as_text = _build_computestring_etrav(e,as,cs);
    as->clear(); delete(as);
    cs->clear(); delete(cs);
    //free(as);
    //free(cs);
    return computation_as_text;
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


string _build_array_compute_component_array(cphvb_array* array, cphvb_index index) {
    stringstream ss;
    ss << "*(offs[" << index << "]+((" << cphvb_type_typetext(array->type) << "*) ds[" << index << "]->data))";
    return ss.str();
}

string _build_array_compute_component_var_nocast(cphvb_index index) {
    stringstream ss;
    ss << "*(" << jitcg_nameing_array_offset("",index) << " + " << jitcg_nameing_array_datapointer("",index) << ")";
    return ss.str();
}

string jitcg_build_array_compute_component(cphvb_index index) {
    //return _build_array_compute_component_array(array,index);
    return _build_array_compute_component_var_nocast(index);
}

string _build_constant_compute_component(cphvb_constant* constant, cphvb_index index){
    stringstream ss;
    ss << " *((" << cphvb_type_typetext(constant->type)  << "*)" << " &cs[" << index << "]->value) ";
    return ss.str();
}

string _build_constant_compute_component_var_nocast(cphvb_index index) {
    stringstream ss;
    ss << "*" << jitcg_nameing_constant_datapointer(index) << "";    
    return ss.str();
}

string jitcg_build_constant_compute_component(cphvb_index index) {    
    //return _build_array_compute_component_constant;
    return _build_constant_compute_component_var_nocast(index);
}


    
string create_array_datapointers(map<cphvb_array*,cphvb_intp>* arrays, cphvb_index offset) {
    // name is 'D<prefix>#' where # is a incrementing number        
    stringstream ss;
    //printf("offset = %ld\n",offset);
    map<cphvb_array*,cphvb_intp>::iterator it;
    int i=0;
    for(it=arrays->begin(); it!=arrays->end();it++,i++) {
        ss << cphvb_type_typetext(it->first->type) << "* " << jitcg_nameing_array_datapointer("",i+offset) << " = (" << cphvb_type_typetext(it->first->type) << "*) cphvb_base_array( as[" << i+offset << "])->data;\n";
        ss << "cphvb_index " << jitcg_nameing_array_offset("",i+offset) << ";\n";
    }   
    return ss.str();  
}


string create_constant_datapointers(map<cphvb_constant*,cphvb_intp>* constants) {
    stringstream ss;
    map<cphvb_constant*,cphvb_intp>::iterator it;
    int i=0;
    for(it=constants->begin(); it!=constants->end();it++,i++) {
        ss << cphvb_type_typetext(it->first->type) << "* " << jitcg_nameing_constant_datapointer(i) << " = (" << cphvb_type_typetext(it->first->type) << "*) &cs[" << i << "]->value;\n";        
    }
    
    return ss.str();
}




string create_kernel_function_travers_nocast(string name,
                    string computation_string,                
                    map<cphvb_array*,cphvb_intp>* outputarrays,
                    map<cphvb_array*,cphvb_intp>* arrays,
                    map<cphvb_constant*,cphvb_intp>* constants) {

    cphvb_index num_outarray_dimension = arrays->begin()->first->ndim;
    cphvb_index num_arrays = arrays->size();
    cphvb_index num_constants = constants->size();


    
                        
    //printf("create_kernel_function_travers_nocast(%s). oa-size = %ld\n",name.c_str(),outputarrays->size());
    
    stringstream ss;
    //    ss << "#include <cphvb_win_math.hpp>\n";
    ss << "#include \"math.h\"\n";
    ss << "#include \"stdlib.h\"\n";
    ss << "#include \"cphvb_array.h\"\n";
    ss << "#include \"cphvb_type.h\"\n";
    ss << "#include \"cphvb.h\"\n\n";
    
    ss << "#define K_NARRAYS " << num_arrays << "\n";
    ss << "#define K_NCONSTANTS " << num_constants << "\n";
    ss << "#define K_OUT_NDIM " << num_outarray_dimension << "\n";    
    ss << kernel_define_header();
    ss <<"\n";    
    ss << "void " << name.c_str() << "(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {\n"
"        cphvb_index last_dim = as[0]->ndim-1;\n"
"        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( as[0]->ndim, as[0]->shape );\n"
"        cphvb_index ec = 0;\n"
"        cphvb_index coord[K_OUT_NDIM];\n"
"       memset(coord, 0, K_OUT_NDIM * sizeof(cphvb_index));\n";    
    ss << create_array_datapointers(arrays,0);    
    ss << create_constant_datapointers(constants);

//"        K_TOUT * doa = (K_TOUT *) cphvb_base_array(oa)->data;\n"
//"        cphvb_array* ds[K_NARRAYS];\n"

    ss <<"        int j=0, i=0;\n"        
"        if (skip>0) {                                // Create coord based on skip\n"
"            while(ec<skip) {\n"
"                ec += as[0]->shape[last_dim];\n"
"                for(j = (last_dim-1); j >= 0; --j) {\n"
"                    coord[j]++;\n"
"                }\n"
"            }\n"
"        }\n"
"        while( ec < nelements ) {\n";

        
        
        for(int i=0;i< (arrays->size());i++) {
            ss << "\t" << jitcg_nameing_array_offset("",i) << " = as[" << i << "]->start;\n";            
        }
        
    
ss << "            // Compute offset based on coord\n"
"            for(j=0; j<last_dim; ++j) {\n";

        for(int i=0;i< (arrays->size());i++) {
            ss << "\t\t" <<jitcg_nameing_array_offset("",i) << " += as[" << i << "]->stride[j] * coord[j];\n";            
        }

ss << "            }\n"  
"            for(j=0; j < as[0]->shape[last_dim]; j++ ) {\n"
"                // computation code\n";
    ss << "                " << computation_string << ";\n";


        for(int i=0;i< (arrays->size());i++) {
            ss << "\t\t" <<jitcg_nameing_array_offset("",i) << " += as[" << i << "]->stride[last_dim];\n";            
        }
    
ss << "            }\n"
"            ec += as[0]->shape[last_dim];\n"
"            for(j = last_dim-1; j >= 0; --j) {\n"
"                coord[j]++;\n"
"                if (coord[j] < as[0]->shape[j]) {\n"
"                    break;\n"
"                } else {\n"
"                    coord[j] = 0;\n"
"                }\n"
"            }\n"   
"        }\n"
"    }\n";
    return ss.str();
}    



/**
 **/
string create_kernel_function_travers(string name,
                    string computation_string,                
                    map<cphvb_array*,cphvb_intp>* outputarrays,
                    map<cphvb_array*,cphvb_intp>* arrays,
                    map<cphvb_constant*,cphvb_intp>* constants) {    
    logDebug("create_kernel_function_travers(%s,comp)",name.c_str());

    cphvb_index num_outarray_dimension = outputarrays->begin()->first->ndim;
    cphvb_index num_arrays = arrays->size();
    cphvb_index num_constants = constants->size();
    
    stringstream ss;
    ss << "#include \"math.h\"\n";
    //ss << "#include <cstdlib>\n";
    //ss << "#include <cphvb_win_math.hpp>\n";    
    ss << "#include \"cphvb_array.h\"\n";
    ss << "#include \"cphvb_type.h\"\n";
    ss << "#include \"cphvb.h\"\n";
    ss << "#define K_NARRAYS " << num_arrays << "\n";    
    ss << kernel_define_header();
    ss <<"\n";
    ss << "void " << name.c_str() << "(cphvb_array** as, cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {\n"
"        cphvb_index last_dim = as[0]->ndim-1;\n"
"        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( as[0]->ndim, as[0]->shape );\n"
"        cphvb_index ec = 0;\n"
"        int j=0, i=0;\n"
"        cphvb_index* coord;\n"
"        coord = (cphvb_index*) malloc(as[0]->ndim * sizeof(cphvb_index));\n"
"        memset(coord, 0, as[0]->ndim * sizeof(cphvb_index));\n"  
"        cphvb_index* offs;\n"
"        offs = (cphvb_index*) malloc(K_NARRAYS * sizeof(cphvb_index));\n"
"        cphvb_array** ds;\n"
"        ds = (cphvb_array**) malloc(K_NARRAYS * sizeof(cphvb_array*));\n"
"        for(i=0;i<K_NARRAYS;i++) {\n"
"           ds[i] = cphvb_base_array(as[i]);\n"
"        }\n"
"        if (skip>0) {                                // Create coord based on skip\n"
"            while(ec<skip) {\n"
"                ec += as[0]->shape[last_dim];\n"
"                for(j = (last_dim-1); j >= 0; --j) {\n"
"                    coord[j]++;\n"
"                }\n"
"            }\n"
"        }\n"
"        while( ec < nelements ) {\n"
"            for(i=0;i<K_NARRAYS;i++) {\n"
"                offs[i] = as[i]->start;\n"               
"            }\n"
"            // Compute offset based on coord\n"
"            for(j=0; j<last_dim; ++j) {\n"
"                for(i=0;i<K_NARRAYS;i++) {\n"
"                    offs[i] += coord[j] * as[i]->stride[j];\n"
"                }\n"
"            }\n"  
"            for(j=0; j < as[0]->shape[last_dim]; j++ ) {\n"
"                // computation code\n";
    ss << "                " << computation_string << ";\n";
    ss << "                for(i=0;i<K_NARRAYS;i++) {\n"
"                   offs[i] += as[i]->stride[last_dim];\n"
"                }\n"
"            }\n"
"            ec += as[0]->shape[last_dim];\n"
"            for(j = last_dim-1; j >= 0; --j) {\n"
"                coord[j]++;\n"
"                if (coord[j] < as[0]->shape[j]) {\n"
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

void traverse_kernel_str(cphvb_array* oa, cphvb_array* as[],cphvb_index num_as, cphvb_constant* cs[], cphvb_index skip, cphvb_index limit);

//~ void test_computation(cphvb_array* output_array, jit_expr* expr,jit_ssa_map* ssamap,jit_name_table* nametable) {
    //~ logInfo("s test_computation()\n");
    //~ // create maps and stringstream
    //~ stringstream comp_ss;    
    //~ std::map<cphvb_intp,cphvb_intp>* as = new std::map<cphvb_intp,cphvb_intp>();
    //~ std::map<cphvb_constant*,cphvb_intp>* cs = new std::map<cphvb_constant*,cphvb_intp>();
        //~ 
    //~ comp_ss << "*(((" << cphvb_type_typetext(output_array->type) << "*) oa->data) + off_oa) = ";
    //~ 
    //~ // traverse the expr, creating array and constant vectors and computation code        
    //~ //jitcg_expr_travers_a(expr, ssamap, as, cs, &comp_ss);
    //~ 
    //~ jitcg_expr_traverse(expr, ssamap, as, cs, &comp_ss);
    //~ 
    //~ // convert maps into vectors with key==index
    //~ vector<cphvb_array*>* vas = new vector<cphvb_array*>(as->size());
    //~ vector<cphvb_constant*>* vcs = new vector<cphvb_constant*>(cs->size());
    //~ 
    //~ cphvb_array* vasa[as->size()];
    //~ cphvb_constant* vcsa[cs->size()];    
    //~ 
    //~ 
    //~ std::map<cphvb_intp,cphvb_intp>::iterator ita;    
    //~ for(ita = as->begin();ita != as->end();ita++) {        
        //~ //vas->at((*ita).second) =  cphvb_base_array(jita_nametable_lookup(nametable,(*ita).first)->arrayp);
        //~ 
        //~ vasa[(*ita).second] = cphvb_base_array(jita_nametable_lookup(nametable,(*ita).first)->arrayp);
    //~ }
    //~ std::map<cphvb_constant*,cphvb_intp>::iterator itc;    
    //~ for(itc = cs->begin();itc != cs->end();itc++) {
        //~ //vcs->at((*itc).second) = (*itc).first;
        //~ 
        //~ vcsa[(*itc).second] = (*itc).first;
    //~ }
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
    //~ // a change in kernel function to 3. kernel->function(cphvb_base_array(output_array), vasa, as->size() ,vcsa, 0, 0);
    //~ //traverse_kernel_str(cphvb_base_array(output_array), vasa, as->size() ,vcsa, 0, 0);
    //~ 
 //~ 
    //~ 
    //~ stringstream s;
    //~ s << hash;
    //~ 
    //~ printf(" === hash %s ===",s.str().c_str());
    //~ 
    //~ //traverse_kernel(cphvb_base_array(output_array), vasa, as->size() ,vcsa, 0, 0, kernel->function);
    //~ logInfo("e test_computation()\n");
//~ }


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


void kernel_func_321436602_3(cphvb_array* oa, cphvb_array** as, cphvb_index num_as, cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
        cphvb_index last_dim = oa->ndim-1;
        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( oa->ndim, oa->shape );
        cphvb_index ec = 0;
        cphvb_index off_oa = 0;
        cphvb_index* coord;
        coord = (cphvb_index*) malloc(oa->ndim * sizeof(cphvb_index));
        memset(coord, 0, oa->ndim * sizeof(cphvb_index));
        cphvb_index* offs;
        offs = (cphvb_index*) malloc(num_as * sizeof(cphvb_index));
        int j=0, i=0;
        cphvb_array** ds;
        ds = (cphvb_array**) malloc(num_as * sizeof(cphvb_array*));
        cphvb_array* doa = cphvb_base_array(oa);
        for(i=0;i<num_as;i++) {
           ds[i] = cphvb_base_array(as[i]);
        }
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
                off_oa = oa->start;
            }
            // Compute offset based on coord
            for(j=0; j<last_dim; ++j) {
                for(i=0;i<num_as;i++) {
                    offs[i] += coord[j] * as[i]->stride[j];
                    off_oa += coord[j] * oa->stride[j];
                }
            }
            for(j=0; j < oa->shape[last_dim]; j++ ) {
                // computation code
                *(((cphvb_float32*) doa->data) + off_oa) = (*(offs[0]+((cphvb_float32*) as[0]->data)) + ( *((cphvb_float32*) &cs[0]->value)  + *(offs[1]+((cphvb_float32*) as[1]->data))));
                for(i=0;i<num_as;i++) {
                   offs[i] += as[i]->stride[last_dim];
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
        free(coord);
        free(offs);
    }



void kernel_func_4265622974_01(cphvb_array* oa, cphvb_array** as, cphvb_index num_as, cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
        cphvb_index last_dim = oa->ndim-1;
        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( oa->ndim, oa->shape );
        cphvb_index ec = 0;
        cphvb_index off_oa = 0;
        cphvb_index* coord;
        coord = (cphvb_index*) malloc(oa->ndim * sizeof(cphvb_index));
        memset(coord, 0, oa->ndim * sizeof(cphvb_index));
        cphvb_index* offs;
        offs = (cphvb_index*) malloc(num_as * sizeof(cphvb_index));
        int j=0, i=0;
        cphvb_array** ds;
        ds = (cphvb_array**) malloc(num_as * sizeof(cphvb_array*));
        
        cphvb_array* doa = cphvb_base_array(oa);
        for(i=0;i<num_as;i++) {
           ds[i] = cphvb_base_array(as[i]);
        }
        
        if (skip>0) {                                // Create coord based on skip
            while(ec<skip) {
                ec += oa->shape[last_dim];
                for(j = (last_dim-1); j >= 0; --j) {
                    coord[j]++;
                }
            }
        }
        printf("starts: %ld %ld %ld %ld \n",oa->start,as[0]->start,as[1]->start,as[2]->start);
        while( ec < nelements ) {
            for(i=0;i<num_as;i++) {
                offs[i] = as[i]->start;                
            }
            off_oa = oa->start;
            printf("off_oa: %ld\n",off_oa);
            // Compute offset based on coord
            for(j=0; j<last_dim; ++j) {
                for(i=0;i<num_as;i++) {
                    offs[i] += coord[j] * as[i]->stride[j];                    
                }
                off_oa += coord[j] * oa->stride[j];
            }
            printf("off_oa: %ld\n",off_oa);
            for(j=0; j < oa->shape[last_dim]; j++ ) {
                // computation code
                printf("Os: %ld %ld %ld %ld, nelements: %ld\n",off_oa,offs[0],offs[1],offs[2],nelements);
                *(off_oa+((cphvb_float32*) doa->data)) = ((*(offs[0]+((cphvb_float32*) ds[0]->data)) + *(offs[1]+((cphvb_float32*) ds[1]->data))) + *(offs[2]+((cphvb_float32*) ds[2]->data)));

                for(i=0;i<num_as;i++) {
                   offs[i] += as[i]->stride[last_dim];
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
        free(coord);
        free(offs);
    }


#include "stdlib.h"
#include "cphvb_array.h"
#include "cphvb_type.h"
#include "cphvb.h"

#define K_NARRAYS 3
#define K_NCONSTANTS 0
#define K_OUT_NDIM 2
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#if _WIN32
#include <float.h>
#define cphvb_isnan(x) (_isnan(x))
#define cphvb_isinf(x) (!_isnan(x) || !_finite(x))
#else
#define cphvb_isnan(x) (std::isnan(x))
#define cphvb_isinf(x) (std::isinf(x))
#endif
#define DEG_CIR 360.0
#define DEG_RAD (M_PI / (DEG_CIR / 2.0))
#define RAD_DEG ((DEG_CIR / 2.0) / M_PI)



void kernel_func_888379450_1(cphvb_array** as, cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
        cphvb_index last_dim = as[0]->ndim-1;
        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( as[0]->ndim, as[0]->shape );
        cphvb_index ec = 0;
        int j=0, i=0;
        cphvb_index* coord;
        coord = (cphvb_index*) malloc(as[0]->ndim * sizeof(cphvb_index));
        memset(coord, 0, as[0]->ndim * sizeof(cphvb_index));
        cphvb_index* offs;
        offs = (cphvb_index*) malloc(K_NARRAYS * sizeof(cphvb_index));
        cphvb_array** ds;
        ds = (cphvb_array**) malloc(K_NARRAYS * sizeof(cphvb_array*));
        for(i=0;i<K_NARRAYS;i++) {
           ds[i] = cphvb_base_array(as[i]);
        }
        if (skip>0) {                                // Create coord based on skip
            while(ec<skip) {
                ec += as[0]->shape[last_dim];
                for(j = (last_dim-1); j >= 0; --j) {
                    coord[j]++;
                }
            }
        }
        while( ec < nelements ) {
            for(i=0;i<K_NARRAYS;i++) {
                offs[i] = as[i]->start;
            }
            // Compute offset based on coord
            for(j=0; j<last_dim; ++j) {
                for(i=0;i<K_NARRAYS;i++) {
                    offs[i] += coord[j] * as[i]->stride[j];
                }
            }
            for(j=0; j < as[0]->shape[last_dim]; j++ ) {
                // computation code
                *(((cphvb_float64*) ds[0]->data) + offs[0]) = (((((*(offs[0]+((cphvb_float64*) ds[0]->data)) + *(offs[1]+((cphvb_float64*) ds[1]->data))) + *(offs[2]+((cphvb_float64*) ds[2]->data))) + *(offs[3]+((cphvb_float64*) ds[3]->data))) + *(offs[4]+((cphvb_float64*) ds[4]->data))) *  *((cphvb_float64*) &cs[0]->value) );
                for(i=0;i<K_NARRAYS;i++) {
                   offs[i] += as[i]->stride[last_dim];
                }
            }
            ec += as[0]->shape[last_dim];
            for(j = last_dim-1; j >= 0; --j) {
                coord[j]++;
                if (coord[j] < as[0]->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }
        free(coord);
        free(offs);
    }


void kernel_func_3859029243_3(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
        cphvb_index last_dim = as[0]->ndim-1;
        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( as[0]->ndim, as[0]->shape );
        cphvb_index ec = 0;
        cphvb_index coord[K_OUT_NDIM];
       memset(coord, 0, K_OUT_NDIM * sizeof(cphvb_index));
cphvb_float64* DA0 = (cphvb_float64*) cphvb_base_array( as[0])->data;
cphvb_index OffA0;
cphvb_float64* DA1 = (cphvb_float64*) cphvb_base_array( as[1])->data;
cphvb_index OffA1;
cphvb_float64* DA2 = (cphvb_float64*) cphvb_base_array( as[2])->data;
cphvb_index OffA2;
        int j=0, i=0;
        if (skip>0) {                                // Create coord based on skip
            while(ec<skip) {
                ec += as[0]->shape[last_dim];
                for(j = (last_dim-1); j >= 0; --j) {
                    coord[j]++;
                }
            }
        }
        while( ec < nelements ) {
	OffA0 = as[0]->start;
	OffA1 = as[1]->start;
	OffA2 = as[2]->start;
            // Compute offset based on coord
            for(j=0; j<last_dim; ++j) {
		OffA0 += as[0]->stride[j] * coord[j];
		OffA1 += as[1]->stride[j] * coord[j];
		OffA2 += as[2]->stride[j] * coord[j];
            }
            for(j=0; j < as[0]->shape[last_dim]; j++ ) {
                // computation code
                *(OffA0+DA0) = ((*(OffA1 + DA1) - *(OffA2 + DA2)) < 0.0 ? -(*(OffA1 + DA1) - *(OffA2 + DA2)) : (*(OffA1 + DA1) - *(OffA2 + DA2)));
		OffA0 += as[0]->stride[last_dim];
		OffA1 += as[1]->stride[last_dim];
		OffA2 += as[2]->stride[last_dim];
            }
            ec += as[0]->shape[last_dim];
            for(j = last_dim-1; j >= 0; --j) {
                coord[j]++;
                if (coord[j] < as[0]->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }
    }


void kernel_func_888379450_2(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
        cphvb_index last_dim = as[0]->ndim-1;
        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( as[0]->ndim, as[0]->shape );
        cphvb_index ec = 0;
        cphvb_index coord[K_OUT_NDIM];
       memset(coord, 0, K_OUT_NDIM * sizeof(cphvb_index));
cphvb_float64* DA0 = (cphvb_float64*) cphvb_base_array( as[0])->data;
cphvb_index OffA0;
cphvb_float64* DA1 = (cphvb_float64*) cphvb_base_array( as[1])->data;
cphvb_index OffA1;
cphvb_float64* DA2 = (cphvb_float64*) cphvb_base_array( as[2])->data;
cphvb_index OffA2;
cphvb_float64* DA3 = (cphvb_float64*) cphvb_base_array( as[3])->data;
cphvb_index OffA3;
cphvb_float64* DA4 = (cphvb_float64*) cphvb_base_array( as[4])->data;
cphvb_index OffA4;
cphvb_float64* DA5 = (cphvb_float64*) cphvb_base_array( as[5])->data;
cphvb_index OffA5;
        int j=0, i=0;
        if (skip>0) {                                // Create coord based on skip
            while(ec<skip) {
                ec += as[0]->shape[last_dim];
                for(j = (last_dim-1); j >= 0; --j) {
                    coord[j]++;
                }
            }
        }
        while( ec < nelements ) {
            OffA0 = as[0]->start;
            OffA1 = as[1]->start;
            OffA2 = as[2]->start;
            OffA3 = as[3]->start;
            OffA4 = as[4]->start;
            OffA5 = as[5]->start;
            // Compute offset based on coord
            for(j=0; j<last_dim; ++j) {
                OffA0 += as[0]->stride[j] * coord[j];
                OffA1 += as[1]->stride[j] * coord[j];
                OffA2 += as[2]->stride[j] * coord[j];
                OffA3 += as[3]->stride[j] * coord[j];
                OffA4 += as[4]->stride[j] * coord[j];
                OffA5 += as[5]->stride[j] * coord[j];
            }
            for(j=0; j < as[0]->shape[last_dim]; j++ ) {
                // computation code
                *(OffA0+DA0) = (((((*(OffA1 + DA1) + *(OffA2 + DA2)) + *(OffA3 + DA3)) + *(OffA4 + DA4)) + *(OffA5 + DA5)) *  *((cphvb_float64*) &cs[0]->value) );
                OffA0 += as[0]->stride[last_dim];
                OffA1 += as[1]->stride[last_dim];
                OffA2 += as[2]->stride[last_dim];
                OffA3 += as[3]->stride[last_dim];
                OffA4 += as[4]->stride[last_dim];
                OffA5 += as[5]->stride[last_dim];
            }
            ec += as[0]->shape[last_dim];
            for(j = last_dim-1; j >= 0; --j) {
                coord[j]++;
                if (coord[j] < as[0]->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }
    }

// A+B
void kernel_func_1899990464_0(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
        cphvb_index last_dim = as[0]->ndim-1;
        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( as[0]->ndim, as[0]->shape );
        cphvb_index ec = 0;
        cphvb_index coord[K_OUT_NDIM];
       memset(coord, 0, K_OUT_NDIM * sizeof(cphvb_index));
cphvb_float32* DA0 = (cphvb_float32*) cphvb_base_array( as[0])->data;
cphvb_index OffA0;
cphvb_float32* DA1 = (cphvb_float32*) cphvb_base_array( as[1])->data;
cphvb_index OffA1;
cphvb_float32* DA2 = (cphvb_float32*) cphvb_base_array( as[2])->data;
cphvb_index OffA2;
        int j=0, i=0;
        if (skip>0) {                                // Create coord based on skip
            while(ec<skip) {
                ec += as[0]->shape[last_dim];
                for(j = (last_dim-1); j >= 0; --j) {
                    coord[j]++;
                }
            }
        }
        while( ec < nelements ) {
	OffA0 = as[0]->start;
	OffA1 = as[1]->start;
	OffA2 = as[2]->start;
            // Compute offset based on coord
            for(j=0; j<last_dim; ++j) {
		OffA0 += as[0]->stride[j] * coord[j];
		OffA1 += as[1]->stride[j] * coord[j];
		OffA2 += as[2]->stride[j] * coord[j];
            }
            for(j=0; j < as[0]->shape[last_dim]; j++ ) {
                // computation code
                *(OffA0 + DA0) = (*(OffA1 + DA1) + *(OffA2 + DA2));
		OffA0 += as[0]->stride[last_dim];
		OffA1 += as[1]->stride[last_dim];
		OffA2 += as[2]->stride[last_dim];
            }
            ec += as[0]->shape[last_dim];
            for(j = last_dim-1; j >= 0; --j) {
                coord[j]++;
                if (coord[j] < as[0]->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }
    }

// C
void kernel_func_4265622974_0(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
        cphvb_index last_dim = as[0]->ndim-1;
        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( as[0]->ndim, as[0]->shape );
        cphvb_index ec = 0;
        cphvb_index coord[K_OUT_NDIM];
       memset(coord, 0, K_OUT_NDIM * sizeof(cphvb_index));
cphvb_float32* DA0 = (cphvb_float32*) cphvb_base_array( as[0])->data;
cphvb_index OffA0;
cphvb_float32* DA1 = (cphvb_float32*) cphvb_base_array( as[1])->data;
cphvb_index OffA1;
cphvb_float32* DA2 = (cphvb_float32*) cphvb_base_array( as[2])->data;
cphvb_index OffA2;
cphvb_float32* DA3 = (cphvb_float32*) cphvb_base_array( as[3])->data;
cphvb_index OffA3;
        int j=0, i=0;
        if (skip>0) {                                // Create coord based on skip
            while(ec<skip) {
                ec += as[0]->shape[last_dim];
                for(j = (last_dim-1); j >= 0; --j) {
                    coord[j]++;
                }
            }
        }
        while( ec < nelements ) {
	OffA0 = as[0]->start;
	OffA1 = as[1]->start;
	OffA2 = as[2]->start;
	OffA3 = as[3]->start;
            // Compute offset based on coord
            for(j=0; j<last_dim; ++j) {
		OffA0 += as[0]->stride[j] * coord[j];
		OffA1 += as[1]->stride[j] * coord[j];
		OffA2 += as[2]->stride[j] * coord[j];
		OffA3 += as[3]->stride[j] * coord[j];
            }
            for(j=0; j < as[0]->shape[last_dim]; j++ ) {
                // computation code
                *(OffA0 + DA0) = ((*(OffA1 + DA1) + *(OffA2 + DA2)) + *(OffA3 + DA3));
		OffA0 += as[0]->stride[last_dim];
		OffA1 += as[1]->stride[last_dim];
		OffA2 += as[2]->stride[last_dim];
		OffA3 += as[3]->stride[last_dim];
            }
            ec += as[0]->shape[last_dim];
            for(j = last_dim-1; j >= 0; --j) {
                coord[j]++;
                if (coord[j] < as[0]->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }
    }

// D
void kernel_func_1625152366_0(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
        cphvb_index last_dim = as[0]->ndim-1;
        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( as[0]->ndim, as[0]->shape );
        cphvb_index ec = 0;
        cphvb_index coord[K_OUT_NDIM];
       memset(coord, 0, K_OUT_NDIM * sizeof(cphvb_index));
cphvb_float32* DA0 = (cphvb_float32*) cphvb_base_array( as[0])->data;
cphvb_index OffA0;
cphvb_float32* DA1 = (cphvb_float32*) cphvb_base_array( as[1])->data;
cphvb_index OffA1;
cphvb_float32* DA2 = (cphvb_float32*) cphvb_base_array( as[2])->data;
cphvb_index OffA2;
cphvb_float32* DA3 = (cphvb_float32*) cphvb_base_array( as[3])->data;
cphvb_index OffA3;
cphvb_float32* DA4 = (cphvb_float32*) cphvb_base_array( as[4])->data;
cphvb_index OffA4;
        int j=0, i=0;
        if (skip>0) {                                // Create coord based on skip
            while(ec<skip) {
                ec += as[0]->shape[last_dim];
                for(j = (last_dim-1); j >= 0; --j) {
                    coord[j]++;
                }
            }
        }
        while( ec < nelements ) {
	OffA0 = as[0]->start;
	OffA1 = as[1]->start;
	OffA2 = as[2]->start;
	OffA3 = as[3]->start;
	OffA4 = as[4]->start;
            // Compute offset based on coord
            for(j=0; j<last_dim; ++j) {
		OffA0 += as[0]->stride[j] * coord[j];
		OffA1 += as[1]->stride[j] * coord[j];
		OffA2 += as[2]->stride[j] * coord[j];
		OffA3 += as[3]->stride[j] * coord[j];
		OffA4 += as[4]->stride[j] * coord[j];
            }
            for(j=0; j < as[0]->shape[last_dim]; j++ ) {
                // computation code
                *(OffA0 + DA0) = (((*(OffA1 + DA1) + *(OffA2 + DA2)) + *(OffA3 + DA3)) + *(OffA4 + DA4));
		OffA0 += as[0]->stride[last_dim];
		OffA1 += as[1]->stride[last_dim];
		OffA2 += as[2]->stride[last_dim];
		OffA3 += as[3]->stride[last_dim];
		OffA4 += as[4]->stride[last_dim];
            }
            ec += as[0]->shape[last_dim];
            for(j = last_dim-1; j >= 0; --j) {
                coord[j]++;
                if (coord[j] < as[0]->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }
    }

#define K_OUT_NDIM 2
void kernel_func_3286243741_0(cphvb_array** as,cphvb_constant** cs, cphvb_index skip, cphvb_index limit) {
        cphvb_index last_dim = as[0]->ndim-1;
        cphvb_index nelements = (limit>0) ? limit : cphvb_nelements( as[0]->ndim, as[0]->shape );
        cphvb_index ec = 0;
        cphvb_index coord[K_OUT_NDIM];
       memset(coord, 0, K_OUT_NDIM * sizeof(cphvb_index));
cphvb_float32* DA0 = (cphvb_float32*) cphvb_base_array( as[0])->data;
cphvb_index OffA0;
cphvb_float32* DA1 = (cphvb_float32*) cphvb_base_array( as[1])->data;
cphvb_index OffA1;
cphvb_float32* DA2 = (cphvb_float32*) cphvb_base_array( as[2])->data;
cphvb_index OffA2;
cphvb_float32* DA3 = (cphvb_float32*) cphvb_base_array( as[3])->data;
cphvb_index OffA3;
cphvb_float32* DA4 = (cphvb_float32*) cphvb_base_array( as[4])->data;
cphvb_index OffA4;
cphvb_float32* DA5 = (cphvb_float32*) cphvb_base_array( as[5])->data;
cphvb_index OffA5;
        int j=0, i=0;
        if (skip>0) {                                // Create coord based on skip
            while(ec<skip) {
                ec += as[0]->shape[last_dim];
                for(j = (last_dim-1); j >= 0; --j) {
                    coord[j]++;
                }
            }
        }
        while( ec < nelements ) {
	OffA0 = as[0]->start;
	OffA1 = as[1]->start;
	OffA2 = as[2]->start;
	OffA3 = as[3]->start;
	OffA4 = as[4]->start;
	OffA5 = as[5]->start;
            // Compute offset based on coord
            for(j=0; j<last_dim; ++j) {
		OffA0 += as[0]->stride[j] * coord[j];
		OffA1 += as[1]->stride[j] * coord[j];
		OffA2 += as[2]->stride[j] * coord[j];
		OffA3 += as[3]->stride[j] * coord[j];
		OffA4 += as[4]->stride[j] * coord[j];
		OffA5 += as[5]->stride[j] * coord[j];
            }
            for(j=0; j < as[0]->shape[last_dim]; j++ ) {
                // computation code
                *(OffA0 + DA0) = ((((*(OffA1 + DA1) + *(OffA2 + DA2)) + *(OffA3 + DA3)) + *(OffA4 + DA4)) + *(OffA5 + DA5));
		OffA0 += as[0]->stride[last_dim];
		OffA1 += as[1]->stride[last_dim];
		OffA2 += as[2]->stride[last_dim];
		OffA3 += as[3]->stride[last_dim];
		OffA4 += as[4]->stride[last_dim];
		OffA5 += as[5]->stride[last_dim];
            }
            ec += as[0]->shape[last_dim];
            for(j = last_dim-1; j >= 0; --j) {
                coord[j]++;
                if (coord[j] < as[0]->shape[j]) {
                    break;
                } else {
                    coord[j] = 0;
                }
            }
        }
    }
