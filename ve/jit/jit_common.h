// jit_common.h

#ifndef __JIT_COMMON_H
#define __JIT_COMMON_H

#include <stdio.h>
#include <string.h>
#include <cphvb.h>
#include <cphvb_pprint.h>
#include "jit_analyser.h"
#include <string>

void cphvb_pprint_instr_list_small( cphvb_instruction* instruction_list, cphvb_intp instruction_count, const char* txt );
std::string jit_pprint_nametable(jit_name_table* nt);
#endif
