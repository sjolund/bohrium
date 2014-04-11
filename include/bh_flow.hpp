/*
This file is part of Bohrium and copyright (c) 2012 the Bohrium
team <http://www.bh107.org>.

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

#ifndef __BH_FLOW_HPP
#define __BH_FLOW_HPP

#include <bh.h>
#include <vector>
#include <memory>
#include <map>
#include <set>

namespace bh
{
    class Flow
    {
    private:
        struct Instruction
        {
            bh_intp id;
            const bh_instruction* instr;
            //The sub-DAG index this instruction is part of (-1 means none)
            bh_intp subDAG;
            Instruction(bh_intp index, const bh_instruction* instr) : 
                index(index), instr(instr), timestep(0), subDAG(-1) {}
            bh_intp opcode() const { return instr->opcode; }
            bh_intp operands() const { return bh_operands(instr->opcode); }
            const bh_view* view(bh_intp o) const {return &instr->operands[o]; }
            bool operator==(const Instruction& rhs) const
            { return (index == rhs.index); }
            bool operator!=(const Instruction& rhs) const
            { return !(*this == rhs); }
            bool operator<(const Instruction& rhs) const
            { return (index < rhs.index); }
        };
        struct Access 
        { 
            const Instruction& instr;
            bh_intp op;    // index op the operand
            Access(const Instruction& instr, bh_intp op) : instr(instr), op(op) {}
            bool operator==(const Node& rhs) const
            { return (instr == rhs.instr && op == rhs.op); }
            bool operator!=(const Node& rhs) const
            { return !(*this == rhs); }
            bool write() const { return (op == 0); }
            const bh_view* view() const
            { return &(instr.instr->operand[op]); }
            const bh_base* base() const
            { return view()->base; }
        };
        struct SubDAG
        {
            bh_intp id;
            std::set<Instruction&> instructions;
            std::set<Instruction&> deps;
            bool canInsert(const Instruction& instr) const;
            SubDAG(Instruction& instr, const std::set<Instruction&>& deps)
                : id(instr.subDAG)
                , deps(deps) 
            { instructions.insert(instr); }
        };
        // Number of Instructions
        bh_intp ninstr;
        // Original instruction list
        const bh_instruction* bh_instructions; 
        // Augmented instruction list
        std::unique_ptr<Instruction[]> instructions;
        // List of all Nodes accessing the same bh_base ordered by timestep
        std::map<const bh_base*, std::vector<Access> > bases;
        // List of instruction ids in each timestep
        std::map<bh_intp, SubDAG > subDAGs;

        // Try to merge the two subDAGs idenfied by (id1,id2) 
        // If merged reutrn true, subDAGs[id2] will no longer exist.
        bool merge(bh_intp id1, bh_intp id2);
        
    public:
        Flow(bh_intp ninstr, const bh_instruction* instr_list);
        void bhir_fill(bh_ir *bhir);
    };
}
#endif
