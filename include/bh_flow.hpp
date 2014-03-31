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
#include <map>
#include <set>

#define MAX_NO_OPS 3 // Max number of operands in an instruction

namespace bh
{
    class Flow
    {
    private:
        struct Node 
        { 
            bh_intp instr; // index of the instruction 
            bh_intp op;    // index op the operand
            Node(bh_intp instr, bh_intp op) : instr(instr), op(op) {}
            bool operator==(const Node& rhs) const
            { return (instr == rhs.instr && op == rhs.op); }
            bool operator!=(const Node& rhs) const
            { return !(*this == rhs); }
            bool operator<(const Node& rhs) const
            { return (instr*MAX_NO_OPS+op < rhs.instr*MAX_NO_OPS+op); }
            bool write() const { return (op == 0); }
        };
        struct Instruction
        {
            bh_intp index;
            const bh_instruction* instr;
            bh_intp timestep;
            //The sub-DAG index this instruction is part of (-1 means none)
            bh_intp subDAG;
            bh_intp baseIndices[MAX_NO_OPS]; // 
            Instruction(bh_intp index, const bh_instruction* instr) : 
                index(index), instr(instr), timestep(0), subDAG(-1) {}
        };
        // Original instruction list
        const bh_instruction* bh_instructions; 
        // Augmented instruction list
        std::vector<Instruction> instructions;
        // List of base arrays to maintain chronological order.
        std::vector<const bh_base*> baseList;
        // List of all Nodes accessing the same bh_base
        std::map<const bh_base*, std::vector<Node> > bases;
        // List of instruction ids in each timestep
        std::vector< std::vector< bh_intp > > timesteps;
        // Main subDAG container: mapping subDAG id's to a list of instruction id's
        std::map<bh_intp, std::vector<bh_intp> > subDAGs;

        // Get the bh_base accessed by the Node
        const bh_base* base(const Node node) const;
        // Get the bh_view accessed by the Node
        const bh_view* view(const Node node) const;
        // Get the Instruction the Node belongs to
        const Instruction& instruction(const Node node);
        // Get the list of Nodes with conflicting access
        std::vector<Node> conflicts(const Node& node) const;
        // Set of conflicting Instruction id's
        std::set<bh_intp> conflicts(const Instruction& instr) const; 

        // Get the subDAG the Instruction with iid belongs to. 
        // Creating a subDAG if needed.
        bh_intp subDAG(bh_intp iid);

        // Try to merge the two subDAGs idenfied by (id1,id2) 
        // If merged reutrn true, subDAGs[id2] will no longer exist.
        bool merge(bh_intp id1, bh_intp id2);
        
        //Cluster the flow object into sub-DAGs suitable for kernels
        void clustering();
    public:
        Flow(bh_intp ninstr, const bh_instruction* instr_list);
        void html(const char* filename);
        void bhir_fill(bh_ir *bhir);
    };
}
#endif
