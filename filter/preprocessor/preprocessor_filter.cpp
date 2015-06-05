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
#include <preprocessor_filter.h>


using namespace std;

#define DEBUG (false)
#define TIMER (false)

void preprocessor_filter::preprocess(bh_ir *bhir)
{
    #if DEBUG
        helper_pprint_instr_list(bhir);
    #endif

    std::vector<Optimization*> optimizations;
    optimizations.push_back(new Optimization(COMMON, &bases_));
    //optimizations.push_back(new Optimization(POWER, &bases_));
    optimizations.push_back(new Optimization(POWER_CACHE, &bases_));

    std::vector<Optimization*>::iterator it;

    #if TIMER
        auto begin = chrono::high_resolution_clock::now();
    #endif

    for(it = optimizations.begin(); it < optimizations.end(); it++){
        (*it)->optimize(bhir);
    }

    #if TIMER
        auto end = chrono::high_resolution_clock::now();    
        auto dur = end - begin;
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
        cout << "Apply filters duration in microseconds: " << ms << endl;
    #endif
    #if DEBUG
        printf("\nAfter running optimizations\n");
        helper_pprint_instr_list(bhir);
    #endif
    
}

void preprocessor_filter::gc() {
    for(size_t size = bases_.size(); size > 0; --size) {
        //delete bases_.back();

        free(bases_.back());
        bases_.pop_back();
    }
}


