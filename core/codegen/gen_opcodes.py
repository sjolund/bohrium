#!/usr/bin/env python
import json 
import sys
import os
import re
import time
"""
    Generates the include/cphvb_opcode.h and core/cphvb_opcode 
    based on the definitnion in /core/codegen/opcodes.json.
"""

def gen_headerfile(opcode_dict):
    ret = """
    /*
     * Do not edit this file. It has been auto generate by 
     * ../core/codegen/gen_opcodes.py at %s. 
     */

    #ifndef __CPHVB_OPCODE_H
    #define __CPHVB_OPCODE_H

    #include "cphvb_type.h"

    #ifdef __cplusplus
    extern "C" {
    #endif

    /* Codes for known oparations */
    enum /* cphvb_opcode */
    {
    \n"""%time.strftime("%d/%m/%Y")
    for op in opcode_dict:
        ret += "        %s,\n"%op
        last_op = op
    ret = ret[:-2]#Remove the last comma.
    ret +=  """
    };

    #define CPHVB_NO_OPCODES %s

    #ifdef __cplusplus
    }
    #endif

    #endif
    """%last_op
    return ret

def gen_cfile(opcode_dict):
    ret = """
    /*
     * Do not edit this file. It has been auto generate by 
     * ../core/codegen/gen_opcodes.py at %s. 
     */

    #include <stdlib.h>
    #include <stdio.h>
    #include <cphvb_opcode.h>
    #include <cphvb.h>
    #include <stdbool.h>


    /* Number of operands for operation
     *
     * @opcode Opcode for operation
     * @return Number of operands
     */
    int cphvb_operands(cphvb_opcode opcode)
    {
        switch(opcode) 
        {
    \n"""%time.strftime("%d/%m/%Y")
    for op in opcode_dict:
        nop = opcode_dict[op]['nop']
        ret += "            case %s: return %s;\n"%(op,nop)
    ret +=   """            default: return -1;
        }
    }

    /* Number of operands in instruction
     * NB: this function handles user-defined function correctly
     * @inst Instruction
     * @return Number of operands
     */
    int cphvb_operands_in_instruction(cphvb_instruction *inst)
    {
        if (inst->opcode == CPHVB_USERFUNC)
            return inst->userfunc->nin + inst->userfunc->nout;
        else
            return cphvb_operands(inst->opcode);
    }


    /* Text descriptions for a given operation */
    const char* _opcode_text[CPHVB_NONE+1];
    bool _opcode_text_initialized = false;

    /* Text string for operation
     *
     * @opcode Opcode for operation
     * @return Text string.
     */
    const char* cphvb_opcode_text(cphvb_opcode opcode)
    {
        switch(opcode)
        {
        \n"""
    for op in opcode_dict:
        ret += "            case %s: return %s;\n"%(op,op)
    ret +=   """            default: return -1;
        }
    }
    """
    return ret

def comment_remover(text):
    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return ""
        else:
            return s
    pattern = re.compile(
        r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
        re.DOTALL | re.MULTILINE
    )
    return re.sub(pattern, replacer, text)

def main(script_dir):
    #Read the opcode definitions from opcodes.json. 
    f = open(os.path.join(script_dir,'opcodes.json')).read()
    #We have to manual ignore comments since comments is not 
    #part of the JSON standard.
    opcodes = json.loads(comment_remover(f))
    headerfile = gen_headerfile(opcodes)
    cfile = gen_cfile(opcodes)    
    h = open(os.path.join(script_dir,'..','..','include','cphvb_opcode.h'),"w")
    h.write(headerfile)
    h = open(os.path.join(script_dir,'..','cphvb_opcode.c'),"w")
    h.write(cfile)    

if __name__ == "__main__":
    try:
        script_dir = os.path.abspath(os.path.dirname(__file__))
    except NameError:
        print "The build script cannot run interactively."
        sys.exit(-1)
    main(script_dir)
