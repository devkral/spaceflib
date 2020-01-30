#include "../src/fgraph.hpp"
#include "../src/graph.hpp"
#include "../src/commondefinitions.h"
#include "../src/fgraph.hpp"
#include "thirdparty/Catch2/include/external/clara.hpp"

#include <iostream>
#include <memory>
#include <cstdlib>
#include <fstream>
#include <string>


int main( int argc, char* argv[] ) {
    std::shared_ptr<SFLGraph> graph;
    std::string graphpath="";
    uint64_t vertex=1;
    uint64_t removed_vertex=0;
    bool firstprinted=false;
    bool printgraph=false;
    bool help=false;
    bool fewoutput=false;
    auto cli = Catch::clara::Opt( vertex, "vertexid" )
    ["-n"]["--vertex"]
    ("vertex to use")
    | Catch::clara::Opt( removed_vertex, "removed_vertex" )
    ["-r"]["--remove"]
    ("Remove a vertex from graph experimental!!")
    | Catch::clara::Opt( fewoutput, "fewoutput" )
    ["-f"]["--few"]
    ("Reduce output")
    | Catch::clara::Opt( printgraph, "printgraph" )
    ["-p"]["--print"]
    ("print graph")
    | Catch::clara::Help( help )
    ["-h"]["--help"]
    ("help") // help
    | Catch::clara::Arg( graphpath, "graph" )
    ("graph to use, "" for test graph");

    auto result = cli.parse( Catch::clara::Args( argc, argv ) );
    if( !result || help ){
        std::cout << cli << std::endl;
        return 1;
    }

    // global setup...
    if (graphpath=="") {
        std::cout << "No graph given, fallback to intern" << std::endl;
        SFL_ID_SIZE edges[] = {1,2, 2,3, 3,4, 3,5, 1,3, 1,5, 5,4, 1,6, 8,9, 9,10, 10,8};
        // +1 unconnected node before 3 node twice cc, +1 unconnected node after
        graph = std::make_shared<SFLGraph>(SFLGraph::create(11, edges, 11));
    } else{
        std::ifstream filestrm;
        filestrm.open(graphpath, std::ifstream::in);
        if(!filestrm.is_open()){
            std::cerr << "invalid file" << std::endl;
            return 1;
        }
#ifdef USE_BOOST
        if (graphpath.find(".dot", graphpath.size()-5)!=-1){
            graph = std::make_shared<SFLGraph>(SFLGraph::create_from_dot(filestrm));
        } else
#endif
        {
            graph = std::make_shared<SFLGraph>(SFLGraph::create_from_adj(filestrm));
        }
        filestrm.close();
    }
    RSBitmap removed = null_bitmap.copy();
    if (removed_vertex>0){
        removed = RSBitmap(graph->n());
        removed.set(removed_vertex, true);
        std::cout << "experimental mode active" << std::endl;
    }

    if (printgraph){
        std::cout << "------------------------- graph --------------------------------" << std::endl;
        SFL_ID_SIZE amount_nodes=graph->n();
        for(SFL_ID_SIZE node=1; node<=amount_nodes; node++){
            if (removed.get(node))
                continue;
            SFL_ID_SIZE amount_edges=graph->deg(node);
            if (amount_edges==0){
                std::cout << "Node " << node << std::endl;
            } else {
                std::cout << "Node " << node << ": ";
            }
            for(SFL_POS_SIZE edge=1; edge<=amount_edges; edge++){
                if (edge==amount_edges){
                    std::cout << graph->head(node, edge) << std::endl;
                } else {
                    std::cout << graph->head(node, edge) << ", ";
                }
            }
        }
    }

    SFLGraph* helpergraph = graph.get();

    annotated_edges_t annotated = annotate_edges(*graph);
    if (removed.ones()>0){
        update_edges(*graph, annotated, removed);
    }
    int64_t level=0;
    if (!fewoutput){
        std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, SFL_ID_SIZE, uint8_t)> climb_down = [&level, &annotated, &removed](SFL_ID_SIZE last, SFL_POS_SIZE edge, SFL_ID_SIZE next, uint8_t color) {
            if (removed.get(next))
                return false;
            level = level+1;
            for (uint64_t levelc=0; levelc<level; levelc++)
                std::cout << " ";
            std::cout << "climb down from: " << last <<" to: " << next << ", color: " << (int)color << ", mark: " << annotated.get<edge_marks>(annotated.get_pos(last, edge)) << std::endl;
            level = level+1;
            return true;
        };
        std::function<void (SFL_ID_SIZE, SFL_ID_SIZE, uint8_t)> climb_up = [&level, &helpergraph, &annotated, &removed](SFL_ID_SIZE next, SFL_ID_SIZE last, uint8_t color) {
            if (removed.get(last))
                return;
            level = level-1;
            for (uint64_t levelc=0; levelc<level; levelc++)
                std::cout << " ";
            level = level-1;
            std::cout << "climb up from: " << last << " to: " << next << ", color: " << (int)color << ", mark: " << annotated.get<edge_marks>(annotated.get_pos(last, find_pos_for_id(*helpergraph, last, next))) << std::endl;
        };
        std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, bool)> pre_processing = [&level, &removed](SFL_ID_SIZE node, SFL_POS_SIZE size, bool is_root) {
            if (removed.get(node))
                return false;
            for (uint64_t levelc=0; levelc<level; levelc++)
                std::cout << " ";
            std::cout << "pre process: " << node << " size: " << size << std::endl;
            return true;
        };
        std::function<void (SFL_ID_SIZE, SFL_POS_SIZE)> post_processing = [&level, &removed](SFL_ID_SIZE node, SFL_POS_SIZE size) {
            if (removed.get(node))
                return;
            for (uint64_t levelc=0; levelc<level; levelc++)
                std::cout << " ";
            std::cout << "post process: " << node << " size: " << size << std::endl;
        };
        if (!removed.get(vertex)){
            std::cout << "------------------------- dfs node --------------------------------" << std::endl;
            // O(n) dfs (without parents)
            dfs(*graph, vertex, climb_down, climb_up, pre_processing, post_processing);
            std::cout << "n: " << graph->n() << " last level: " << level << std::endl;
        }
        level = 0;
        std::cout << "------------------------- dfs graph --------------------------------" << std::endl;
        if(annotated.all_empty()>0 || removed.ones()>0){
            // O(n) dfs (without parents)
            dfs(*graph, 0, climb_down, climb_up, pre_processing, post_processing);
            std::cout << "n: " << graph->n() << " last level: " << level << std::endl;
            level = 0;
        } else {
            std::cout << "skip" << std::endl;
        }
    }
    {
        std::cout << "------------------------- cutvertices --------------------------------" << std::endl;
        // shared ptr would keep reference too long
        auto& cutvertices_ret = cutvertices(*graph, annotated, removed);
        std::cout << "cutvertices: ";
        firstprinted=false;
        for(uint64_t rank=0; rank<cutvertices_ret.ones(); rank++){
            /** if (fewoutput && rank > 40000){
                std::cout << ", ...";
                break;
            }*/
            if (firstprinted)
                std::cout << ", ";
            std::cout << cutvertices_ret.select(rank);
            firstprinted = true;
        }
        std::cout << std::endl;
    }

    {
        std::cout << "-------------------- biconnected components --------------------------------" << std::endl;
        firstprinted=false;
        uint64_t counter_nodes=0;
        std::function<void (SFL_ID_SIZE, bool)> output_func = [&level, &firstprinted, &counter_nodes, &fewoutput](SFL_ID_SIZE node, bool new_component) {
            if (new_component){
                if(firstprinted && counter_nodes<100)
                    std::cout << std::endl << std::endl;
                firstprinted=false;
                level++;
                counter_nodes=0;
                std::cout << "component Nr. " << level << ": ";
            }
            if (fewoutput){
                if(counter_nodes == 100){
                    std::cout << ", ..." << std::endl << std::endl;
                    counter_nodes++;
                    return;
                } else if(counter_nodes>100){
                    counter_nodes++;
                    return;
                }
            }
            if (firstprinted)
                std::cout << ", ";

            std::cout << node;
            firstprinted = true;
            counter_nodes++;
        };
        assert(std::get<edges_parent>(annotated.arrays).is_static());
        assert(std::get<edges_backlink>(annotated.arrays).is_static());
        assert(std::get<edge_marks>(annotated.arrays).is_static());
        biconnected_components(*graph, annotated, output_func, removed);
        std::cout << std::endl;
        level = 0;
    }
    if (graphpath=="") {
        std::cout << "Intern test graph finished. Use -h, --help for help" << std::endl;
    }
    return 0;
}
