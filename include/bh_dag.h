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

#ifndef __BH_IR_DAG_H
#define __BH_IR_DAG_H

#include <boost/foreach.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <iterator>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <stdexcept>
#include <bh.h>

namespace bohrium {
namespace dag {

typedef uint64_t Vertex;

//The weight class bundled with the weight graph
struct EdgeWeight
{
    int64_t value;
    Vertex src, dst;
    bool ordered;
    EdgeWeight(){}
    EdgeWeight(int64_t weight, Vertex s, Vertex d):value(weight), src(s), dst(d), ordered(true){}
    EdgeWeight(int64_t weight):value(weight), ordered(false){}
};

//The type declaration of the boost graphs, vertices and edges.
typedef boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS,
                              bh_ir_kernel> GraphD;
typedef boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS,
                              boost::no_property, EdgeWeight> GraphW;
typedef typename boost::graph_traits<GraphD>::edge_descriptor EdgeD;
typedef typename boost::graph_traits<GraphW>::edge_descriptor EdgeW;

//Forward declaration
bool path_exist(Vertex a, Vertex b, const GraphD &dag,
                bool ignore_neighbors);

/* The GraphDW class encapsulate both a dependency graph and
 * a weight graph. The public methods ensures that the two
 * graphs are synchronized and are always in a valid state.
 */
class GraphDW
{
protected:
    GraphD _bglD;
    GraphW _bglW;

public:
    const GraphD &bglD() const {return _bglD;}
    const GraphW &bglW() const {return _bglW;}

    /* Adds a vertex to both the dependency and weight graph.
     * Additionally, both dependency and weight edges are
     * added / updated as needed.
     *
     * @kernel  The kernel to bundle with the new vertex
     */
    Vertex add_vertex(const bh_ir_kernel &kernel)
    {
        Vertex d = boost::add_vertex(kernel, _bglD);
        Vertex w = boost::add_vertex(_bglW);
        assert(w == d);

        //Add edges
        BOOST_REVERSE_FOREACH(Vertex v, vertices(_bglD))
        {
            if(d != v and not path_exist(v, d, _bglD, false))
            {
                bool dependency = false;
                if(kernel.dependency(_bglD[v]))
                {
                    dependency = true;
                    boost::add_edge(v, d, _bglD);
                }
                int64_t cost = kernel.dependency_cost(_bglD[v]);
                if((cost > 0) or (cost == 0 and dependency))
                {
                    if(dependency)
                        boost::add_edge(v, d, EdgeWeight(cost, v, d), _bglW);
                    else
                        boost::add_edge(v, d, EdgeWeight(cost), _bglW);
                }
            }
        }
        return d;
    }

    /* The default and the copy constructors */
    GraphDW(){};
    GraphDW(const GraphDW &graph):_bglD(graph.bglD()), _bglW(graph.bglW()) {};

    /* Constructor based on a dependency graph. All weights are zero.
     *
     * @dag     The dependency graph
     */
    GraphDW(const GraphD &dag)
    {
        _bglD = dag;
        _bglW = GraphW(boost::num_vertices(dag));
    }

    /* Clear the vertex without actually removing it.
     * NB: invalidates all existing edge iterators
     *     but NOT pointers to neither vertices nor edges.
     *
     * @v  The Vertex
     */
    void clear_vertex(Vertex v)
    {
        boost::clear_vertex(v, _bglD);
        boost::clear_vertex(v, _bglW);
        _bglD[v] = bh_ir_kernel();
    }

    /* Remove the previously cleared vertices.
     * NB: invalidates all existing vertex and edge pointers
     *     and iterators
     */
    void remove_cleared_vertices()
    {
        std::vector<Vertex> removes;
        BOOST_FOREACH(Vertex v, boost::vertices(_bglD))
        {
            if(_bglD[v].instr_list().size() == 0)
            {
                removes.push_back(v);
            }
        }
        //NB: because of Vertex invalidation, we have to traverse in reverse
        BOOST_REVERSE_FOREACH(Vertex &v, removes)
        {
            boost::remove_vertex(v, _bglD);
            boost::remove_vertex(v, _bglW);
        }
    }

    /* Merge vertex 'a' and 'b' by appending 'b's instructions to 'a'.
     * Vertex 'b' is cleared rather than removed thus existing vertex
     * and edge pointers are still valid after the merge.
     *
     * NB: invalidates all existing edge iterators.
     *
     * @a  The first vertex
     * @b  The second vertex
     */
    void merge_vertices(Vertex a, Vertex b)
    {
        using namespace std;
        using namespace boost;

        BOOST_FOREACH(const bh_instruction &i, _bglD[b].instr_list())
        {
            _bglD[a].add_instr(i);
        }
        BOOST_FOREACH(const Vertex &v, adjacent_vertices(b, _bglD))
        {
            if(a != v)
            {
                add_edge(a, v, _bglD);
                int64_t cost = _bglD[v].dependency_cost(_bglD[a]);
                EdgeD e;
                bool exist;
                tie(e, exist) = edge(a, v, _bglW);
                if(exist)
                {
                    if(cost >= 0)
                        _bglW[e] = EdgeWeight(cost, a, v);
                    else
                        remove_edge(e, _bglW);
                }
                else if(cost >= 0)
                {
                    add_edge(a, v, EdgeWeight(cost, a, v), _bglW);
                }
            }
        }
        BOOST_FOREACH(const Vertex &v, inv_adjacent_vertices(b, _bglD))
        {
            if(a != v)
            {
                add_edge(v, a, _bglD);
                int64_t cost = _bglD[a].dependency_cost(_bglD[v]);
                EdgeD e;
                bool exist;
                tie(e, exist) = edge(v, a, _bglW);
                if(exist)
                {
                    if(cost >= 0)
                        _bglW[e] = EdgeWeight(cost, v, a);
                    else
                        remove_edge(e, _bglW);
                }
                else if(cost >= 0)
                {
                    add_edge(v, a, EdgeWeight(cost, v, a), _bglW);
                }
            }
        }
        BOOST_FOREACH(const Vertex &v, adjacent_vertices(b, _bglW))
        {
            if(a != v)
            {
                int64_t cost = _bglD[a].dependency_cost(_bglD[v]);
                EdgeD e;
                bool exist;
                tie(e, exist) = edge(v, a, _bglW);
                if(exist)
                {
                    if(_bglW[e].ordered)
                    {
                        if(cost >= 0)
                            _bglW[e] = EdgeWeight(cost);
                        else
                            remove_edge(e, _bglW);
                    }
                }
                else if(cost >= 0)
                {
                    add_edge(v, a, EdgeWeight(cost), _bglW);
                }
            }
        }
        clear_vertex(b);
        if(not _bglD[a].fusible())
        {
            cout << "kernel merge " << a << " " << b << endl;
            assert(1 == 2);
        }
    }

    /* Transitive reduce the 'dag', i.e. remove all redundant edges,
     * NB: invalidates all existing edge iterators.
     *
     * Complexity: O(E * (E + V))
     *
     * @a   The first vertex
     * @b   The second vertex
     * @dag The DAG
     */
    void transitive_reduction()
    {
        BOOST_FOREACH(EdgeD e, edges(_bglD))
        {
            if(path_exist(source(e,_bglD), target(e,_bglD), _bglD, true))
               boost::remove_edge(e, _bglD);
        }
    }
};

/* Creates a new DAG based on a bhir that consist of single
 * instruction kernels.
 * NB: a vertex in the 'dag' must bundle with the bh_ir_kernel class
 *
 * Complexity: O(n^2) where 'n' is the number of instructions
 *
 * @bhir  The BhIR
 * @dag   The output dag
 *
 * Throw logic_error() if the kernel_list wihtin 'bhir' isn't empty
 */
void from_bhir(const bh_ir &bhir, GraphDW &dag)
{
    using namespace std;
    using namespace boost;

    if(bhir.kernel_list.size() != 0)
    {
        throw logic_error("The kernel_list is not empty!");
    }
    //Build a singleton DAG
    BOOST_FOREACH(const bh_instruction &instr, bhir.instr_list)
    {
        bh_ir_kernel k;
        k.add_instr(instr);
        dag.add_vertex(k);
    }
}

/* Creates a new DAG based on a kernel list where each vertex is a kernel.
 *
 * Complexity: O(E + V)
 *
 * @kernels The kernel list
 * @dag     The output dag
 */
void from_kernels(const std::vector<bh_ir_kernel> &kernels, GraphDW &dag)
{
    using namespace std;
    using namespace boost;

    BOOST_FOREACH(const bh_ir_kernel &kernel, kernels)
    {
        if(kernel.instr_list().size() == 0)
            continue;

        dag.add_vertex(kernel);
    }
}

/* Fills the kernel list based on the DAG where each vertex is a kernel.
 *
 * Complexity: O(E + V)
 *
 * @dag     The dag
 * @kernels The kernel list output
 */
void fill_kernels(const GraphD &dag, std::vector<bh_ir_kernel> &kernels)
{
    using namespace std;
    using namespace boost;

    vector<Vertex> topological_order;
    topological_sort(dag, back_inserter(topological_order));
    BOOST_REVERSE_FOREACH(const Vertex &v, topological_order)
    {
        if(dag[v].instr_list().size() > 0)
            kernels.push_back(dag[v]);
    }
}

/* Determines whether there exist a path from 'a' to 'b' with
 * length more than one ('a' and 'b' is not adjacent).
 *
 * Complexity: O(E + V)
 *
 * @a                 The first vertex
 * @b                 The second vertex
 * @dag               The DAG
 * @ignore_neighbors  Whether to accept neighbor paths
 * @return            True if there is a path
 */
bool path_exist(Vertex a, Vertex b, const GraphD &dag,
                bool ignore_neighbors=false)
{
    using namespace std;
    using namespace boost;

    struct path_visitor:default_bfs_visitor
    {
        const Vertex dst;
        path_visitor(Vertex b):dst(b){};

        void examine_edge(EdgeD e, const GraphD &g) const
        {
            if(target(e,g) == dst)
                throw runtime_error("");
        }
    };
    struct long_visitor:default_bfs_visitor
    {
        const Vertex src, dst;
        long_visitor(Vertex a, Vertex b):src(a),dst(b){};

        void examine_edge(EdgeD e, const GraphD &g) const
        {
            if(source(e,g) != src and target(e,g) == dst)
                throw runtime_error("");
        }
    };
    try
    {
        if(ignore_neighbors)
            breadth_first_search(dag, a, visitor(long_visitor(a,b)));
        else
            breadth_first_search(dag, a, visitor(path_visitor(b)));
    }
    catch (const runtime_error &e)
    {
        return true;
    }
    return false;
}

/* Determines whether there are cycles in the Graph
 *
 * Complexity: O(E + V)
 *
 * @g       The digraph
 * @return  True if there are cycles in the digraph, else false
 */
bool cycles(const GraphD &g)
{
    using namespace std;
    using namespace boost;
    try
    {
        //TODO: topological sort is an efficient method for finding cycles,
        //but we should avoid allocating a vector
        vector<Vertex> topological_order;
        topological_sort(g, back_inserter(topological_order));
        return false;
    }
    catch (const not_a_dag &e)
    {
        return true;
    }
}

/* Determines the cost of the DAG.
 *
 * Complexity: O(E + V)
 *
 * @dag     The DAG
 * @return  The cost
 */
uint64_t dag_cost(const GraphD &dag)
{
    using namespace std;
    using namespace boost;

    uint64_t cost = 0;
    BOOST_FOREACH(const Vertex &v, vertices(dag))
    {
        cost += dag[v].cost();
    }
    return cost;
}

/* Sort the weights in descending order
 *
 * Complexity: O(E * log E)
 *
 * @edges  The input/output edge list
 */
void sort_weights(const GraphW &dag, std::vector<EdgeW> &edges)
{
    struct wcmp
    {
        const GraphW &graph;
        wcmp(const GraphW &d): graph(d){}
        bool operator() (const EdgeW &e1, const EdgeW &e2)
        {
            return (graph[e1].value > graph[e2].value);
        }
    };
    sort(edges.begin(), edges.end(), wcmp(dag));
}

/* Writes the DOT file of a DAG
 *
 * Complexity: O(E + V)
 *
 * @dag       The DAG to write
 * @filename  The name of DOT file
 */
void pprint(const GraphDW &dag, const char filename[])
{
    using namespace std;
    using namespace boost;
    //Lets create a graph with both vertical and horizontal edges
    GraphD new_dag(dag.bglD());
    map<pair<Vertex, Vertex>, pair<int64_t, bool> > weights;

    BOOST_FOREACH(const EdgeW &e, edges(dag.bglW()))
    {
        Vertex src = source(e, dag.bglW());
        Vertex dst = target(e, dag.bglW());
        bool exist = edge(src,dst,new_dag).second or edge(dst,src,new_dag).second;
        if(not exist)
            add_edge(src, dst, new_dag);

        //Save an edge map of weights and if it is directed
        weights[make_pair(src,dst)] = make_pair(dag.bglW()[e].value, exist);
    }

    //We define a graph and a kernel writer for graphviz
    struct graph_writer
    {
        const GraphD &graph;
        graph_writer(const GraphD &g) : graph(g) {};
        void operator()(std::ostream& out) const
        {
            out << "labelloc=\"t\";" << endl;
            out << "label=\"DAG with a total cost of " << dag_cost(graph);
            out << " bytes\";" << endl;
            out << "graph [bgcolor=white, fontname=\"Courier New\"]" << endl;
            out << "node [shape=box color=black, fontname=\"Courier New\"]" << endl;
        }
    };
    struct kernel_writer
    {
        const GraphD &graph;
        kernel_writer(const GraphD &g) : graph(g) {};
        void operator()(std::ostream& out, const Vertex& v) const
        {
            char buf[1024*10];
            out << "[label=\"Kernel " << v << ", cost: " << graph[v].cost();
            out << " bytes\\n";
            out << "Input views: \\l";
            BOOST_FOREACH(const bh_view &i, graph[v].input_list())
            {
                bh_sprint_view(&i, buf);
                out << buf << "\\l";
            }
            out << "Output views: \\l";
            BOOST_FOREACH(const bh_view &i, graph[v].output_list())
            {
                bh_sprint_view(&i, buf);
                out << buf << "\\l";
            }
            out << "Temp base-arrays: \\l";
            BOOST_FOREACH(const bh_base *i, graph[v].temp_list())
            {
                bh_sprint_base(i, buf);
                out << buf << "\\l";
            }
            out << "Instruction list: \\l";
            BOOST_FOREACH(const bh_instruction &i, graph[v].instr_list())
            {
                bh_sprint_instr(&i, buf, "\\l");
                out << buf << "\\l";
            }
            out << "\"]";
        }
    };
    struct edge_writer
    {
        const GraphD &graph;
        const map<pair<Vertex, Vertex>, pair<int64_t, bool> > &wmap;
        edge_writer(const GraphD &g, const map<pair<Vertex, Vertex>, pair<int64_t, bool> > &w) : graph(g), wmap(w) {};
        void operator()(std::ostream& out, const EdgeD& e) const
        {
            Vertex src = source(e, graph);
            Vertex dst = target(e, graph);
            int64_t c = -1;
            bool directed = true;
            map<pair<Vertex, Vertex>, pair<int64_t, bool> >::const_iterator it = wmap.find(make_pair(src,dst));
            if(it != wmap.end())
                tie(c,directed) = (*it).second;

            out << "[label=\" ";
            if(c == -1)
                out << "N/A\" color=red";
            else
                out << c << " bytes\"";
            if(not directed)
                out << " dir=none color=green constraint=false";
            out << "]";
        }
    };
    ofstream file;
    file.open(filename);
    write_graphviz(file, new_dag, kernel_writer(new_dag),
                   edge_writer(new_dag, weights), graph_writer(new_dag));
    file.close();
}

/* Fuse vertices in the graph that can be fused without
 * changing any future possible fusings
 * NB: invalidates all existing vertex and edge pointers.
 *
 * Complexity: O(E * (E + V))
 *
 * @dag The DAG to fuse
 */
void fuse_gentle(GraphDW &dag)
{
    using namespace std;
    using namespace boost;

    const GraphD &d = dag.bglD();
    bool not_finished = true;
    while(not_finished)
    {
        not_finished = false;
        BOOST_FOREACH(const EdgeD &e, edges(d))
        {
            const Vertex &src = source(e, d);
            const Vertex &dst = target(e, d);
            if((in_degree(dst, d) == 1 and out_degree(dst, d) == 0) or
               (in_degree(src, d) == 0 and out_degree(src, d) == 1) or
               (in_degree(dst, d) <= 1 and out_degree(src, d) <= 1))
            {
                if(d[dst].fusible_gently(d[src]))
                {
                    dag.merge_vertices(src, dst);
                    not_finished = true;
                    break;
                }
            }
        }
    }
    dag.remove_cleared_vertices();
}

/* Fuse vertices in the graph greedily, which is a non-optimal
 * algorithm that fuses the most costly edges in the DAG first.
 * NB: invalidates all existing edge iterators.
 *
 * Complexity: O(E^2 * (E + V))
 *
 * @dag The DAG to fuse
 */
void fuse_greedy(GraphDW &dag)
{
    using namespace std;
    using namespace boost;

    bool not_finished = true;
    vector<EdgeW> sorted;
    while(not_finished)
    {
        not_finished = false;
        sorted.clear();
        BOOST_FOREACH(const EdgeW &e, edges(dag.bglW()))
        {
            sorted.push_back(e);
        }
        sort_weights(dag.bglW(), sorted);
        BOOST_FOREACH(const EdgeW &e, sorted)
        {
            GraphDW new_dag(dag);
            Vertex a = source(e, dag.bglW());
            Vertex b = target(e, dag.bglW());
            if(path_exist(a, b, dag.bglD(), false))
                new_dag.merge_vertices(a, b);
            else
                new_dag.merge_vertices(b, a);

            if(not cycles(new_dag.bglD()))
            {
                dag = new_dag;
                not_finished = true;
                break;
            }
        }
    }
    dag.remove_cleared_vertices();
}

}} //namespace bohrium::dag

#endif
