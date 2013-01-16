/*
This file is part of cphVB and copyright (c) 2012 the cphVB team:
http://cphvb.bitbucket.org

cphVB is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 
of the License, or (at your option) any later version.

cphVB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the 
GNU Lesser General Public License along with cphVB. 

If not, see <http://www.gnu.org/licenses/>.
*/

#include <cassert>
#include <StaticStore.hpp>
#include <cphvb.h>
#include <vector>
#include "task.h"
#include "exec.h"
#include "except.h"
#include "pgrid.h"
#include "comm.h"

static StaticStore<cphvb_array> tmp_ary_store(512);
static std::vector<task> task_store;

/* Returns a temporary array that will be freed on a batch flush
 * 
 * @return The new temporary array
 */
cphvb_array* batch_tmp_ary()
{
    return tmp_ary_store.c_next();
}


/* Schedule an task. 
 * NB: for now, we will flush in every task scheduling
 * @t  The task to schedule 
 */
void batch_schedule(const task& t)
{
    task_store.push_back(t);
    for(std::vector<task>::iterator it=task_store.begin(); 
        it != task_store.end(); ++it)
    {
        switch((*it).inst.type) 
        {
            case TASK_INST:
            {
                cphvb_error e;
                cphvb_instruction *inst = &((*it).inst.inst);
                if((e = exec_vem_execute(1, inst)) != CPHVB_SUCCESS)
                    EXCEPT_INST(inst->opcode, e, inst->status);
                break;
            }
            case TASK_SEND_RECV:
            {
                cphvb_error e;
                bool dir         = (*it).send_recv.direction;
                int rank         = (*it).send_recv.rank;
                cphvb_array *ary = (*it).send_recv.local_ary;
                cphvb_intp s     = cphvb_type_size(ary->type);
                cphvb_intp nelem = cphvb_nelements(ary->ndim, ary->shape);
                
                if(dir)//Sending
                {
                    char *data = (char*) cphvb_base_array(ary)->data;
                    assert(data != NULL);
                    data += ary->start * s;
                    if((e = MPI_Send(data, nelem * s, MPI_BYTE, rank, 0, 
                                     MPI_COMM_WORLD)) != MPI_SUCCESS)
                    EXCEPT_MPI(e);
                }
                else//Receiving
                {
                    if((e = cphvb_data_malloc(ary)) != CPHVB_SUCCESS)
                        EXCEPT_OUT_OF_MEMORY();
                    char *data = (char*) cphvb_base_array(ary)->data;
                    data += ary->start * s;
                    if((e = MPI_Recv(data, nelem * s, MPI_BYTE, rank, 
                                     0, MPI_COMM_WORLD, 
                                     MPI_STATUS_IGNORE)) != MPI_SUCCESS)
                        EXCEPT_MPI(e);
                }
                break;
            }
            case TASK_GATHER_SCATTER:
            {
                bool dir         = (*it).gather_scatter.direction;
                cphvb_array *ary = (*it).gather_scatter.global_ary;
                comm_gather_scatter(dir, ary);
                break;
            }
            default:
            {
                fprintf(stderr, "[VEM-CLUSTER] batch_schedule encountered "
                        "an unknown task type\n");
                MPI_Abort(MPI_COMM_WORLD,CPHVB_ERROR);
            }
        }        
    }
    task_store.clear();
}


/* Schedule an instruction
 * @inst   The instruction to schedule 
 */
void batch_schedule(const cphvb_instruction& inst)
{
    task t;
    t.inst.type = TASK_INST;
    t.inst.inst = inst;
    batch_schedule(t); 
}


/* Schedule an instruction that only takes one instruction.
 *
 * @opcode   The opcode of the instruction
 * @operand  The local operand in the instruction
 */
void batch_schedule(cphvb_opcode opcode, cphvb_array *operand)
{
    task t;
    t.inst.type = TASK_INST;
    t.inst.inst.opcode = opcode;
    t.inst.inst.status = CPHVB_INST_PENDING;
    t.inst.inst.operand[0] = operand;
    assert(cphvb_operands_in_instruction(&t.inst.inst) == 1);
    batch_schedule(t); 
}


/* Schedule an instruction.
 *
 * @opcode   The opcode of the instruction
 * @operands The local operands in the instruction
 * @ufunc    The user-defined function struct when opcode is CPHVB_USERFUNC.
 */
void batch_schedule(cphvb_opcode opcode, cphvb_array *operands[],
                    cphvb_userfunc *ufunc)
{
    task t;
    t.inst.type = TASK_INST;
    t.inst.inst.opcode = opcode;
    t.inst.inst.status = CPHVB_INST_PENDING;
    t.inst.inst.userfunc = ufunc;
    if(ufunc == NULL)
    {
        assert(opcode != CPHVB_USERFUNC);
        memcpy(t.inst.inst.operand, operands, cphvb_operands(opcode) 
                                              * sizeof(cphvb_array*));
    }
    batch_schedule(t); 
}


/* Schedule an send/receive instruction.
 *
 * @direction  If True the array is send else it is received.
 * @rank       The to send to or receive from
 * @local_ary  The local array to communicate
 */
void batch_schedule(bool direction, int rank, cphvb_array *local_ary)
{
    task t;
    t.send_recv.type      = TASK_SEND_RECV;
    t.send_recv.direction = direction;
    t.send_recv.rank      = rank;
    t.send_recv.local_ary = local_ary;
    batch_schedule(t); 
}


/* Flush all scheduled instructions
 * 
 */
void batch_flush()
{
//    task_store.clear();
    tmp_ary_store.clear(); 
}



