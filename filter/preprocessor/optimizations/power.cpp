#include <bh.h>
#include <stdio.h>
#include <bh_opcode.h>
#include "helper_functions.h"
#include <stack>
#include <optimization.h>


using namespace std;

// Power-multipy replacement
void Optimization::power(bh_ir *bhir) {

    for(std::size_t i = 0; i < bhir->instr_list.size(); ++i){
        if ((bh_opcode)bhir->instr_list[i].opcode == (bh_opcode)BH_POWER && bh_is_constant(&bhir->instr_list[i].operand[2])) {
            if (const_is_int(&bhir->instr_list[i]) && const_get_int(&bhir->instr_list[i]) > 1) {

                int exponent = const_get_int(&bhir->instr_list[i]);
                bhir->instr_list[i].opcode = (bh_opcode)BH_MULTIPLY;
                bhir->instr_list[i].operand[2] = bhir->instr_list[i].operand[1];

                
                std::stack<int> mult_operand_sequence;
                while(exponent > 2) {
                    // Exponent is odd
                    if (exponent & 0x1) {
                        mult_operand_sequence.push(1);
                        exponent -= 1;
                    }
                    // Exponent is even
                    else {
                        mult_operand_sequence.push(0);
                        exponent = exponent >> 1; // divided by two
                    }

                }

                std::vector<bh_instruction> mult_sequence;
                while(!mult_operand_sequence.empty()) {
                    bh_view operand [3] = {bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[mult_operand_sequence.top()] };
                    bh_instruction multiply = { (bh_opcode)BH_MULTIPLY };
                    memcpy(&multiply.operand, &operand, sizeof(operand));


                    mult_sequence.push_back(multiply);
                    
                    mult_operand_sequence.pop();
                }
                bhir->instr_list.insert(bhir->instr_list.begin()+i+1, mult_sequence.begin(), mult_sequence.end());
            }
        }
    }
}