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

#include <bh.h>
#include <bh_vector.h>
#include <bh_flow.hpp>
#include <assert.h>
#include <algorithm>
#include <stdexcept>

namespace bh
{
    Flow::Flow(bh_intp ninstr, const bh_instruction *instr_list) 
        : ninstr(ninstr)
        , bh_instructions(instr_list)
        , instructions((Instruction*)malloc(sizeof(Instruction)*ninstr))
    {
        for(bh_intp i=0; i<ninstr; i++)
        {
            const bh_instruction *bh_instr = &instr_list[i];
            Instruction& instr = instructions[i];
            instr = Instruction(i,bh_instr);

            std::set<Instruction&> deps;
            for(bh_intp o = 0; o < bh_operands_in_instruction(bh_instr); ++o)
            {
                const bh_view *op = &(bh_instr->operand[o]);
                if(bh_is_constant(op))
                    continue;
                auto bi = bases.find(op->base);
                Access access(instr,o);
                if (bi == bases.end())
                {
                    bases[op->base] = std::vector<Access>(1,access);
                } else {
                    bi.second.push_back(access);
                    deps.insert(depends(access).instr);
                }
            }
            auto lastDep = deps.crbegin();
            if (deps.size > 0 && lastDep->canInsert(instr))
            {
                Subdsg& sd = *lastdep;
                instr.subDag = sd.id;
                sd.instructions.insert(instr);
                for(auto d = lastDep+1; d != deps.crend(); ++d )
                    sd.deps.insert(*d);
            } else {
                instr.subDag = instr.id;
                subDags[instr.subDag] = SubDAG(instr,deps);
            }
        }
    }

    const Flow::Access& Flow::depends(const Access& access) const
    {
        //Search through all nodes with the same base as 'node'
        auto sbit = bases.find(access.base());
        assert (sbit != bases.end());
        const std::vector<Access>& sameBase = sbit->second;
        //Now continue iterating through possible conflicts
        for(const Access& ac: sameBase)
        {
            if(!access.write() && !ac.write())
                continue; //No possible conflict when both is read only
            if(access.instr == ac.instr)
                continue; //No possible conflict within the same instruction
            if(!bh_view_disjoint(access.view(), ac.view()))
                return ac;
        }
    }
    
    bool Flow::SubDAG::canInsert(const Instruction& instr) const;
    {
        if (instr.opcode() == BH_FREE || instr.opcode() == BH_DISCARD)
            return true;
        for (const Instruction& i: instructions)
        {
            if (i.opcode() == BH_FREE || i.opcode() == BH_DISCARD)
                continue;
            for(bh_intp o = 1; o < i.operands(); ++o)
            {
                if (!(bh_view_disjoint(insrt.view(0), i.view(o)) || 
                      bh_view_aligned(instr.view(0), i.view(o))))
                    return false;
            }
            if (!(bh_view_disjoint(instr.view(0), i.view(0)) || 
                  bh_view_aligned(instr.view(0), i.view(0))))
                return false;
            for(bh_intp o = 1; o < instr.operands(); ++o)
            {
                if (!(bh_view_disjoint(instr.view(o), i.view(0)) || 
                      bh_view_aligned(instr.view(o), i.view(0))))
                    return false;
            }
        }
        return true;
    }

    //Private class for sorting the 'dag_deps' and 'dag_deps'
    struct topological_sort
    {
        const std::vector<std::set<bh_intp> > &dag_deps;
        std::vector<bh_intp> map;
        bh_intp count;
        topological_sort(const std::vector<std::set<bh_intp> > &deps)
            : dag_deps(deps)
            , map(dag_deps.size(),-1)
            , count(0)
        {};
        
        void visit(bh_intp i)
        {
            const std::set<bh_intp> &deps = dag_deps[i];
            if(map[i] == -2)
                throw std::runtime_error("The root DAG, where each node is a sub-dag, contains cycles!");
            if(map[i] != -1)
                return;//Visited already
            
            map[i] = -2; //temporary mark for cycle detection
            
            for(auto d: deps)
                visit(d);
            map[i] = count++;
        }
        std::vector<bh_intp> get_map(void)
        {
            for(size_t i=0; i<dag_deps.size(); ++i)
            {
                if(map[i] == -1)
                    visit(i);
            }
            return map;
        }
    };

    
    //Fill the uninitialized 'bhir' based on the flow object
    void Flow::bhir_fill(bh_ir *bhir)
    {
        assert(bhir->dag_list == NULL);
        
        //A set of dependencies for each instruction index in the flow
        std::vector<std::set<bh_intp> > instr_deps(ninstr);
        //Map between flow and bhir sub-DAG indices
        std::map<bh_intp, bh_intp> dag_f2b;
        //Number of sub-DAGs
        size_t ndags = 0;

        //Initiate the sub-DAGs
        for (bh_intp i = 0; i < ninstructions; ++i)
        {
            const Instruction instr = instructions[i];
            assert(instr.subDAG != -1);
            if(dag_f2b.count(instr.subDAG) == 0)
                dag_f2b[instr.subDAG] = ndags++;
        }
        //A set of dependencies for each sub-DAG in the bhir
        std::vector<std::set<bh_intp> > dag_deps(ndags);

        //A set of instructions for each sub-DAG in the flow
        std::vector<std::set<bh_intp> > dag_nodes(ndags); // flow_instr* -> bh_intp

        //Compute dependencies both between nodes and sub-DAGs
        for (bh_intp i = 0; i < ninstructions; ++i)
        {
            const Instruction instr = instructions[i];
            for(bh_intp iid: conflicts(instr))
            {
                const Instruction& ci = instructions[iid];
                if(instr.subDAG == ci.subDAG)//The dependency is within a sub-DAG
                {
                    if(instr.index != ci.index)//We cannot conflict with ourself
                        instr_deps[instr.index].insert(iid);
                }
                else//The dependency is to another sub-DAG
                {
                    dag_deps[dag_f2b[instr.subDAG]].insert(dag_f2b[ci.subDAG]);
                }
            }
            dag_nodes[dag_f2b[instr.subDAG]].insert(instr.index);
        }

        //Allocate the DAG list
        bhir->ndag = ndags+1;//which includes the root DAG
        bhir->dag_list = (bh_dag*) bh_vector_create(sizeof(bh_dag), bhir->ndag, bhir->ndag);
        if(bhir->dag_list == NULL)
            throw std::bad_alloc();

        Lets sort the sub-dags topologically
        {
            topological_sort top = topological_sort(dag_deps);
            std::vector<bh_intp> sorted = top.get_map();
            const std::vector<std::set<bh_intp> > t_deps(dag_deps);
            const std::vector<std::set<bh_intp> > t_nodes(dag_nodes); // flow_instr* -> bh_intp
            for(size_t i=0; i<sorted.size(); ++i)
            {
                dag_deps[i] = t_deps[sorted[i]];
                dag_nodes[i] = t_nodes[sorted[i]];
            }
        }

        //Create the root DAG where all nodes are sub-DAGs
        {
            bh_dag *dag = &bhir->dag_list[0];
            dag->node_map = (bh_intp*) bh_vector_create(sizeof(bh_intp), ndags, ndags);
            if(dag->node_map == NULL)
                throw std::bad_alloc();
            for(size_t i=0; i<ndags; ++i)
                dag->node_map[i] = (-1*(i+1)-1);
            dag->nnode = ndags;
            dag->tag = 0;
            dag->adjmat = bh_adjmat_create(ndags);
            if(dag->adjmat == NULL)
                throw std::bad_alloc();

            //Fill each row in the adjacency matrix with the dependencies between sub-DAGs
            for(size_t i=0; i < ndags; i++)
            {
                const std::set<bh_intp> &deps = dag_deps[i];
                if(deps.size() > 0)
                {
                    std::vector<bh_intp> sorted_vector(deps.begin(), deps.end());
                    bh_error e = bh_adjmat_fill_empty_col(dag->adjmat, i,
                                                          deps.size(),
                                                          &sorted_vector[0]);
                    if(e != BH_SUCCESS)
                        throw std::bad_alloc();
                }
            }
            if(bh_adjmat_finalize(dag->adjmat) != BH_SUCCESS)
                throw std::bad_alloc();
        }
        //Create all sub-DAGs
        for(size_t dag_idx=0; dag_idx<dag_nodes.size(); ++dag_idx)
        {
            const std::set<bh_intp> &nodes = dag_nodes[dag_idx];
            bh_dag *dag = &bhir->dag_list[dag_idx+1];
            dag->node_map = (bh_intp*) bh_vector_create(sizeof(bh_intp), nodes.size(), nodes.size());
            if(dag->node_map == NULL)
                throw std::bad_alloc();
            dag->nnode = nodes.size();
            dag->tag = 0;
            dag->adjmat = bh_adjmat_create(nodes.size());
            if(dag->adjmat == NULL)
                throw std::bad_alloc();

            //Fill the adjmat sequentially starting at row zero
            std::map<bh_intp,bh_intp> instr2row;//instruction index to row in the adjmat
            size_t row = 0;
            for (bh_intp iid: nodes)
            {
                const std::set<bh_intp> &deps = instr_deps[iid]; // flow_instr* -> bh_intp 
                
                //Note that the order of 'row' is ascending thus the topological order is preserved.
                dag->node_map[row] = iid;
                //Mapping from flow index to node index within the sub-DAG.
                instr2row[iid] = row;

                if(deps.size() > 0)
                {
                    //Convert flow indices to indices in the local sub-DAG
                    std::vector<bh_intp> sorted_vector;
                    for (bh_intp diid: deps)
                    {
                        sorted_vector.push_back(instr2row[diid]);
                    }
                    bh_error e = bh_adjmat_fill_empty_col(dag->adjmat, row,
                                                          sorted_vector.size(),
                                                          &sorted_vector[0]);
                    if(e != BH_SUCCESS)
                        throw std::bad_alloc();
                }
                ++row;
            }
            if(bh_adjmat_finalize(dag->adjmat) != BH_SUCCESS)
                throw std::bad_alloc();
        }
    }
}
