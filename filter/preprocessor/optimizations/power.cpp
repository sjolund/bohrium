#include <bh.h>
#include <bh_pprint.h>
#include <stdio.h>
#include <bh_opcode.h>
#include "helper_functions.h"
#include <stack>
#include <string>
#include <sstream>
#include <chrono>
#include <algorithm> 
#include <optimization.h>


using namespace std;

#define DEBUG (false)

// Power-multipy replacement
void Optimization::power(bh_ir *bhir) {
    #if DEBUG
        //helper_pprint_instr_list(bhir);
    #endif

    for(std::size_t i = 0; i < bhir->instr_list.size(); ++i){
        if ((bh_opcode)bhir->instr_list[i].opcode == (bh_opcode)BH_POWER && bh_is_constant(&bhir->instr_list[i].operand[2])) {
            if (const_is_int(&bhir->instr_list[i]) && const_get_int(&bhir->instr_list[i]) > 1) {
                #if DEBUG
                    bh_pprint_instr_list(&bhir->instr_list[max(0,(int)i-2)], min(5, (int)bhir->instr_list.size()), "Number of instructions in whole batch before power-mult replace");
                #endif

                #if DEBUG
                    auto begin = chrono::high_resolution_clock::now();
                #endif
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
                    

                    /*insertView(*bhir
                        , mult_sequence.end()
                        , BH_MULTIPLY
                        , &bhir->instr_list[i].operand[0]
                        , &bhir->instr_list[i].operand[0]
                        , &bhir->instr_list[i].operand[mult_operand_sequence.top()]
                    );*/
                    mult_operand_sequence.pop();
                }
                bhir->instr_list.insert(bhir->instr_list.begin()+i+1, mult_sequence.begin(), mult_sequence.end());

                #if DEBUG
                    auto end = chrono::high_resolution_clock::now();    
                    auto dur = end - begin;
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
                    cout << "duration: " << ms << endl;
                #endif

                #if DEBUG
                    printf("\n\nPower %d replaced with multiply:\n\n\n\n", const_get_int(&bhir->instr_list[i]));
                    bh_pprint_instr_list(&bhir->instr_list[max(0,(int)i-2)], min(5+(int)mult_sequence.size(), (int)bhir->instr_list.size()), "Number of instructions in whole batch after power-mult replace");
                    //helper_pprint_instr_list(bhir);
                #endif
            }
        }
    }

    /*
    for(std::size_t i = 0; i < bhir->instr_list.size(); ++i){
        if ((bh_opcode)bhir->instr_list[i].opcode == (bh_opcode)BH_POWER && bh_is_constant(&bhir->instr_list[i].operand[2])) {
            if (const_equals(&bhir->instr_list[i], 2)) {
                bhir->instr_list[i].opcode = (bh_opcode)BH_MULTIPLY;
                bhir->instr_list[i].operand[2] = bhir->instr_list[i].operand[1];

                #if DEBUG
                    printf("\n\nPower 2 replaced with multiply:\n\n\n\n");
                    helper_pprint_instr_list(bhir);
                #endif
            }
            else if (const_equals(&bhir->instr_list[i], 3)) {
                bhir->instr_list[i].opcode = (bh_opcode)BH_MULTIPLY;
                bhir->instr_list[i].operand[2] = bhir->instr_list[i].operand[1];

                bh_view operand [3] = {bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[1] };
                bh_instruction secondMultiply = { (bh_opcode)BH_MULTIPLY };
                memcpy(&secondMultiply.operand, &operand, sizeof(operand));
                
                bhir->instr_list.insert(bhir->instr_list.begin()+(++i), secondMultiply);
                #if DEBUG
                    printf("\n\nPower 3 replaced with multiply:\n\n\n\n");
                    helper_pprint_instr_list(bhir);
                #endif
            }
            else if (const_equals(&bhir->instr_list[i], 4)) {
                bhir->instr_list[i].opcode = (bh_opcode)BH_MULTIPLY;
                bhir->instr_list[i].operand[2] = bhir->instr_list[i].operand[1];

                bh_view operand [3] = {bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0] };
                bh_instruction secondMultiply = { (bh_opcode)BH_MULTIPLY };
                memcpy(&secondMultiply.operand, &operand, sizeof(operand));
                
                bhir->instr_list.insert(bhir->instr_list.begin()+(++i), secondMultiply);
                #if DEBUG
                    printf("\n\nPower 4 replaced with multiply:\n\n\n\n");
                    helper_pprint_instr_list(bhir);
                #endif
            }
            else if (const_equals(&bhir->instr_list[i], 5)) {
                bhir->instr_list[i].opcode = (bh_opcode)BH_MULTIPLY;
                bhir->instr_list[i].operand[2] = bhir->instr_list[i].operand[1];

                bh_view operand2 [3] = {bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0] };
                bh_instruction multiply2 = { (bh_opcode)BH_MULTIPLY };
                memcpy(&multiply2.operand, &operand2, sizeof(operand2));
                
                bhir->instr_list.insert(bhir->instr_list.begin()+i+1, multiply2);

                bh_view operand3 [3] = {bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[1] };
                bh_instruction multiply3 = { (bh_opcode)BH_MULTIPLY };
                memcpy(&multiply3.operand, &operand3, sizeof(operand3));
                
                bhir->instr_list.insert(bhir->instr_list.begin()+i+2, multiply3);
                // Increment i by 2???
                #if DEBUG
                    printf("\n\nPower 5 replaced with multiply:\n\n\n\n");
                    helper_pprint_instr_list(bhir);
                #endif
            }
            else if (const_equals(&bhir->instr_list[i], 6)) {
                bhir->instr_list[i].opcode = (bh_opcode)BH_MULTIPLY;
                bhir->instr_list[i].operand[2] = bhir->instr_list[i].operand[1];

                bh_view operand2 [3] = {bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[1] };
                bh_instruction multiply2 = { (bh_opcode)BH_MULTIPLY };
                memcpy(&multiply2.operand, &operand2, sizeof(operand2));
                
                bhir->instr_list.insert(bhir->instr_list.begin()+i+1, multiply2);

                bh_view operand3 [3] = {bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0] };
                bh_instruction multiply3 = { (bh_opcode)BH_MULTIPLY };
                memcpy(&multiply3.operand, &operand3, sizeof(operand3));
                
                bhir->instr_list.insert(bhir->instr_list.begin()+i+2, multiply3);
                #if DEBUG
                    printf("\n\nPower 6 replaced with multiply:\n\n\n\n");
                    helper_pprint_instr_list(bhir);
                #endif
            }
            else if (const_equals(&bhir->instr_list[i], 7)) {
                bhir->instr_list[i].opcode = (bh_opcode)BH_MULTIPLY;
                bhir->instr_list[i].operand[2] = bhir->instr_list[i].operand[1];

                bh_view operand2 [3] = {bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[1] };
                bh_instruction multiply2 = { (bh_opcode)BH_MULTIPLY };
                memcpy(&multiply2.operand, &operand2, sizeof(operand2));
                
                bhir->instr_list.insert(bhir->instr_list.begin()+i+1, multiply2);

                bh_view operand3 [3] = {bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0] };
                bh_instruction multiply3 = { (bh_opcode)BH_MULTIPLY };
                memcpy(&multiply3.operand, &operand3, sizeof(operand3));
                
                bhir->instr_list.insert(bhir->instr_list.begin()+i+2, multiply3);

                bh_view operand4 [3] = {bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[1] };
                bh_instruction multiply4 = { (bh_opcode)BH_MULTIPLY };
                memcpy(&multiply4.operand, &operand4, sizeof(operand4));
                
                bhir->instr_list.insert(bhir->instr_list.begin()+i+3, multiply4);
                #if DEBUG
                    printf("\n\nPower 7 replaced with multiply:\n\n\n\n");
                    helper_pprint_instr_list(bhir);
                #endif
            }
            else if (const_equals(&bhir->instr_list[i], 8)) {
                bhir->instr_list[i].opcode = (bh_opcode)BH_MULTIPLY;
                bhir->instr_list[i].operand[2] = bhir->instr_list[i].operand[1];

                bh_view operand2 [3] = {bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0] };
                bh_instruction multiply2 = { (bh_opcode)BH_MULTIPLY };
                memcpy(&multiply2.operand, &operand2, sizeof(operand2));
                
                bhir->instr_list.insert(bhir->instr_list.begin()+i+1, multiply2);

                bh_view operand3 [3] = {bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0] };
                bh_instruction multiply3 = { (bh_opcode)BH_MULTIPLY };
                memcpy(&multiply3.operand, &operand3, sizeof(operand3));
                
                bhir->instr_list.insert(bhir->instr_list.begin()+i+2, multiply3);
                #if DEBUG
                    printf("\n\nPower 8 replaced with multiply:\n\n\n\n");
                    helper_pprint_instr_list(bhir);
                #endif
            }
        }
    }
    */
}