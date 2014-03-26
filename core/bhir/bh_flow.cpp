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
#include <bh_flow.hpp>
#include <assert.h>
#include <algorithm>

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
                Node node(i,o);
                bases[op->base].push_back(node);

                //The timestep of the instruction must be greater than any conflicting instructions
                for(const Node n: conflicts(node))
                {
                    if(instr.timestep <= instruction(n).timestep)
                        instr.timestep = instruction(n).timestep+1;
                }
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
        //clustering();
    }
    
    inline const bh_view* Flow::view(const Node node) const
    { return &bh_instructions[node.instr].operand[node.op]; }

    inline const bh_base* Flow::base(const Node node) const
    { return view(node)->base; }

    inline Flow::Instruction& Flow::instruction(const Node node)
    { return instructions[node.instr]; }

    std::vector<Flow::Node> Flow::conflicts(const Node& node) 
    {
        //Search through all nodes with the same base as 'node'
        const std::vector<Node> &sameBase = bases[base(node)];

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
//        for (const auto &sub_dag: sub_dags)
//        {
            fs << "<tr><td>\n";
            for (const Instruction instr: instructions)
            {
                char buf[100000];
                bh_sprint_instr(&bh_instructions[instr.index], buf, "<br>");
                fs << "<b>" << instr.index << "</b>)" << buf << "<br>";
            }
            fs << "</tr></td>\n";
//        }
        fs << "</table></div></body></html>\n\n";
        fs.close();
    }

}
