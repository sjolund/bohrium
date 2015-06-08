#include <bh.h>
#include <stdio.h>
#include <bh_opcode.h>
#include "helper_functions.h"
#include <stack>
#include <optimization.h>
#include <unordered_map>


using namespace std;

#define DEBUG (false)

void remove_unused_identity_backwards(bh_ir *bhir, int start, int stop) {
    bh_instruction *instr;
    std::unordered_map<bh_base*, int> freed;
    for (; start >= stop; start--) {
        instr = &bhir->instr_list[start];
        if (instr->opcode == (bh_opcode)BH_FREE) {
            // base is added to the list of freed bases
            freed.insert( {{instr->operand[0].base, start}} );
        }
        else if (bh_operands(instr->opcode) > 1) {
            if (freed.count(instr->operand[0].base) == 1) {
                // Unused assignment of base - can be a NONE operation
                instr->opcode = BH_NONE;
                freed.erase(instr->operand[0].base);
            }
            else {
                if (!bh_is_constant(&instr->operand[1])) {
                    // base is used and no preceeding operations can be NONE on behave of base
                    freed.erase(instr->operand[1].base);
                }
                if (bh_operands(instr->opcode) == 3 && !bh_is_constant(&instr->operand[2])) {
                    // base is used and no preceeding operations can be NONE on behave of base
                    freed.erase(instr->operand[2].base);
                }
            }
        }
    }
}

// Common shifted array operation
void Optimization::common(bh_ir *bhir) {

    std::vector<std::size_t> identical;
    std::vector<int> expand;

    bh_view referenceView;
    bh_view referenceView2;

    bh_instruction* instr;
    bh_instruction* instr2;

    for(uint i = 0; i < bhir->instr_list.size(); ++i){

        instr = &bhir->instr_list[i];

        if (bh_operands(instr->opcode) < 2 || operation_datamanagement(instr->opcode)) {
            continue;
        }

        // If the result array has same base as one of the operands, the operand i changed and cannot be used for common shifted array operation
        if (bh_view_same_base(&instr->operand[0], &instr->operand[1])
            || (bh_operands(instr->opcode) == 3 && bh_view_same_base(&instr->operand[0], &instr->operand[2]))) {
            continue;
        }

        identical.clear();
        expand = {0};

        for(std::size_t j = i+1; j < bhir->instr_list.size(); ++j){
            instr2 = &bhir->instr_list[j];
            if (operation_datamanagement(instr2->opcode)) {
                continue;
            }
            if (identical_instr(instr,instr2) && instr->opcode != (bh_opcode)BH_IDENTITY)
            {
                identical.push_back(j);
                // Push to list of identical
            }
            else if (bh_view_same_base(&instr->operand[0], &instr2->operand[0]) 
                     || bh_view_same_base(&instr->operand[1], &instr2->operand[0]) 
                     || (bh_operands(instr->opcode) == 3 && bh_view_same_base(&instr->operand[2], &instr2->operand[0])))
            {
                // instruction modifies the value we will be working on, hence no more identical operations can be found.
                break;
            }
        }

        #if DEBUG
            cout << "Finished searcing for identicals. Found: " << identical.size() << endl;
        #endif
        if (identical.size() > 0) {
            // Expand indenticals (offsets)
            // Make hard copy, since it can be overwritten at a later point.
            bh_view *tempReferenceView = instruction_get_view_of_interest(&bhir->instr_list[i]);
            
            if (tempReferenceView != NULL) {
                referenceView = *tempReferenceView;
                std::unordered_map<bh_base*,int> unusedResults = {{bhir->instr_list[i].operand[0].base,0}};

                for(uint o = 1; o < identical[0]-i; ++o){
                    instr = &bhir->instr_list[i+o];
                    if (operation_datamanagement(instr->opcode) || bh_operands(instr->opcode) < 2) {
                        continue;
                    }

                    // Check if fits for expansion
                    bh_view *viewOne = &instr->operand[1];
                    bh_view *viewTwo = &instr->operand[2];
                    int resultOneOffset = -1;
                    int resultTwoOffset = -1;

                    if (!bh_is_constant(viewOne)) {
                        if (unusedResults.count(viewOne->base) == 1) {
                            resultOneOffset = expand[unusedResults[viewOne->base]];
                            unusedResults.erase(viewOne->base);
                        }
                        else if (!bh_view_equally_shifted(&referenceView, viewOne)) {
                            continue;
                        }
                    }
                    if (bh_operands(instr->opcode) == 3 && !bh_is_constant(viewTwo)) {
                        if (unusedResults.count(viewTwo->base) == 1) {
                            resultTwoOffset = expand[unusedResults[viewTwo->base]];
                            unusedResults.erase(viewTwo->base);
                        }
                        else if (!bh_view_equally_shifted(&referenceView, viewTwo)) {
                            continue;
                        }
                    }




                    // instr is not violating the common shift invaraint

                    // Check if expansion is valid for all identicals
                    bool is_identical = true;
                    for (std::vector<std::size_t>::iterator iter = identical.begin(); iter < identical.end(); iter++) {
                        if ((*iter)+o >= bhir->instr_list.size()) {
                            is_identical = false;
                            #if DEBUG
                                cout << "1 " << endl;
                            #endif
                            break;
                        }
                        instr2 = &bhir->instr_list[(*iter)+o];

                        referenceView2 = *instruction_get_view_of_interest(instr2);
                        viewOne = &instr2->operand[1];
                        viewTwo = &instr2->operand[2];
                        
                        if (instr->opcode != instr2->opcode) {
                            is_identical = false;
                            #if DEBUG
                                cout << "2 " << endl;
                            #endif
                            break;
                        }

                        if (!bh_is_constant(viewOne)) {
                            if (resultOneOffset != -1) {
                                if (bhir->instr_list[(*iter)+resultOneOffset].operand[0].base != viewOne->base){
                                    is_identical = false;
                                    #if DEBUG
                                        cout << "3 " << endl;
                                    #endif
                                    break;
                                }
                            }
                            else if (!bh_view_equally_shifted(&referenceView2, viewOne)) {
                                is_identical = false;
                                #if DEBUG
                                    cout << "4 " << endl;
                                #endif
                                break;
                            }
                        }

                        if (bh_operands(instr->opcode) == 3 && !bh_is_constant(viewTwo)) {
                            if (resultTwoOffset != -1) {
                                if (bhir->instr_list[(*iter)+resultTwoOffset].operand[0].base != viewTwo->base) {
                                    is_identical = false;
                                    #if DEBUG
                                        cout << "5 " << endl;
                                    #endif
                                    break;
                                }
                            }
                            else if (!bh_view_equally_shifted(&referenceView2, viewTwo)) {
                                is_identical = false;
                                #if DEBUG
                                    cout << "6 " << endl;
                                #endif
                                break;
                            }
                        }

                        if (has_constant(instr)
                            && (!has_constant(instr2)
                                || !constants_identical(&instr->constant, &instr2->constant))) {
                            #if DEBUG
                                cout << "constants not identical " << endl;
                            #endif
                            is_identical = false;
                            break;
                        }

                    }

                    if (is_identical)
                    {
                        unusedResults.insert( {{instr->operand[0].base, expand.size()}} );
                        expand.push_back(o);
                    }
                    else {
                        break;
                    }
                    
                }
                #if DEBUG
                    cout << "expand size: " << expand.size() << endl;
                #endif

                // Check if all results are used in expanded
                int lowestIndex = -1;
                for (std::unordered_map<bh_base*,int>::iterator iter = unusedResults.begin(); iter != unusedResults.end(); iter++) {
                    if (iter->second < lowestIndex || lowestIndex == -1) {
                        lowestIndex = iter->second;
                    }
                }
                expand.resize(max(0,lowestIndex+1));
                #if DEBUG
                    cout << "expand size again: " << expand.size() << endl;
                #endif

            }

            int baseSize = referenceView.base->nelem;

            std::unordered_map<int, bh_base*> resultBases;
            std::unordered_map<int, bh_view*> resultViews;

            instr = &bhir->instr_list[i];


            // Do optimization on identical list

            // add to base vector for later deallocation in bh_filter_preprocessor_execute
            
            // Reusing the restultBase
            // Create identity instruction for expanded matches
            for (std::vector<std::size_t>::iterator iter = identical.begin(); iter < identical.end(); iter++) {
                bh_view *tempReferenceView2 = instruction_get_view_of_interest(&bhir->instr_list[(*iter)]);
                referenceView2 = *tempReferenceView2;
                for (uint e = 0; e < expand.size(); ++e) {
                    if (resultBases.count(expand[e]) == 0) {
                        
                        bh_base* resultBase;
                        bh_create_base(bhir->instr_list[(*iter)+expand[e]].operand[0].base->type, baseSize, &resultBase);
                        resultBases.insert({{expand[e], resultBase}});
                        addBaseToGC(resultBase);
                    }
                    bhir->instr_list[(*iter)+expand[e]].opcode = (bh_opcode)BH_IDENTITY;
                    bhir->instr_list[(*iter)+expand[e]].operand[1] = build_similar_view(&referenceView2, resultBases[expand[e]]);

                }
                remove_unused_identity_backwards(bhir, min((int)(*iter)+expand.back()+3, (int)bhir->instr_list.size()-1), (*iter));
            }

            // Do operations on base
            std::unordered_map<bh_base*,int> unusedResults;
            for (uint e = 0; e < expand.size(); ++e) {
                instr = &bhir->instr_list[i+expand[e]];

                bh_view *viewOne = &instr->operand[1];
                bh_view *viewTwo = &instr->operand[2];

                unusedResults.insert({{instr->operand[0].base, expand[e]}});

                // backups:
                bh_view operand0;
                memcpy(&operand0, &instr->operand[0], sizeof(instr->operand[0]));

                instr->operand[0] = build_flat_view(resultBases[expand[e]], baseSize);
                if (!bh_is_constant(viewOne)) {
                    if (bh_view_equally_shifted(&referenceView, viewOne)) {
                        instr->operand[1] = build_flat_view(viewOne->base, baseSize);
                    }
                    else {
                        instr->operand[1] = build_flat_view(resultBases[unusedResults[viewOne->base]], baseSize);
                    }
                }
                if (bh_operands(instr->opcode) == 3 && !bh_is_constant(viewTwo)) {
                    if (bh_view_equally_shifted(&referenceView, viewTwo)) {
                        instr->operand[2] = build_flat_view(viewTwo->base, baseSize);
                    }
                    else {
                        instr->operand[2] = build_flat_view(resultBases[unusedResults[viewTwo->base]], baseSize);
                    }
                }
                
                
                bh_view *operand1 = new bh_view();
                build_similar_view2(&referenceView, resultBases[expand[e]], operand1);

                bh_view operand [3] = {operand0, *operand1};
                delete operand1;
                bh_instruction *copyResult = new bh_instruction();
                copyResult->opcode = (bh_opcode)BH_IDENTITY;
                memcpy(&copyResult->operand, &operand, sizeof(operand));
                

                if (true) {
                    // Inserting the first identity at last, is necessary to not mess up the iterators in identical vector.
                    bhir->instr_list.insert(bhir->instr_list.begin()+i+expand[e]+1, *copyResult);

                    ++i;
                }
                delete copyResult;

            }
            remove_unused_identity_backwards(bhir, min((int)i+expand.back()+3, (int)bhir->instr_list.size()-1), max(0, (int)(i-expand.size())));

            std::vector<bh_instruction> freeDiscardInstructions;
            for (auto& x: resultBases) {
                
                bh_instruction instr;
                instr.opcode = BH_FREE;
                instr.operand[0] = build_flat_view(x.second, x.second->nelem);
                freeDiscardInstructions.push_back(instr);

                bh_instruction instr2;
                instr2.opcode = BH_DISCARD;
                instr2.operand[0] = build_flat_view(x.second, x.second->nelem);
                freeDiscardInstructions.push_back(instr2);
                

            }
            int offset = min((int)i+(int)identical.back()+expand.back()+1, (int)bhir->instr_list.size()-1);
            bhir->instr_list.insert(bhir->instr_list.begin()+offset, freeDiscardInstructions.begin(), freeDiscardInstructions.end());

            i += expand.back();
            
        }
    }
}
