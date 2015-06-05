#include <bh.h>
#include <stdio.h>
#include <bh_opcode.h>
#include "helper_functions.h"
#include <stack>
#include <unordered_map>
#include <optimization.h>



using namespace std;

#define DEBUG (true)


/*typedef struct
{
    bh_view*      view;
    int       exponent;
    int      index;
}power_cache;
*/


class Power_cache
{
    public:
        void add(bh_base *base, int exponent, int index) {
            if (previous_powers.count(base) == 0) {
                previous_powers.insert({{base, new map<int, int>()}});
            }
            previous_powers[base]->insert({{exponent, index}});
            #if DEBUG
                cout << "Added power cahce with exponent: " << exponent << ", index: " << index << " and base: " << base << endl;
            #endif
        }
        int get_closest(bh_base *base, int exponent) {
            if (previous_powers.count(base) > 0) {
                int idx = -1;
                #if DEBUG
                    cout << "Try finding closest" << endl;
                #endif
                for (std::map<int,int>::iterator it=previous_powers[base]->begin(); it!=previous_powers[base]->end(); ++it) {
                    if (it->first > exponent)
                        break;
                    idx = it->second;
                }
                #if DEBUG
                    cout << "Closest idx found: " << idx << endl;
                #endif
                return idx;
            }
            #if DEBUG
                cout << "No closest found" << endl;
            #endif
            return -1;
        }
    private:
        std::unordered_map< bh_base*, map<int, int>* > previous_powers;
};

// Power-multipy replacement
void Optimization::power_cache(bh_ir *bhir) {

    Power_cache power_cache = Power_cache();

    for(std::size_t i = 0; i < bhir->instr_list.size(); ++i){

        // TODO: remove from cache if altered

        if ((bh_opcode)bhir->instr_list[i].opcode == (bh_opcode)BH_POWER && bh_is_constant(&bhir->instr_list[i].operand[2])) {
            if (const_is_int(&bhir->instr_list[i])) {
                #if DEBUG
                    helper_pprint_instr_list(bhir);
                    cout << "i: " << i << endl;
                #endif

                int exponent = const_get_int(&bhir->instr_list[i]);
                int exponent_original = exponent;
                bh_base* base_to_cache = bhir->instr_list[i].operand[1].base;

                if (exponent > 1) {

                    int closest = power_cache.get_closest(bhir->instr_list[i].operand[1].base, exponent);

                    #if DEBUG
                        cout << "closest: " << closest << endl;
                    #endif

                    //if (closest != -1 && bh_view_equally_shifted(&bhir->instr_list[i].operand[1], &bhir->instr_list[closest].operand[1])) {
                    if (closest != -1) {
                        exponent -= const_get_int(&bhir->instr_list[closest]);
                    }

                    std::vector<bh_instruction> instr_sequence;
                    if (exponent > 1) {
                        #if DEBUG
                            cout << "new exponent: " << exponent << endl;
                        #endif

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

                        while(!mult_operand_sequence.empty()) {
                            bh_view operand [3] = {bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[0], bhir->instr_list[i].operand[mult_operand_sequence.top()] };
                            bh_instruction multiply = { (bh_opcode)BH_MULTIPLY };
                            memcpy(&multiply.operand, &operand, sizeof(operand));


                            instr_sequence.push_back(multiply);
                            
                            mult_operand_sequence.pop();
                        }
                        
                    }
                    #if DEBUG
                        printf("\n5\n");
                    #endif

                    if (closest != -1) {
                        if (exponent == 0) {
                            bhir->instr_list[i].opcode = (bh_opcode)BH_IDENTITY;
                            bhir->instr_list[i].operand[1] = bhir->instr_list[closest].operand[0]; 
                        }
                        else {
                            bh_instruction multiply = { (bh_opcode)BH_MULTIPLY };
                            multiply.operand[0] = bhir->instr_list[i].operand[0];
                            multiply.operand[1] = bhir->instr_list[closest].operand[0];

                            if (exponent > 1) {
                                multiply.operand[2] = instr_sequence.back().operand[0];
                            }
                            if (exponent == 1) {
                                #if DEBUG
                                    printf("\n5.3\n");
                                #endif
                                multiply.operand[2] = bhir->instr_list[i].operand[1];
                                #if DEBUG
                                    printf("\n5.4\n");
                                #endif
                                bhir->instr_list.erase(bhir->instr_list.begin()+i);
                                #if DEBUG
                                    printf("\n5.5\n");
                                #endif
                            }
                            instr_sequence.push_back(multiply);
                        }
                    }


                    // Find possible datamanagement instructions for reused base
                    int offset = 0;
                    if (closest != -1) {
                        for(std::size_t j = closest; j < i; ++j){
                            if (operation_datamanagement(bhir->instr_list[j].opcode) && bhir->instr_list[j].operand[0].base == bhir->instr_list[closest].operand[0].base) {
                                instr_sequence.push_back(bhir->instr_list[j]);
                                bhir->instr_list.erase(bhir->instr_list.begin()+j);
                                --j;
                                --offset;
                            }
                        }
                    }


                    bhir->instr_list.insert(bhir->instr_list.begin()+i+offset+ (exponent == 1 ? 0 : 1), instr_sequence.begin(), instr_sequence.end());
                    #if DEBUG
                        printf("\n6\n");
                        cout << "i: " << i << ", instr_sequence size: " << instr_sequence.size() << ", -1or0: " << (exponent == 1 ? 1 : 0) << ", offset: " << offset << endl;
                        cout << "calculated offset: " << i+instr_sequence.size()-(exponent == 1 ? 1 : 0)+offset << endl;
                    #endif
                    power_cache.add(base_to_cache, exponent_original, i+instr_sequence.size()-(exponent == 1 ? 1 : 0)+offset*2);
                    #if DEBUG
                        printf("\n7\n");
                    #endif
                }
                else if (exponent == 1) {
                    bhir->instr_list[i].opcode = (bh_opcode)BH_IDENTITY;
                }

                #if DEBUG
                    printf("\nAfter running optimizations\n");
                    helper_pprint_instr_list(bhir);
                #endif
            }
        }
    }
    #if DEBUG
        printf("\nDONE\n");
    #endif
}