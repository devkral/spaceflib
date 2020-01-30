/*! \file fgraph.hpp
    \author Alexander Kaftan
    \brief Base Graph Class
*/
#ifndef spaceffgraph
#define spaceffgraph

// c++ stuff

#include "commondefinitions.h"
#include <vector>
#include <algorithm>
#include <istream>
#include <iostream>
#include <tuple>

#ifdef USE_BOOST
#include <boost/graph/graphviz.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/lexical_cast.hpp>

typedef boost::property < boost::vertex_name_t, std::string> VertexProperty;
// be explicit, maybe make first class graph object from it
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexProperty> SFLBoostGraph;

#endif

//! link to next node including backlink
/*! \typedef AdjEntry
    \param 1 other node
    \param 2 position of backlink to this node in the array of the other node
*/
typedef std::tuple<SFL_ID_SIZE, SFL_POS_SIZE> AdjEntry;
//! Array of AdjEntry objects
typedef std::vector<AdjEntry> AdjArray;

//! Node of graph
/*! \struct sfl_node
*/
struct sfl_node{
    //! edges of node
    AdjArray edges;
    sfl_node(){}
};

//! iterator for nodes in adjacence array of SFLGraph
/*! \typedef node_iterator
*/
typedef std::vector<sfl_node>::const_iterator node_iterator;

//! iterator for degree of nodes in adjacence array of SFLGraph
/*! \class DegIterator
*/
class DegIterator : public uint64_iterator
{
    node_iterator node_iter;
public:
    //! copy constructor, required
    /*!
        \param it node_iterator of SFLGraph nodes array
    */
    DegIterator(node_iterator it) :node_iter(it) {}
    //! move constructor
    DegIterator(const DegIterator& mit) : node_iter(mit.node_iter) {}
    //! prefix ++
    DegIterator& operator++() {++node_iter;return *this;}
    //! postfix ++
    DegIterator operator++(int) {DegIterator tmp(*this); operator++(); return tmp;}
    //! compare iterators ==
    bool operator==(const DegIterator& rhs) const {return node_iter==rhs.node_iter;}
    //! compare iterators !=
    bool operator!=(const DegIterator& rhs) const {return node_iter!=rhs.node_iter;}
    //! get degree
    uint64_t operator*() {return node_iter->edges.size();}
};

//! Base Implementation
/*! \class SFLGraph
*/
class SFLGraph{
protected:
    //! adjacence array
    std::vector<sfl_node> nodes;
    //! initialize graph with number of nodes, needs further initialisation
    /*!
        \param num_nodes number of nodes to initialize with
    */
    SFLGraph(SFL_ID_SIZE num_nodes) : nodes(num_nodes) {}
public:
    //! disable copy constructor
    SFLGraph(const SFLGraph& other) = delete;
    //! Move Constructor
    /*!
        steal node list
    */
    SFLGraph(SFLGraph&& other) = default;
    //! allow move assignment operation
    SFLGraph& operator=(SFLGraph&&) = default;

    //! destructor
    virtual ~SFLGraph(){}
    //! Constructs graph starting with id 1
    /*! \param num_nodes amount of nodes generated. Highest node number num_nodes-1
        \param edges nodeid pairs, should be unique pairs. Must be multiple of 2 (pairs)
        \param num_edges number of edges (pairs)
        \return NULL or SFLGraph
        Highest node number == num_nodes
    */
    static SFLGraph create(SFL_ID_SIZE num_nodes, const SFL_ID_SIZE edges[], uint64_t num_edges){
        SFLGraph temp(num_nodes);
        SFL_POS_SIZE current_position, counter_position;
        SFL_ID_SIZE current_node, next_node;
        for (uint64_t count_edge=0; count_edge<num_edges*2; count_edge+=2){
            current_node = edges[count_edge];
            next_node = edges[count_edge+1];
            // invalid nodes
            SFLCHECK(current_node!=0 && next_node!=0)
            SFLCHECK(current_node!=next_node)
            SFLCHECK(current_node<=num_nodes && next_node<=num_nodes)
            current_position = temp.deg(current_node)+1;
            counter_position = temp.deg(next_node)+1;
            //std::cerr << current_node << " " << next_node << " "  << current_position << " " << counter_position << std::endl;
            temp.nodes[current_node-1].edges.push_back(AdjEntry(next_node, counter_position));
            temp.nodes[next_node-1].edges.push_back(AdjEntry(current_node, current_position));
        }
        return temp;
    }
    //! Constructs graph from adjacence list file
    /*! \param in input stream
        \return SFLGraph or fails
        Note: lower nodes must specify links to higher nodes
        TODO: fix detection of missing newline
    */
    static SFLGraph create_from_adj(std::istream &in){
        std::size_t max_size=0;
        SFL_POS_SIZE current_position, counter_position;
        std::vector<char> number; // save the single parts of the number here
        SFL_ID_SIZE next_node;
        char tempc, old_tempc='\n';
        SFLCHECK(in.good())

        std::size_t count_lines=0;
        while (in.good()){
            tempc = in.get();
            if (tempc=='\n' || (tempc==-1 && old_tempc!='\n')){
                count_lines++;
            }
            old_tempc = tempc;
        }
        SFLGraph temp(count_lines);
        count_lines = 0;
        in.clear();
        in.seekg(0, std::ios_base::beg);
        while (in.good()){
            tempc = in.get();
            if (!number.empty() && (tempc==-1 || tempc == ' ' || tempc == '\n')){
                number.push_back('\0');
                next_node = strtoul(number.data(), NULL, 0);
                number.clear();
                if (next_node<=count_lines+1){ // skip, should be in there already
                    // check that this node exists as counter node in lower node
                    // EXPENSIVE OPERATION
#ifndef NO_EXPENSIVE_VALIDATIONS
                    assert(
                        std::find_if(
                            temp.nodes[next_node-1].edges.begin(),
                            temp.nodes[next_node-1].edges.end(),
                            [&count_lines](AdjEntry& in){return count_lines+1==std::get<0>(in);}
                        ) != temp.nodes[next_node-1].edges.end() // end=not exist
                    );
#endif
                } else {
                    // invalid nodes
                    SFLCHECK(count_lines+1!=0 && next_node!=0)
                    SFLCHECK(count_lines+1!=next_node)
                    SFLCHECK(next_node<=temp.n())
                    current_position = temp.deg(count_lines+1)+1;
                    counter_position = temp.deg(next_node)+1;
                    //std::cerr << current_node << " " << next_node << " "  << current_position << " " << counter_position << std::endl;
                    temp.nodes[count_lines].edges.push_back(AdjEntry(next_node, counter_position));
                    temp.nodes[next_node-1].edges.push_back(AdjEntry(count_lines+1, current_position));
                }
            }
            switch(tempc){
                case '\n':
                case -1:
                    count_lines++;
                case '\r':
                case ' ':
                    break;
                default:
                    number.push_back(tempc);
                    break;
            }
        }
        return temp;
    }

    //! Copy SFLGraph
    /*! \param in SFLGraph to copy
        \return SFLGraph or fails
    */
    static SFLGraph copy_from(const SFLGraph &in){
        SFLGraph temp(in.n()); // has n information now
        SFL_POS_SIZE current_edge, degree;
        for(SFL_ID_SIZE current_node=1; current_node<=temp.n(); current_node++){
            degree = in.deg(current_node);
            for (SFL_POS_SIZE current_edge=1; current_edge<=degree; current_edge++){
                temp.nodes[current_node-1].edges.push_back(in.mate(current_node, current_edge));
            }
        }
        return temp;
    }


#ifdef USE_BOOST
    //! Constructs graph from dot file
    /*! \param in input stream
        \return NULL or SFLGraph
        Only available with boost
    */
    static SFLGraph create_from_dot(std::istream &in){  // untested
        size_t max_size=0;
        SFL_ID_SIZE temp_node_id;
        SFLCHECK (in.good())
        SFLBoostGraph bgraph;
        boost::dynamic_properties dp;
        boost::property_map<SFLBoostGraph, boost::vertex_name_t>::type name = boost::get(boost::vertex_name, bgraph);
        dp.property("node_id",name);
        SFLCHECK(boost::read_graphviz(in, bgraph, dp))

        auto index = boost::get(boost::vertex_name_t(), bgraph);

        boost::graph_traits<SFLBoostGraph>::vertex_iterator vi, last_vertex;
        boost::tie(vi, last_vertex) = boost::vertices(bgraph);
        for (; vi != last_vertex; vi++) {
            temp_node_id = boost::lexical_cast<SFL_ID_SIZE>(index[*vi]);
            if (temp_node_id==0){
                std::cerr << "node is 0" << std::endl;
                throw;
            }
            if(temp_node_id>max_size)
                max_size = temp_node_id;
        }

        SFLGraph temp(max_size);
        SFL_POS_SIZE current_position, counter_position;
        SFL_ID_SIZE current_node, next_node;

        boost::graph_traits<SFLBoostGraph>::edge_iterator _edge, _edge_end;
        boost::tie(_edge, _edge_end) = boost::edges(bgraph);
        for (; _edge != _edge_end; _edge++) {
            current_node = boost::lexical_cast<SFL_ID_SIZE>(index[boost::source(*_edge, bgraph)]);
            next_node = boost::lexical_cast<SFL_ID_SIZE>(index[boost::target(*_edge, bgraph)]);
            current_position = temp.deg(current_node)+1;
            counter_position = temp.deg(next_node)+1;
            //std::cerr << current_node << " " << next_node << " "  << current_position << " " << counter_position << std::endl;
            temp.nodes[current_node-1].edges.push_back(AdjEntry(next_node, counter_position));
            temp.nodes[next_node-1].edges.push_back(AdjEntry(current_node, current_position));
        }
        return temp;
    }
#endif
    //! number of nodes
    /*! \return number of nodes
    */
    virtual SFL_ID_SIZE n() const;
    //! degree of node
    /*! \param nodeid id of node
        \return degree of node
        Return the degree (=amount of edges) of a node
    */
    virtual SFL_POS_SIZE deg(SFL_ID_SIZE nodeid) const;
    //! follow edge
    /*! \param nodeid id of node
        \param edge_position edge position
        \return corresponding nodeid
    */
    virtual SFL_ID_SIZE head(SFL_ID_SIZE nodeid, SFL_POS_SIZE edge_position) const;
    //! mate operation
    /*! \param nodeid id of node
        \param edge_position edge position
        \return mate operation
    */
    virtual AdjEntry mate(SFL_ID_SIZE nodeid, SFL_POS_SIZE edge_position) const;
    //! get node to nodeid
    sfl_node &node(SFL_ID_SIZE nodeid);

    //! iterator to begin of nodes
    /*! iterator return degrees of node
    */
    DegIterator begin_deg() const{
        return DegIterator(nodes.begin());
    }

    //! iterator to end of nodes array
    /*! iterator return degrees of node
    */
    DegIterator end_deg() const{
        return DegIterator(nodes.end());
    }
};


#endif
