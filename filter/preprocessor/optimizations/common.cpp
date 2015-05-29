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
#include <unordered_map>


using namespace std;

#define DEBUG (false)
#define DEBUG2 (false)
#define DBPOINTS (false)

void remove_unused_identity_backwards(bh_ir *bhir, int start, int stop) {
    bh_instruction *instr;
    std::unordered_map<bh_base*, int> freed;
    //cout << "start: "<<start<<", stop: "<<stop<< ", freed size: "<<freed.size()<<endl;
    for (; start >= stop; start--) {
        instr = &bhir->instr_list[start];
        if (instr->opcode == (bh_opcode)BH_FREE) {
            // base is added to the list of free'd bases
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
        #if DBPOINTS
            printf("\nfrom beginning \n");
            cout << "i: " << i << endl;
        #endif
        instr = &bhir->instr_list[i];

        #if DBPOINTS
            bh_pprint_instr_list(instr, 1, "\ninstr");
            printf("\n0.3 \n");
        #endif

        if (bh_operands(instr->opcode) < 2 || operation_datamanagement(instr->opcode)) {
            continue;
        }

        #if DBPOINTS
            printf("\n0.5 \n");
        #endif

        // If the result array has same base as one of the operands, the operand i changed and cannot be used for common shifted array operation
        if (bh_view_same_base(&instr->operand[0], &instr->operand[1])
            || (bh_operands(instr->opcode) == 3 && bh_view_same_base(&instr->operand[0], &instr->operand[2]))) {
            continue;
        }

        #if DBPOINTS
            printf("\n0.8 \n");
        #endif

        identical.clear();
        expand = {0};
        #if DBPOINTS
            //printf("\n operation is commutative: %s", operation_commutative(instr->opcode) ? "true" : "false");
            printf("\n1 \n");
        #endif

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

        #if DBPOINTS
            printf("\n1.5 \n");
        #endif

        #if DEBUG2
            printf("\n\nidentical list length: %d\n\n\n\n", identical.size());
        #endif

        if (identical.size() > 0) {
            #if DEBUG
                printf("\nMATCHING found\n");
            #endif
            #if DBPOINTS
                printf("\n1.6 \n");
            #endif
            // Expand indenticals (offsets)
            // Make hard copy, since it can be overwritten at a later point.
            bh_view *tempReferenceView = instruction_get_view_of_interest(&bhir->instr_list[i]);
            #if DBPOINTS
                printf("\n1.7 \n");
            #endif
            //memcpy(referenceView, tempReferenceView, sizeof(*tempReferenceView));
            
            #if DBPOINTS
                printf("\n1.9 \n");
            #endif

            
            if (tempReferenceView != NULL) {
                referenceView = *tempReferenceView;
                #if DEBUG2
                    printf("\nreferenceView \n");
                    helper_pprint_view(&referenceView);
                #endif
                std::unordered_map<bh_base*,int> unusedResults = {{bhir->instr_list[i].operand[0].base,0}};

                #if DBPOINTS
                    printf("\n2 \n");
                #endif
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
                            #if DBPOINTS
                                printf("\nbefore resultOneOffset \n");
                            #endif
                            resultOneOffset = expand[unusedResults[viewOne->base]];
                            #if DBPOINTS
                                printf("\nafter resultOneOffset \n");
                            #endif
                            unusedResults.erase(viewOne->base);
                        }
                        else if (!bh_view_equally_shifted(&referenceView, viewOne)) {
                            continue;
                        }
                    }
                    if (bh_operands(instr->opcode) == 3 && !bh_is_constant(viewTwo)) {
                        if (unusedResults.count(viewTwo->base) == 1) {
                            #if DBPOINTS
                                printf("\nbefore resultTwoOffset \n");
                            #endif
                            resultTwoOffset = expand[unusedResults[viewTwo->base]];
                            #if DBPOINTS
                                printf("\nafter resultTwoOffset \n");
                            #endif
                            unusedResults.erase(viewTwo->base);
                        }
                        else if (!bh_view_equally_shifted(&referenceView, viewTwo)) {
                            continue;
                        }
                    }




                    // instr is not violating the common shift invaraint

                    #if DBPOINTS
                        printf("\n2.5 \n");
                    #endif

                    // Check if expansion is valid for all identicals
                    bool is_identical = true;
                    for (std::vector<std::size_t>::iterator iter = identical.begin(); iter < identical.end(); iter++) {
                        if ((*iter)+o >= bhir->instr_list.size()) {
                            is_identical = false;
                            break;
                        }
                        instr2 = &bhir->instr_list[(*iter)+o];

                        referenceView2 = *instruction_get_view_of_interest(instr2);
                        viewOne = &instr2->operand[1];
                        viewTwo = &instr2->operand[2];
                        
                        if (instr->opcode != instr2->opcode) {
                            is_identical = false;
                            break;
                        }

                        if (!bh_is_constant(viewOne)) {
                            if (resultOneOffset != -1) {
                                if (bhir->instr_list[(*iter)+resultOneOffset].operand[0].base != viewOne->base){
                                    is_identical = false;
                                    break;
                                }
                            }
                            else if (!bh_view_equally_shifted(&referenceView2, viewOne)) {
                                is_identical = false;
                                break;
                            }
                        }

                        if (bh_operands(instr->opcode) == 3 && !bh_is_constant(viewTwo)) {
                            if (resultTwoOffset != -1) {
                                if (bhir->instr_list[(*iter)+resultTwoOffset].operand[0].base != viewTwo->base) {
                                    is_identical = false;
                                    break;
                                }
                            }
                            else if (!bh_view_equally_shifted(&referenceView2, viewTwo)) {
                                is_identical = false;
                                break;
                            }
                        }

                        if (has_constant(instr)
                            && (!has_constant(instr2)
                                || !constants_identical(&instr->constant, &instr2->constant))) {
                            is_identical = false;
                            break;
                        }
                        #if DBPOINTS
                            printf("\n3 \n");
                        #endif

                    }
                    #if DBPOINTS
                        printf("\n4 \n");
                    #endif

                    if (is_identical)
                    {
                        unusedResults.insert( {{instr->operand[0].base, expand.size()}} );
                        expand.push_back(o);
                    }
                    else {
                        break;
                    }
                    
                }

                #if DBPOINTS
                    printf("\n5 \n");
                #endif

                // Check if all results are used in expanded
                int lowestIndex = -1;
                for (std::unordered_map<bh_base*,int>::iterator iter = unusedResults.begin(); iter != unusedResults.end(); iter++) {
                    if (iter->second < lowestIndex || lowestIndex == -1) {
                        lowestIndex = iter->second;
                    }
                }
                expand.resize(max(0,lowestIndex+1));
                

            }

            #if DBPOINTS
                printf("\n6 \n");
            #endif
            #if DEBUG
                printf("\nexpansion length: %d \n", (int)expand.size());
                for( std::vector<int>::const_iterator i = expand.begin(); i != expand.end(); ++i) {
                    std::cout << *i << ' ';
                }
            #endif

            

            int baseSize = referenceView.base->nelem;

            std::unordered_map<int, bh_base*> resultBases;
            std::unordered_map<int, bh_view*> resultViews;

            instr = &bhir->instr_list[i];





            // FOR DEBUGGING PURPOSE ONLY!!!!
            //expand.resize(1);






            // Do optimization on identical list
            

            #if DEBUG
                printf("\nCOMMON optimization applied!\n");
            #endif


            // add to base vector for later deallocation in bh_filter_preprocessor_execute
            
            // Reusing the restultBase
            // Create identity instruction for expanded matches
            for (std::vector<std::size_t>::iterator iter = identical.begin(); iter < identical.end(); iter++) {
                bh_view *tempReferenceView2 = instruction_get_view_of_interest(&bhir->instr_list[(*iter)]);
                //memcpy(referenceView2, tempReferenceView2, sizeof(*tempReferenceView2));
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

                #if DBPOINTS
                    printf("\ne0 \n");
                #endif


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
                        //bh_view flattenedOperand2 = { instr->operand[2].base, 1, 0, {instr->operand[2].base->nelem}, {1}};
                        instr->operand[2] = build_flat_view(viewTwo->base, baseSize);
                    }
                    else {
                        instr->operand[2] = build_flat_view(resultBases[unusedResults[viewTwo->base]], baseSize);
                    }
                }
                #if DBPOINTS
                    printf("\ne1 \n");
                #endif
                
                
                bh_view *operand1 = new bh_view();
                build_similar_view2(&referenceView, resultBases[expand[e]], operand1);

                bh_view operand [3] = {operand0, *operand1};
                delete operand1;
                //bh_instruction copyResult = { (bh_opcode)BH_IDENTITY };
                bh_instruction *copyResult = new bh_instruction();
                copyResult->opcode = (bh_opcode)BH_IDENTITY;
                memcpy(&copyResult->operand, &operand, sizeof(operand));
                

                if (true) {
                    // Inserting the first identity at last, is necessary to not mess up the iterators in identical vector.
                    bhir->instr_list.insert(bhir->instr_list.begin()+i+expand[e]+1, *copyResult);

                    ++i;
                }
                delete copyResult;

                

                #if DBPOINTS
                    printf("\ne2 \n");
                #endif

            }
            remove_unused_identity_backwards(bhir, min((int)i+expand.back()+3, (int)bhir->instr_list.size()-1), max(0, (int)(i-expand.size())));

            std::vector<bh_instruction> freeDiscardInstructions;
            for (auto& x: resultBases) {

                /*bh_view view;

                view.ndim = 1;
                view.start = 0;
                view.shape[BH_MAXDIM];
                view.stride[BH_MAXDIM];

                view.base = x.second;*/
                
                bh_instruction instr;
                instr.opcode = BH_FREE;
                //instr.operand[0].base = x.second;
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
    
    #if DBPOINTS
        printf("\n9 \n");
    #endif   
 

}
