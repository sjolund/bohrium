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
    Flow::Flow(bh_intp ninstr, const bh_instruction *instr_list) 
        : ninstructions(ninstr)
        , bh_instructions(instr_list)
        , instructions((Instruction*)malloc(sizeof(Instruction)*ninstr))
    {
        bh_intp nttid = 0; // Next timetable id
        for(bh_intp i=0; i<ninstr; i++)
        {
            const bh_instruction *bh_instr = &instr_list[i];
            Instruction& instr = instructions[i];
            instr = Instruction(i,bh_instr);

            std::map<bh_intp,bh_intp> timesteps; // std::map<timestepTable, timestep> 
            for(bh_intp o = 0; o < bh_operands_in_instruction(bh_instr); ++o)
            {
                const bh_view *op = &(bh_instr->operand[o]);
                if(bh_is_constant(op))
                    continue;
                auto bi = bases.find(op->base);
                if (bi == bases.end())
                {
                    // Maintain the chronological list of bases
                    baseList.push_back(op->base);
                    bases[op->base] = std::multiset<Node>();
                } else {
                    // Get timestep
                    const std::multiset<Node>& sameBase = bi->second;
                    auto ni = sameBase.crbegin();
                    for (; ni != sameBase.crend(); ++ni)
                    {
                        if(!(o == 0) && !ni->write())
                            continue;//No possible conflict when both is read only
                        if(!bh_view_disjoint(op, ni->view()))
                            break;
                    }
                    if (ni == sameBase.crend()) // No other instructions we have to be after
                    {
                        if (ni == sameBase.crbegin()) // No other instructions accessing same base
                            continue;
                        else if (timesteps.find(sameBase.crbegin()->timestepTable()) == timesteps.end())
                            timesteps[sameBase.crbegin()->timestepTable()] = 0;
                    } else {
                        bh_intp ts = ni->timestep() + 1;
                        if (timesteps.find(ni->timestepTable()) == timesteps.end())
                        {
                            timesteps[ni->timestepTable()] = ts;
                        } else {
                            timesteps[ni->timestepTable()] = MAX(timesteps[ni->timestepTable()],ts);
                        }
                        instr.timestep = MAX(instr.timestep,ts);
                    }
                }
            }
            switch (timesteps.size())
            {
            case 0:
            {   // Create a new timestep table containing the instruction 
//                std::cout << "creating timestepTables[" << nttid << "] instr: " << i << std::endl;
                assert (instr.timestep == 0);
                instr.timestepTable = nttid;
                assert (timestepTables.find(nttid) == timestepTables.end());
                timestepTables.insert(std::make_pair(nttid,
                               std::vector<std::vector<bh_intp> >(1,std::vector<bh_intp>(1,i))));
                ++nttid;
                break;
            }
            case 1:
            {   // Add the instruction to the corect timestep table 
                instr.timestepTable = timesteps.begin()->first; 
//                std::cout << "table: " << instr.timestepTable << " (" << timesteps.begin()->second << 
//                    "), timestep: " << instr.timestep << " instr: " << i << std::endl;
                assert (instr.timestep == timesteps.begin()->second);
                auto ttsit = timestepTables.find(instr.timestepTable);
                assert (ttsit != timestepTables.end());
                std::vector<std::vector<bh_intp> >&  timestepTable = ttsit->second;
                if (instr.timestep < timestepTable.size())
                {
                    timestepTable[instr.timestep].push_back(i);
                } else {
                    assert (instr.timestep == timestepTable.size());
                    timestepTable.push_back(std::vector<bh_intp>(1,i));
                }
                break;
            }
            default:
            {   // Merge the timestep tables and insert the instruction into the merged table
                auto tsi = timesteps.begin();
                // Find the primary table (the one where the timestep maches)
                while (tsi->second < instr.timestep) 
                    ++tsi;
                assert (tsi != timesteps.end());
                assert (tsi->second == instr.timestep);
                instr.timestepTable = tsi->first;
                auto ttsit = timestepTables.find(instr.timestepTable);
                assert (ttsit != timestepTables.end());
                std::vector<std::vector<bh_intp> >& timestepTable = ttsit->second;
                timesteps.erase(tsi); // Delete the primary table reference. The rest will be merged in
                for (tsi = timesteps.begin(); tsi != timesteps.end(); ++tsi)
                {
                    bh_intp table_ = tsi->first;
                    bh_intp delta = instr.timestep - tsi->second; // timestep delta
//                    std::cout << "merging " << instr.timestepTable << " <- " << table_ << " delta: " << 
//                        delta << std::endl; 
                    assert (delta >= 0);
                    auto ttsit_ = timestepTables.find(table_);
                    assert (ttsit_ != timestepTables.end());
                    const std::vector<std::vector<bh_intp> >& timestepTable_ = ttsit_->second;
                    for (const std::vector<bh_intp>& timestep: timestepTable_)
                    {
                        for (bh_intp iidx: timestep)
                        {
                            Instruction& i = instructions[iidx];
                            i.timestepTable = instr.timestepTable;
                            i.timestep += delta;
//                            std::cout << "--- " << iidx << " " << i.timestep << " " <<  timestepTable.size() << std::endl;
                            if (i.timestep < timestepTable.size())
                            {
                                timestepTable[i.timestep].push_back(iidx);
                            } else {
                                assert (i.timestep == timestepTable.size());
                                timestepTable.push_back(std::vector<bh_intp>(1,iidx));
                            }
                        }
                    }
                    // Delete the obsolete timetable
//                    std::cout << "deleting timestepTables[" << table_ << "]" << std::endl;
                    timestepTables.erase(ttsit_);
                }
                // insert the new instruction
//                std::cout << "table: " << instr.timestepTable << ", timestep: " << instr.timestep << 
//                    " instr: " << i << std::endl;
                if (instr.timestep < timestepTable.size())
                {
                    timestepTable[instr.timestep].push_back(i);
                } else {
                    assert (instr.timestep == timestepTable.size());
                    timestepTable.push_back(std::vector<bh_intp>(1,i));
                }
            }
            }
            for(bh_intp o = 0; o < bh_operands_in_instruction(bh_instr); ++o)
            {   // Insert Nodes into bases now that the instr.timestep is set 
                const bh_view *op = &(bh_instr->operand[o]);
                if(bh_is_constant(op))
                    continue;
                auto bi = bases.find(op->base);
                assert (bi != bases.end());
                bi->second.insert(Node(instr,o));
            }            
        }
        clustering();
    }

    void Flow::clustering()
    {
        for (const auto tti: timestepTables)
        {
            const std::vector< std::vector< bh_intp > >& timesteps = tti.second;
            for (bh_intp iid: timesteps[0])
                subDAG(iid);
            for (size_t ts = 1; ts < timesteps.size(); ++ts)
            {
                const std::vector<bh_intp> iids = timesteps[ts];
                for (bh_intp iid: iids)
                {
                    const bh_instruction *bh_instr = &bh_instructions[iid];
                    for(bh_intp o = 0; o < bh_operands_in_instruction(bh_instr); ++o)
                    {
                        const bh_view* op = &(bh_instr->operand[o]);
                        if(bh_is_constant(op))
                            continue;
                        const std::multiset<Node>& nodes = bases[op->base];
                        Node node(instructions[iid],o);
                        auto ni = nodes.find(node); // Find equivilant node
                        assert (ni != nodes.end());
                        // Find last node in previous timestep
                        while (ni != nodes.begin() && ni->timestep() >= ts -1)
                            --ni;
                        if (ni->timestep() < ts -1)
                            ++ni;
                        // Try to merege with all nodes up to self
                        while (*ni != node)
                        {
                            merge(subDAG(iid),subDAG(ni->instr.index));
                            ++ni;
                            assert (ni != nodes.end());
                            assert (ni->timestep() <= ts);
                        }
                    }
                }
            }
        }
    }


    std::multiset<Flow::Node> Flow::conflicts(const Node& node) const
    {
        //Search through all nodes with the same base as 'node'
        auto sbit = bases.find(node.base());
        assert (sbit != bases.end());
        const std::multiset<Node>& sameBase = sbit->second;
        
        // Find same or equivalent node
        auto endit = sameBase.find(node); 
        assert(endit != sameBase.end()); //'node' must be found in 'same_base'
        
        std::multiset<Node> res;
        //Now continue iterating through possible conflicts
        for(auto it = sameBase.begin() ; it != endit; ++it)
        {
            if(!node.write() && !it->write())
                continue;//No possible conflict when both is read only
            
            if(node.instr == it->instr)
                continue;//No possible conflict within the same instruction
            
            if(!bh_view_disjoint(node.view(), it->view()))
            {
                res.insert(*it);
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
            for(const Node n: conflicts(Node(instr,o)))
            {
                res.insert(n.instr.index);
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
            // assert that we are actually ADDING a subDAG
            bool adding = subDAGs.insert(std::make_pair(instr.subDAG,std::vector<bh_intp>(1,instr.index))).second;
            assert(adding);
        } else {
            assert(subDAGs.find(instr.subDAG) != subDAGs.end());
        }
        return instr.subDAG;
    }

    inline const bh_view* Flow::view(bh_intp i, bh_intp o) const
    { return &bh_instructions[i].operand[o]; }

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
                    if (!(bh_view_disjoint(view(iid1,0), view(iid2,o)) || 
                          bh_view_aligned(view(iid1,0), view(iid2,o))))
                        return false;
                }
                if (!(bh_view_disjoint(view(iid1,0), view(iid2,0)) || 
                      bh_view_aligned(view(iid1,0), view(iid2,0))))
                    return false;
                for(bh_intp o = 1; o < bh_operands_in_instruction(&bh_instructions[iid1]); ++o)
                {
                    if (!(bh_view_disjoint(view(iid1,o), view(iid2,0)) || 
                          bh_view_aligned(view(iid1,o), view(iid2,0))))
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
        std::vector<std::set<bh_intp> > instr_deps(ninstructions); // flow_instr* -> bh_intp
        //Map between flow and bhir sub-DAG indices
        std::map<bh_intp, bh_intp> dag_f2b;
        //Number of sub-DAGs
        size_t ndags = 0;

        //Initiate the sub-DAGs
        //for(vector<flow_instr>::const_iterator i=flow_instr_list.begin(); i!=flow_instr_list.end(); i++)
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

        // Lets sort the sub-dags topologically
        // {
        //     topological_sort top = topological_sort(dag_deps);
        //     std::vector<bh_intp> sorted = top.get_map();
        //     const std::vector<std::set<bh_intp> > t_deps(dag_deps);
        //     const std::vector<std::set<bh_intp> > t_nodes(dag_nodes); // flow_instr* -> bh_intp
        //     for(size_t i=0; i<sorted.size(); ++i)
        //     {
        //         dag_deps[i] = t_deps[sorted[i]];
        //         dag_nodes[i] = t_nodes[sorted[i]];
        //     }
        // }

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
                view_in_base[base.first].insert(node.view());
            }
        }
        bh_intp timesteps = 0;
        //Fill 'table'
        uint64_t ncol=0;
        for (const bh_base* base: baseList)
        {
            for (auto v: view_in_base[base])
            {
                for(const Node& n: bases[base])
                {
                    if(bh_view_identical(n.view(), v))
                    {
                        char str[100];
                        snprintf(str, 100, "%ld<sub>%s</sub><sup>%ld</sup>", 
                                 (long) n.instr.index, n.write()?"W":"R", (long) n.instr.subDAG);
                        table[n.instr.timestep][ncol] += str;
                        timesteps = MAX(n.instr.timestep,timesteps);
                    }
                }
                ncol++;
            }
        }
        ++timesteps;
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
        for(uint64_t row=0; row<timesteps; ++row)
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
