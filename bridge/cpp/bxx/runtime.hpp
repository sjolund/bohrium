/*
This file is part of Bohrium and copyright (c) 2012 the Bohrium team:
http://bohrium.bitbucket.org

Bohrium is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.

Bohrium is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the
GNU Lesser General Public License along with Bohrium.

If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __BOHRIUM_BRIDGE_CPP_RUNTIME
#define __BOHRIUM_BRIDGE_CPP_RUNTIME
#include <iostream>
#include <sstream>

namespace bxx {

// Runtime : Definition
inline Runtime& Runtime::instance()
{
    static Runtime instance;
    return instance;
}

inline Runtime::Runtime() : extension_count(BH_MAX_OPCODE_ID+1), queue_size(0)
{
    bh_error err;
    char err_msg[1000];

    if((err = bh_component_init(&bridge, NULL)) != BH_SUCCESS)
        exit(-1);
    if(bridge.nchildren != 1)
    {
        sprintf(err_msg, "Error in the runtime configuration: the bridge must "
                         "have exactly one child of type VEM or FILTER.\n");
        throw std::runtime_error(err_msg);
    }
    runtime = &bridge.children[0];
    if((err = runtime->init(runtime->name)) != BH_SUCCESS)
    {
        sprintf(err_msg, "Error in the initialization of the VEM.\n");
        throw std::runtime_error(err_msg);
    }
}

inline Runtime::~Runtime()
{
    if (ref_count.size()) {
        std::cout << "There are " << ref_count.size() << " dangling refs." << std::endl;
        for (std::map<bh_base*, size_t>::iterator it=ref_count.begin();
            it!=ref_count.end();
            ++it) {
            std::cout << it->first << " => " << it->second << '\n';
        }
    }
 
    flush();
    runtime->shutdown();
    bh_component_destroy(&bridge);
}

inline size_t Runtime::get_queue_size()
{
    return queue_size;
}

/**
 * De-allocate all bh_base associated with BH_DISCARD instruction in the queue.
 *
 * @param count The max number of instructions to look at.
 * @return The number of bh_base that got de-allocated.
 */
inline
size_t Runtime::deallocate_meta(size_t count)
{
    size_t deallocated = 0;
    if (count > BH_CPP_QUEUE_MAX) {
        throw std::runtime_error("Trying to de-allocate more than physically possible.");
    }
    while(!garbage.empty()) {
        delete garbage.front();
        ++deallocated;
        garbage.pop_front();
    }
    return deallocated;
}

/**
 * Sends the instruction-list to the bohrium runtime.
 *
 * NOTE: Assumes that the list is non-empty.
 */
inline
size_t Runtime::execute()
{
    size_t cur_size = queue_size;

    bh_ir bhir = bh_ir(queue_size, queue);
    bh_error status = runtime->execute(&bhir);   // Send instructions to Bohrium
    queue_size = 0;                                // Reset size of the queue

    if (status != BH_SUCCESS) {
        std::stringstream err_msg;
        err_msg << "Err: Runtime::execute() child->execute() failed: " << bh_error_text(status) << std::endl;

        throw std::runtime_error(err_msg.str());
    }

    deallocate_meta(cur_size);

    return cur_size;
}

/**
 * Flush the instruction-queue if it is about to get overflowed.
 *
 * This is used as a pre-condition to adding instructions to the queue.
 *
 * @return The number of instructions flushed.
 */
inline
size_t Runtime::guard()
{
    if (queue_size >= BH_CPP_QUEUE_MAX) {
        return execute();
    }
    return 0;
}

/**
 * Flush the instruction-queue.
 *
 * This will force the runtime system to execute the queued up instructions,
 * at least if there are any queued instructions.
 *
 * @return The number of instructions flushed.
 */
inline
size_t Runtime::flush()
{
    if (queue_size > 0) {
        return execute();
    }
    return 0;
}

/**
 * Create an unitialized intermediate operand.
 */
template <typename T>
inline
multi_array<T>& Runtime::temp()
{
    multi_array<T>* operand = new multi_array<T>();
    operand->setTemp(true);

    return *operand;
}

/**
 * Create an intermediate operand based on another operands meta-data.
 */
template <typename T, typename OtherT>
inline
multi_array<T>& Runtime::temp(multi_array<OtherT>& input)
{
    multi_array<T>* operand = new multi_array<T>(input);
    operand->setTemp(true);

    return *operand;
}

/**
 * Create an alias/segment/view of the supplied base operand.
 */
template <typename T>
inline
multi_array<T>& Runtime::view(multi_array<T>& base)
{
    multi_array<T>* operand = new multi_array<T>(base);

    operand->meta.base = base.meta.base;
    Runtime::instance().ref_count[operand->meta.base] += 1;

    return *operand;
}

/**
 * Create an intermediate alias/segment/view of the supplied base operand.
 */
template <typename T>
inline
multi_array<T>& Runtime::temp_view(multi_array<T>& base)
{
    multi_array<T>* operand = new multi_array<T>(base);
    
    operand->meta.base = base.meta.base;
    operand->setTemp(true);
    Runtime::instance().ref_count[operand->meta.base] += 1;

    return *operand;
}

template <typename Out, typename In1, typename In2>
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<Out>& op0, multi_array<In1>& op1, multi_array<In2>& op2)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = op0.meta;
    instr->operand[1] = op1.meta;
    instr->operand[2] = op2.meta;

    if (op1.getTemp()) { delete &op1; }
    if (op2.getTemp()) { delete &op2; }
}

template <typename Out, typename In1, typename In2>
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<Out>& op0, multi_array<In1>& op1, const In2 op2)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = op0.meta;
    instr->operand[1] = op1.meta;
    instr->operand[2].base = NULL;
    assign_const_type(&instr->constant, op2);

    if (op1.getTemp()) { delete &op1; }
}

template <typename Out, typename In1, typename In2>
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<Out>& op0, const In1 op1, multi_array<In2>& op2)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = op0.meta;
    instr->operand[1].base = NULL;
    instr->operand[2] = op2.meta;
    assign_const_type( &instr->constant, op1 );

    if (op2.getTemp()) { delete &op2; }
}

template <typename Out, typename In1>
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<Out>& op0, multi_array<In1>& op1)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = op0.meta;
    instr->operand[1] = op1.meta;
    instr->operand[2].base = NULL;

    if (op1.getTemp()) { delete &op1; }
}

template <typename Out, typename In1>
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<Out>& op0, const In1 op1)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = op0.meta;
    instr->operand[1].base = NULL;
    instr->operand[2].base = NULL;
    assign_const_type(&instr->constant, op1);
}

template <typename Out>
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<Out>& op0)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = op0.meta;
    instr->operand[1].base = NULL;
    instr->operand[2].base = NULL;
}

//
//  This function should only be used by random to encode the degenerate bh_r123 type.
//
template <typename T>
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<T>& op0, const uint64_t op1, const uint64_t op2)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = op0.meta;
    instr->operand[1].base = NULL;
    instr->operand[2].base = NULL;

    instr->constant.type = BH_R123;
    instr->constant.value.r123.start = op1;
    instr->constant.value.r123.key   = op2;
}

template <typename Ret, typename In1, typename In2>
inline
void Runtime::enqueue_extension(const std::string& name,
                                multi_array<Ret>& op0,
                                multi_array<In1>& op1,
                                multi_array<In2>& op2)
{
    bh_instruction* instr;
    bh_opcode opcode;

    // Look for the extension opcode
    std::map<std::string, bh_opcode>::iterator it = extensions.find(name);
    if (it != extensions.end()) {   // Got it
        opcode = it->second;
    } else {                        // Add it
        opcode = extension_count++;
        extensions.insert(std::pair<std::string, bh_opcode>(name, opcode));
        if(runtime->extmethod(name.c_str(), opcode) != BH_SUCCESS)
            throw std::runtime_error("Unknown extmethod");
    }

    guard();

    // Construct and enqueue the instructions
    instr = &queue[queue_size++];
    instr->opcode = extensions[name];
    instr->operand[0] = op0.meta;
    instr->operand[1] = op1.meta;
    instr->operand[2] = op2.meta;

    if (op1.getTemp()) { delete &op1; }
    if (op2.getTemp()) { delete &op2; }
}

template <typename T>
T scalar(multi_array<T>& op)
{
    Runtime::instance().enqueue((bh_opcode)BH_SYNC, op);
    Runtime::instance().flush();

    bh_base *op_a = op.getBase();
    T* data = (T*)(op_a->data);
    data += op.meta.start;

    T value = *data;

    if (op.getTemp()) { // If it was a temp you will never see it again
        delete &op;     // so you better clean it up!
        Runtime::instance().flush();
    }

    return value;
}

inline void Runtime::trash(bh_base *base)
{
    garbage.push_back(base);
}

}
#endif

