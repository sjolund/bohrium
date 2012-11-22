//
//
// do SSA ( single static assignment) 
// No control flow ! just assignments of the form  array = expr.

// 1) lookop name (target) to find version and references (the expression side references must be renamed)

// for SSA, a name table is needed wich points to a SSAVersionTable


//

//#include <cphvb.h>
//#include <iostream>
//#include <map>
//#include <utility>

//using namespace std;

//typedef enum  { bin_op, un_op, const_val, array_val} ExpType;
                        
//typedef struct Exp {
    //ExpType                                         tag;
    //int                                             id;  
                                                   
    //union { cphvb_constant*                         constant;
            //cphvb_array*                            array;
                                
            //struct {    cphvb_opcode    opcode;
                        //struct Exp*     left;
                        //struct Exp*     right; }    binary;

            //struct {    cphvb_opcode    opcode;
                        //struct Exp*     left; }     unary;
    //} op;
//} ast;

//typedef struct SSA_Version_Table {
    //cphvb_intp  version;
    //std::map<cphvb_intp, ast*> table;
//} ssavt;


//cphvb_error array_to_exp(cphvb_array *array, ast *result) {        
    //ast* exp = (ast*) malloc(sizeof(ast));        
    //exp->tag = array_val;
    //exp->op.array = array;    

    //result = exp;
    //return CPHVB_SUCCESS;
//}

//cphvb_error const_to_exp(cphvb_constant *constant, ast *result) {        
    //ast* exp = (ast*) malloc(sizeof(ast));        
    //exp->tag = const_val;
    //exp->op.constant = constant;    

    //result = exp;
    //return CPHVB_SUCCESS;
//}


//cphvb_error operand_to_exp(cphvb_instruction *inst, cphvb_intp operand_no, ast *result) {
    //if (cphvb_is_constant(inst->operand[operand_no])) {
        //return const_to_exp(&inst->constant,result);
    //} else {
        //return array_to_exp(inst->operand[operand_no],result);
    //}
//}

//cphvb_error create_ast_from_instruction(cphvb_instruction *inst, ast *result) {        
    //ast* exp = (ast*) malloc(sizeof(ast));        
    
    //if(cphvb_operands(inst->opcode) == 3) {
        //exp->tag = bin_op;
        //exp->op.binary.opcode = inst->opcode;
        
        //if (operand_to_exp(inst,1,exp->op.binary.left) != CPHVB_SUCCESS) {
            //// print a error message ??
            //return CPHVB_ERROR;
        //}        
        //if (operand_to_exp(inst,2,exp->op.binary.right) != CPHVB_SUCCESS) {
            //// print a error message ??
            //return CPHVB_ERROR;
        //}

    //} else {
        //exp->tag = un_op;
        //exp->op.binary.opcode = inst->opcode;
        //if (operand_to_exp(inst,1,exp->op.binary.left) != CPHVB_SUCCESS) {
            //// print a error message ??
            //return CPHVB_ERROR;
        //}
    //}

    //result = exp;
    //return CPHVB_SUCCESS;
//}



//cphvb_error instruction_to_stm(cphvb_instruction *inst, map<cphvb_intp,cphvb_intp> name_mapping, Stm *result) {
    //Stm* stm = (Stm*) malloc(sizeof(Stm));

    //// loopup name , exists = get version, else version = 0    
    //cphvb_intp operand_name = inst->operand[0];
    //cphvb_intp count = name_mapping[operand_name];
    //count++;

    //stm->name = operand_name;
    //stm->version = count;

    //// create expression from instruction
    //create_ast_from_instruction(inst,stm->exp);    
//}


// SSA functions


/////**  
 //* Lookup the SSA name and exp for the array in the ssalist. If the array
 //* exists the ssa_version_table for the base array is returned.
 //* If it does not exists out is null
 //* */
//cphvb_error ssa_lookup(map<cphvb_array*,ssavt*> ssalist, cphvb_array* array, ssavt* out) {
    
    //// error handling    
    //map<cphvb_array*,ssavt*>::iterator it = ssalist.find(array);
    //if(it == ssalist.end()) {
        //out = NULL;        
    //} else {
        //out = it->second;
    //}
    
    //return CPHVB_SUCCESS;
//}

//// cphvb_instruction* inst
//cphvb_error ssa_add(map<cphvb_array*,ssavt*> ssalist, cphvb_array* key_pointer, ast* expr) {    
    //ssavt* vt;
    //cphvb_error result;
    //// lookup the out_base
    //result = ssa_lookup(ssalist, key_pointer, vt);
    //if (result != CPHVB_SUCCESS) {
        //return result;
    //}
        
    //if (vt == NULL) {
        //// no entry exist for the array, create one.
        //vt = (ssavt*) malloc(sizeof(ssavt));
        //vt->version = 0;
        //vt->table = map<cphvb_intp,ast*> ();        
        //vt->table[0] = expr;
        
        //ssalist[key_pointer] = vt;
        
    //} else {
        
        //vt->version++;
        //vt->table[vt->version] = expr;                
    //}
    

    //// assign

    //return CPHVB_SUCCESS;    
//}



//// create a very simple ast
//int main(int argc,char** argv) {
        
    ////printf("test\n");
    //return 0;
//}







