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
#include <bh_dag.h>
#include <bh_fuse_cache.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/algorithm/string/predicate.hpp> //For iequals()
#include <boost/graph/connected_components.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/thread.hpp>
#include <vector>
#include <map>
#include <iterator>
#include <stdio.h>
#include <exception>
#include <omp.h>

using namespace std;
using namespace boost;
using namespace bohrium;
using namespace bohrium::dag;

//FILO Task Queue thread safe
class TaskQueue
{
public:
    typedef pair<vector<bool>, unsigned int> Task;
private:
    mutex mtx;
    condition_variable non_empty;
    vector<Task> tasks;
    unsigned int nwaiting;
    const unsigned int nthreads;
    bool finished;
public:
    TaskQueue(unsigned int nthreads):nwaiting(0), nthreads(nthreads), finished(false){}

    void push(const vector<bool> &mask, unsigned int offset)
    {
        unique_lock<mutex> lock(mtx);
        tasks.push_back(make_pair(mask, offset));
        non_empty.notify_one();
    }

    Task pop()
    {
        unique_lock<mutex> lock(mtx);
        if(++nwaiting >= nthreads and tasks.size() == 0)
        {
            finished = true;
            non_empty.notify_all();
            throw overflow_error("Out of work");
        }

        while(tasks.size() == 0 and not finished)
            non_empty.wait(lock);

        if(finished)
            throw overflow_error("Out of work");

        Task ret = tasks.back();
        tasks.pop_back();
        --nwaiting;
        return ret;
    }
};

/* Help function that fuses the edges in 'edges2explore' where the 'mask' is true */
pair<int64_t,bool> fuse_mask(int64_t best_cost, const vector<EdgeW> &edges2explore,
                             const GraphDW &graph, const vector<bool> &mask, bh_ir &bhir,
                             GraphD &dag)
{
    bool fusibility=true;
    vector<EdgeW> edges2merge;
    unsigned int i=0;
    BOOST_FOREACH(const EdgeW &e, edges2explore)
    {
        if(mask[i++])
            edges2merge.push_back(e);
    }

    //Help function to find the new location
    struct find_new_location
    {
        Vertex operator()(const map<Vertex, Vertex> &loc_map, Vertex v)
        {
            Vertex v_mapped = loc_map.at(v);
            if(v_mapped == v)
                return v;
            else
                return (*this)(loc_map, v_mapped);
        }
    }find_loc;

    //'loc_map' maps a vertex before the merge to the corresponding vertex after the merge
    map<Vertex, Vertex> loc_map;
    BOOST_FOREACH(Vertex v, vertices(dag))
    {
        loc_map[v] = v;
    }

    //Lets record the merges into 'loc_map'
    BOOST_FOREACH(const EdgeW &e, edges2merge)
    {
        Vertex v1 = find_loc(loc_map, source(e, graph.bglW()));
        Vertex v2 = find_loc(loc_map, target(e, graph.bglW()));
        loc_map[v1] = v2;
    }

    //Pack 'loc_map' such that all keys maps directly to a new vertex thus after
    //this point there is no need to call find_loc().
    BOOST_FOREACH(Vertex v, vertices(dag))
    {
        Vertex v_mapped = find_loc(loc_map, loc_map.at(v));
        if(v_mapped != v)
            loc_map[v] = v_mapped;
    }

    //Create the new vertices and insert instruction topologically
    map<Vertex, bh_ir_kernel> new_vertices;
    BOOST_FOREACH(Vertex v, vertices(dag))
    {
        if(loc_map.at(v) == v)
            new_vertices[v] = bh_ir_kernel(bhir);
    }
    vector<Vertex> topological_order;
    topological_sort(dag, back_inserter(topological_order));
    BOOST_REVERSE_FOREACH(Vertex vertex, topological_order)
    {
        Vertex v = loc_map.at(vertex);
        bh_ir_kernel &k = new_vertices.at(v);
        BOOST_FOREACH(uint64_t idx, dag[vertex].instr_indexes)
        {
            if(not k.fusible(idx))
                fusibility = false;
            k.add_instr(idx);
        }
    }

    //TODO: Remove this assert check
    BOOST_FOREACH(Vertex v, vertices(dag))
    {
        if(loc_map.at(v) == v)
            assert(new_vertices[v].instr_indexes.size() > 0);
    }

    //Find the total cost
    int64_t cost=0;
    BOOST_FOREACH(const bh_ir_kernel &k, new_vertices | adaptors::map_values)
    {
        cost += k.cost();
    }

    //Check if we need to continue
    if(cost >= best_cost or not fusibility)
        return make_pair(cost,false);

    //Merge the vertice in the DAG
    BOOST_FOREACH(Vertex v, vertices(dag))
    {
        Vertex loc_v = loc_map.at(v);
        if(loc_v == v)
        {
            dag[v] = new_vertices.at(v);
            assert(dag[v].instr_indexes.size() > 0);
        }
        else//Lets merge 'v' into 'loc_v'
        {
            BOOST_FOREACH(Vertex a, adjacent_vertices(v, dag))
            {
                a = loc_map.at(a);
                if(a != loc_v)
                    add_edge(loc_v, a, dag);
            }
            BOOST_FOREACH(Vertex a, inv_adjacent_vertices(v, dag))
            {
                a = loc_map.at(a);
                if(a != loc_v)
                    add_edge(a, loc_v, dag);
            }
            clear_vertex(v, dag);
            dag[v] = bh_ir_kernel(bhir);
        }
    }

    //TODO: remove assert check
    BOOST_FOREACH(Vertex v, vertices(dag))
    {
        if(dag[loc_map.at(v)].instr_indexes.size() == 0)
        {
            cout << v << endl;
            cout << loc_map.at(v) << endl;
            assert(1 == 2);
            exit(-1);
        }
    }

    //Check for cycles
    if(cycles(dag))
    {
        return make_pair(cost,false);
    }

    assert(cost == (int64_t)dag_cost(dag));
    return make_pair(cost,true);
}

/* Find the optimal solution through branch and bound */
GraphD branch_n_bound(bh_ir &bhir, const GraphDW &dag, const vector<EdgeW> &edges2explore, FuseCache &cache,
                      const set<Vertex> &ignores, const vector<bool> &init_mask, unsigned int init_offset=0)
{
    //We use the greedy algorithm to find a good initial guess
    int64_t best_cost;
    GraphD best_dag;
    {
        GraphDW new_dag(dag);
        fuse_greedy(new_dag, &ignores);
        best_dag = new_dag.bglD();
        best_cost = dag_cost(best_dag);
    }
    double purge_count=0;
    uint64_t explore_count=0;

    TaskQueue tasks(omp_get_max_threads());
    tasks.push(init_mask, init_offset);
    #pragma omp parallel
    {
    while(1)
    {
        vector<bool> mask;
        unsigned int offset;
        try{
            tie(mask, offset) = tasks.pop();
        }catch(overflow_error &e){
            break;
        }

        //Fuse the task
        GraphD new_dag(dag.bglD());
        bool fusibility;
        int64_t cost;
        tie(cost, fusibility) = fuse_mask(best_cost, edges2explore, dag, mask, bhir, new_dag);

        if(explore_count%1000000 == 0)
        {
            #pragma omp critical
            {
                cout << "[" << explore_count << "][";
                BOOST_FOREACH(bool b, mask)
                {
                    if(b){cout << "1";}else{cout << "0";}
                }
                cout << "] purge count: ";
                cout << purge_count << " / " << pow(2.0,mask.size());
                cout << ", cost: " << cost << ", best_cost: " << best_cost;
                cout << ", fusibility: " << fusibility << endl;
            }
        }
        #pragma omp critical
        ++explore_count;

        if(cost >= best_cost)
        {
            #pragma omp critical
            purge_count += pow(2.0, mask.size()-offset);
            continue;
        }
        if(fusibility)
        {
            #pragma omp critical
            {
                //Lets save the new best dag
                best_cost = cost;
                best_dag = new_dag;
                assert(dag_validate(best_dag));

                //Lets write the current best to file
                vector<bh_ir_kernel> kernel_list;
                fill_kernel_list(best_dag, kernel_list);
                const InstrIndexesList &i = cache.insert(bhir.instr_list, kernel_list);
                cache.write_to_files();

                stringstream ss;
                string filename;
                i.get_filename(filename);
                ss << "new_best_dag-" << filename << ".dot";
                cout << "write file: " << ss.str() << endl;
                pprint(best_dag, ss.str().c_str());
                purge_count += pow(2.0, mask.size()-offset);
            }
            continue;
        }
        //for(unsigned int i=offset; i<mask.size(); ++i) //breadth first
        for(int i=mask.size()-1; i>= (int)offset; --i)   //depth first
        {
            vector<bool> m1(mask);
            m1[i] = false;
            tasks.push(m1, i+1);
        }
    }}
    return best_dag;
}

void get_edges2explore(const GraphDW &dag, const set<Vertex> &vertices2explore,
                       vector<EdgeW> &edges2explore)
{
    //The list of edges that we should try to merge
    BOOST_FOREACH(const EdgeW &e, edges(dag.bglW()))
    {
        if(vertices2explore.find(source(e, dag.bglW())) != vertices2explore.end() or
           vertices2explore.find(target(e, dag.bglW())) != vertices2explore.end())
            edges2explore.push_back(e);
    }
    sort_weights(dag.bglW(), edges2explore);
    string order;
    {
        const char *t = getenv("BH_FUSER_OPTIMAL_ORDER");
        if(t == NULL)
            order ="regular";
        else
            order = t;
    }
    if(not iequals(order, "regular"))
    {
        if(iequals(order, "reverse"))
        {
            reverse(edges2explore.begin(), edges2explore.end());
        }
        else if(iequals(order, "random"))
        {
            random_shuffle(edges2explore.begin(), edges2explore.end());
        }
        else
        {
            cerr << "FUSER-OPTIMAL: unknown BH_FUSER_OPTIMAL_ORDER: " << order << endl;
            order = "regular";
        }
    }
    cout << "BH_FUSER_OPTIMAL_ORDER: " << order << endl;
}

/* Fuse the 'dag' optimally */
void fuse_optimal(bh_ir &bhir, const GraphDW &dag, const set<Vertex> &vertices2explore,
                  GraphD &output, FuseCache &cache)
{
    //The list of edges that we should try to merge
    vector<EdgeW> edges2explore;
    get_edges2explore(dag, vertices2explore, edges2explore);
    if(edges2explore.size() == 0)
        return;

    //We need the set of vertices that the greedy fusion must ignore
    set<Vertex> ignores;
    BOOST_FOREACH(Vertex v, vertices(dag.bglD()))
    {
        if(vertices2explore.find(v) == vertices2explore.end())
            ignores.insert(v);
    }

    //Check for a preloaded initial condition
    vector<bool> mask(edges2explore.size(), true);
    unsigned int preload_offset=0;
    if(edges2explore.size() > 10)
    {
        const char *t = getenv("BH_FUSER_OPTIMAL_PRELOAD");
        if(t != NULL)
        {
            BOOST_FOREACH(const char &c, string(t))
            {
                mask[preload_offset++] = lexical_cast<bool>(c);
                if(preload_offset == mask.size())
                    break;
            }
            cout << "Preloaded path (" << preload_offset << "): ";
            for(unsigned int j=0; j<preload_offset; ++j)
                cout << mask[j] << ", ";
            cout << endl;
            --preload_offset;
        }
    }

    cout << "FUSER-OPTIMAL: the size of the search space is 2^" << mask.size() << "!" << endl;
    output = branch_n_bound(bhir, dag, edges2explore, cache, ignores, mask, preload_offset);
}

void do_fusion(bh_ir &bhir, FuseCache &cache)
{
    vector<bh_ir_kernel> kernel_list;
    {
        GraphDW dag;
        from_bhir(bhir, dag);
        fill_kernel_list(dag.bglD(), kernel_list);
    }
    while(true)
    {
//        cout << endl << "Starting new round." << endl;
        GraphDW dag;
        from_kernels(kernel_list, dag);
        fuse_gently(dag);
        dag.transitive_reduction();
        assert(dag_validate(dag.bglD()));

        vector<set<Vertex> > component2vertices;
        {
            vector<Vertex> vertex2component(num_vertices(dag.bglW()));
            uint64_t num = connected_components(dag.bglW(), &vertex2component[0]);
            component2vertices.resize(num);
            for(Vertex v=0; v<vertex2component.size(); ++v)
            {
                component2vertices[vertex2component[v]].insert(v);
            }
        }

/*        {
            uint64_t component_id = 0;
            BOOST_FOREACH(set<Vertex> &vertices, component2vertices)
            {
                cout << "Component " << component_id << ": ";
                BOOST_FOREACH(Vertex v, vertices)
                {
                    cout << v << ", ";
                }
                cout << endl;
                ++component_id;
            }
        }
*/

        //Find the smallest component with more than one vertex
        uint64_t comp_size=num_vertices(dag.bglW())+1;
        int comp_id=-1;
        for(uint64_t i=0; i < component2vertices.size(); ++i)
        {
            const uint64_t size = component2vertices[i].size();
           if(1 < size and size < comp_size)
           {
               comp_size = size;
               comp_id = i;
           }
        }

        GraphD output;
        if(comp_id == -1)
        {
//            cout << "Round ended, no more components to fuse" << endl << endl;
            output = dag.bglD();
        }
        else
        {
//            cout << "Fusing component: " << comp_id << endl;
            fuse_optimal(bhir, dag, component2vertices[comp_id], output, cache);
        }
        assert(num_vertices(output) > 0);
        assert(dag_validate(output));
        kernel_list.clear();
        fill_kernel_list(output, kernel_list);

        if(comp_id == -1)
            break;
    }
    bhir.kernel_list = kernel_list;
}

void fuser(bh_ir &bhir, FuseCache &cache)
{
    if(bhir.kernel_list.size() != 0)
        throw logic_error("The kernel_list is not empty!");

    BatchHash batch(bhir.instr_list);
    //if(not cache.lookup(batch, bhir, bhir.kernel_list))
    {
        do_fusion(bhir, cache);
        cache.insert(batch, bhir.kernel_list);
    }
}

