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

// Only used by html function
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <set>

namespace bh
{
    Flow::Flow(bh_intp ninstr, const bh_instruction *instr_list) :
        bh_instructions(instr_list)
    {
        for(bh_intp i=0; i<ninstr; i++)
        {
            const bh_instruction *bh_instr = &instr_list[i];
            instructions.push_back(Instruction(i,bh_instr));
            Instruction& instr = instructions.back();

            for(bh_intp o = 0; o < bh_operands_in_instruction(bh_instr); ++o)
            {
                const bh_view *op = &(bh_instr->operand[o]);
                if(bh_is_constant(op))
                    continue;
                // Maintain the chronological list of bases
                if (bases.find(op->base) == bases.end())
                    baseList.push_back(op->base);
                // Add access information
                bases[op->base].push_back(Node(i,o));

            }
            //The timestep of the instruction must be greater than any conflicting instructions
            for(bh_intp instrId: conflicts(instr))
            {
                if(instr.timestep <= instructions[instrId].timestep)
                    instr.timestep = instructions[instrId].timestep+1;
            }

            //Add the timestep of the instruction to 'timesteps'
            if(instr.timestep >= (bh_intp)timesteps.size())
            {
                assert(instr.timestep == (bh_intp)timesteps.size());
                timesteps.push_back(std::vector<bh_intp>(1,i));
            } else {
                timesteps[instr.timestep].push_back(i);
            }
        }
        for (auto &base: bases)
        {
            std::sort(base.second.begin(), base.second.end());
        }
        clustering();
    }
    
    void Flow::clustering()
    {
        for (const bh_base* base: baseList)
        {
            std::vector<Node> nodes = bases[base];
            auto fni = nodes.begin();
            bh_intp sdid1 = subDAG(fni->instr);
            for (++fni; fni != nodes.end(); ++fni)
            {
                bh_intp sdid2 = subDAG(fni->instr);
                merge(sdid1, sdid2);
                sdid1 = subDAG(fni->instr);
            }
        }
    }

    inline const bh_view* Flow::view(const Node node) const
    { return &bh_instructions[node.instr].operand[node.op]; }

    inline const bh_base* Flow::base(const Node node) const
    { return view(node)->base; }

    inline const Flow::Instruction& Flow::instruction(const Node node)
    { return instructions[node.instr]; }

    std::vector<Flow::Node> Flow::conflicts(const Node& node) const
    {
        //Search through all nodes with the same base as 'node'
        auto sbit = bases.find(base(node));
        assert (sbit != bases.end());
        const std::vector<Node>& sameBase = sbit->second;

        //Iterate 'it' to where the 'node' is in 'same_base'
        auto it = sameBase.rbegin();
        while (it != sameBase.rend() && *it != node)
            ++it;
        assert(it != sameBase.rend()); //'node' must be found in 'same_base'

        std::vector<Node> res;
        //Now continue iterating through possible conflicts
        for(++it; it != sameBase.rend(); ++it)
        {
            assert(node != *it);//We should not be here multiple times

            if(!node.write() && !it->write())
                continue;//No possible conflict when both is read only
            
            if(node.instr == it->instr)
                continue;//No possible conflict within the same instruction
            
            if(!bh_view_disjoint(view(node), view(*it)))
            {
                res.push_back(*it);
            }
        }
        return res;
    }

    std::set<bh_intp> Flow::conflicts(const Instruction& instr) const
    {
        std::set<bh_intp> res;
        for(bh_intp o = 0; o < bh_operands_in_instruction(instr.instr); ++o)
        {
            if(bh_is_constant(&(instr.instr->operand[o])))
                continue;
            for(const Node n: conflicts(Node(instr.index,o)))
            {
                res.insert(n.instr);
            }
        }
        return res;
    }

    bh_intp Flow::subDAG(bh_intp iid)
    {
        Instruction& instr = instructions[iid];
        if (instr.subDAG == -1)
        {
            instr.subDAG = instr.index;
            // assert that we are actually adding a subDAG
            assert(subDAGs.insert(std::make_pair(instr.subDAG,std::vector<bh_intp>(1,instr.index))).second);
        } else {
            assert(subDAGs.find(instr.subDAG) != subDAGs.end());
        }
        return instr.subDAG;
    }

    bool Flow::merge(bh_intp id1, bh_intp id2)
    {
        auto sdi1 = subDAGs.find(id1);
        auto sdi2 = subDAGs.find(id2);
        assert(sdi1 != subDAGs.end() && sdi2 != subDAGs.end());
        if (id1 == id2)
        {
            return true;
        }
        for (bh_intp iid1: sdi1->second)
        {
            if (bh_instructions[iid1].opcode == BH_FREE || bh_instructions[iid1].opcode == BH_DISCARD)
                continue;
            for (bh_intp iid2: sdi2->second)
            {
                if (bh_instructions[iid2].opcode == BH_FREE || bh_instructions[iid2].opcode == BH_DISCARD)
                    continue;
                for(bh_intp o = 1; o < bh_operands_in_instruction(&bh_instructions[iid2]); ++o)
                {
                    if (!(bh_view_disjoint(view(Node(iid1,0)), view(Node(iid2,o))) || 
                          bh_view_aligned(view(Node(iid1,0)), view(Node(iid2,o)))))
                        return false;
                }
                if (!(bh_view_disjoint(view(Node(iid1,0)), view(Node(iid2,0))) || 
                      bh_view_aligned(view(Node(iid1,0)), view(Node(iid2,0)))))
                    return false;
                for(bh_intp o = 1; o < bh_operands_in_instruction(&bh_instructions[iid1]); ++o)
                {
                    if (!(bh_view_disjoint(view(Node(iid1,o)), view(Node(iid2,0))) || 
                          bh_view_aligned(view(Node(iid1,o)), view(Node(iid2,0)))))
                        return false;
                }
            }
        }
        for (bh_intp iid: sdi2->second)
        {
            instructions[iid].subDAG = id1;
        }
        sdi1->second.insert(sdi1->second.end(),sdi2->second.begin(),sdi2->second.end());
        subDAGs.erase(sdi2);
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
        std::vector<std::set<bh_intp> > instr_deps(instructions.size()); // flow_instr* -> bh_intp
        //Map between flow and bhir sub-DAG indices
        std::map<bh_intp, bh_intp> dag_f2b;
        //Number of sub-DAGs
        size_t ndags = 0;

        //Initiate the sub-DAGs
        //for(vector<flow_instr>::const_iterator i=flow_instr_list.begin(); i!=flow_instr_list.end(); i++)
        for (const Instruction& i: instructions)
        {
            assert(i.subDAG != -1);
            if(dag_f2b.count(i.subDAG) == 0)
                dag_f2b[i.subDAG] = ndags++;
        }
        //A set of dependencies for each sub-DAG in the bhir
        std::vector<std::set<bh_intp> > dag_deps(ndags);

        //A set of instructions for each sub-DAG in the flow
        std::vector<std::set<bh_intp> > dag_nodes(ndags); // flow_instr* -> bh_intp

        //Compute dependencies both between nodes and sub-DAGs
        for (const Instruction& i: instructions)
        {
            for(bh_intp iid: conflicts(i))
            {
                const Instruction& ci = instructions[iid];
                if(i.subDAG == ci.subDAG)//The dependency is within a sub-DAG
                {
                    if(i.index != ci.index)//We cannot conflict with ourself
                        instr_deps[i.index].insert(iid);
                }
                else//The dependency is to another sub-DAG
                {
                    dag_deps[dag_f2b[i.subDAG]].insert(dag_f2b[ci.subDAG]);
                }
            }
            dag_nodes[dag_f2b[i.subDAG]].insert(i.index);
        }

        //Allocate the DAG list
        bhir->ndag = ndags+1;//which includes the root DAG
        bhir->dag_list = (bh_dag*) bh_vector_create(sizeof(bh_dag), bhir->ndag, bhir->ndag);
        if(bhir->dag_list == NULL)
            throw std::bad_alloc();

        //Lets sort the sub-dags topologically
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



    // In order to remove duplicates, we need a view compare function
    // Only used in html function
    struct view_compare {
        bool operator() (const bh_view *v1, const bh_view *v2) const
        {
            //First we compare base, ndim, and start
            int ret = memcmp(v1, v2, 3*sizeof(bh_intp));
            if(ret == 0)
            {//Then we compare shape and stride
                ret = memcmp(v1->shape, v2->shape, v1->ndim*sizeof(bh_index));
                if(ret == 0)
                    ret = memcmp(v1->stride, v2->stride, v1->ndim*sizeof(bh_index));
            }
            if(ret < 0)
                return true;
            else
                return false;
        }
    };
    // Write the flow object as a html table.
    void Flow::html(const char* filename)
    {
        std::ofstream fs(filename);
        fs << "<!DOCTYPE html><html><body>\n<div style=\"float:left;\">";
        fs << "<table border=\"1\" cellpadding=\"5\" style=\"text-align:center\">\n";

        //'table' contains a string for each cell in the html table (excl. the header)
        //such that table[x][y] returns the string at coordinate (x,y).
        //We write to 'table' before writing to file.
        std::map<uint64_t, std::map<uint64_t, std::string> > table;
        
        //Create a map over all views in each base
        std::map<const bh_base *, std::set<const bh_view*,view_compare> > view_in_base;
        for(auto const& base: bases)
        {
            for(const Node& node: base.second)
            {        
                view_in_base[base.first].insert(view(node));
            }
        }
        //Fill 'table'
        uint64_t ncol=0;
        for (const bh_base* base: baseList)
        {
            for (auto v: view_in_base[base])
            {
                for(const Node& n: bases[base])
                {
                    if(bh_view_identical(view(n), v))
                    {
                        char str[100];
                        if(n.write())
                            snprintf(str, 100, "%ld<sub>W</sub><sup>%ld</sup>", 
                                     (long) n.instr, (long) instruction(n).subDAG);
                        else
                            snprintf(str, 100, "%ld<sub>R</sub><sup>%ld</sup>", 
                                     (long) n.instr, (long) instruction(n).subDAG);
                        table[instruction(n).timestep][ncol] += str;
                    }
                }
                ncol++;
            }
        }
        
        //Write base header
        fs << "<tr>\n";
        for (const bh_base* base: baseList)
        {
            fs << "\t<td colspan=\"" <<view_in_base[base].size() << "\">" << base << "<br>";
            fs << " (#elem: " << base->nelem << ", dtype: " << bh_type_text(base->type);
            fs << ")</td>\n";
        }
        fs << "</tr>\n";
        
        //Write view header
        fs << "<tr>\n";
        for (const bh_base* base: baseList)
        {
            for (auto v: view_in_base[base])
            {
                fs << "\t<td>";
                fs << v->start << "(";
                for(bh_intp i=0; i < v->ndim; ++i)
                {
                    fs << v->shape[i];
                    if(i < v->ndim-1)
                        fs << ",";
                }
                fs << ")(";
                for(bh_intp i=0; i < v->ndim; ++i)
                {
                    fs << v->stride[i];
                    if(i < v->ndim-1)
                        fs << ",";
                }
                fs << ")</td>\n";
            }
        }
        fs << "</tr>\n";
        
        //Write 'table'
        for(uint64_t row=0; row<timesteps.size(); ++row)
        {
            fs << "<tr>\n";
            for(uint64_t col=0; col<ncol; ++col)
            {
                if(table[row].count(col) == 1)
                {
                    fs << "\t<td>" << table[row][col] << "</td>\n";
                }
                else
                {
                    fs << "\t<td></td>\n";
                }
            }
            fs << "</tr>\n";
        }
        fs << "</table></div>\n";
        //Write the instruction list
        fs << "<div style=\"float:right;\"><table  border=\"1\">\n";
        for (const auto &subDAG: subDAGs)
        {
            fs << "<tr><td>\n";
            for (bh_intp iid: subDAG.second)
            {
                char buf[100000];
                bh_sprint_instr(&bh_instructions[iid], buf, "<br>");
                fs << "<b>" << iid << "</b>)" << buf << "<br>";
            }
            fs << "</tr></td>\n";
        }
        fs << "</table></div></body></html>\n\n";
        fs.close();
    }

}
