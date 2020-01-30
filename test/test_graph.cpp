#include "thirdparty/Catch2/include/catch.hpp"


#include "../src/fgraph.hpp"
#include "../src/graph.hpp"
#include "../src/commondefinitions.h"
#include "test_main.hpp"
#include <sstream>
//#include <iostream>

TEST_CASE( "Adjacence loading",  "[SFLGraph][adjancence]") {
    // edges
    SFLGraph graph1_e = SFLGraph::create(9, graph1_edges, 9);

    std::stringstream graph1_stream(graph1_adj_nonew, std::ios::in);
    // adjacence
    SFLGraph graph1_a = SFLGraph::create_from_adj(graph1_stream);
    SFL_POS_SIZE deg;
    bool head_found=false;
    SFL_ID_SIZE head, head_adj;
    REQUIRE(graph1_e.n()==graph1_a.n());
    for (SFL_ID_SIZE node=1; node<=graph1_e.n(); node++){
        CAPTURE(node);
        deg = graph1_e.deg(node);
        REQUIRE(deg==graph1_a.deg(node));
        for (SFL_POS_SIZE edge=1; edge<=deg; edge++){
            head = graph1_e.head(node, edge);
            CAPTURE(head);
            REQUIRE(std::get<0>(graph1_e.mate(node, edge))==head);
            head_found = false;
            // order is not guranteed
            for (SFL_POS_SIZE edge2=1; edge2<=deg; edge2++){
                head_adj = graph1_a.head(node, edge2);
                if (head==head_adj){
                    head_found = true;
                }
                REQUIRE(std::get<0>(graph1_a.mate(node, edge2))==head_adj);
            }
            REQUIRE(head_found);
        }
    }


}

TEST_CASE( "Graph basics",  "[SFLGraph][annotated_edges_t]") {
    // one node more than in graph1_edges
    SFLGraph local_graph = SFLGraph::create(10, graph1_edges, 9);
    SECTION("SFLGraph"){
        REQUIRE(local_graph.node(1).edges.size() == 2);
        REQUIRE(local_graph.deg(1) == 2);
        REQUIRE(local_graph.deg(2) == 2);
        REQUIRE(local_graph.deg(10) == 0);
        REQUIRE(local_graph.head(1, 1) == 2);
        REQUIRE(local_graph.n() == 10);

        annotated_edges_t annotated = annotated_edges_t(local_graph.begin_deg(), local_graph.end_deg(), false, false, array_multi_bit<2>());
        for (size_t node=1; node<=local_graph.n(); node++){
            CAPTURE(node);
            REQUIRE(annotated.segment_size(node) == local_graph.deg(node));
            if (annotated.segment_size(node)>0){
                // set test
                annotated.set<edges_parent>(annotated.get_pos(node, local_graph.deg(node)), true);
                REQUIRE(annotated.get<edges_parent>(annotated.get_pos(node, annotated.segment_size(node))) == true);
                REQUIRE(annotated.select_segment_pos(std::get<edges_parent>(annotated.arrays), node, 1) == local_graph.deg(node));
            }
        }
        // remove bogus parents:
        annotated = std::move(annotate_edges(local_graph));
        REQUIRE(std::get<edges_parent>(annotated.arrays).is_static());
        REQUIRE(std::get<edge_marks>(annotated.arrays).is_static());
        REQUIRE(std::get<edges_backlink>(annotated.arrays).is_static());
        annotated_edges_t annotated2 = annotated.copy();
        RSBitmap tmap(10);
        // remove unmarked node
        tmap.set(10, true);
        update_edges(local_graph, annotated2, tmap);
        for (size_t node=1; node<=local_graph.n(); node++){
            CAPTURE(node);
            CAPTURE(annotated.select_segment_pos(std::get<edges_parent>(annotated.arrays), node, 1));
            CAPTURE(annotated.select_segment_pos(std::get<edges_parent>(annotated2.arrays), node, 1));
            if (annotated.segment_size(node)>0){
                REQUIRE(annotated.select_segment_pos(std::get<edges_parent>(annotated.arrays), node, 1)==annotated.select_segment_pos(std::get<edges_parent>(annotated2.arrays), node, 1));
            }
        }
    }
}
TEST_CASE( "Depth first search", "[dfs]" ) {
    std::shared_ptr<SFLGraph> local_graph;
    int64_t stack_level=0;
    SFL_ID_SIZE down_last=0, down_next=0;
    SFL_ID_SIZE up_last=0, up_next=0;
    std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, SFL_ID_SIZE, uint8_t)> climb_down = [&down_last, &down_next, &up_last, &up_next, &stack_level, &local_graph](SFL_ID_SIZE last, SFL_POS_SIZE edge, SFL_ID_SIZE next, uint8_t color) {
        CAPTURE(last);
        CAPTURE(edge);
        CAPTURE(next);
        REQUIRE(local_graph->deg(last)>0);
        SFL_ID_SIZE next_id_mate;
        SFL_POS_SIZE counter_edge;
        std::tie(next_id_mate, counter_edge) = local_graph->mate(last, edge);
        CAPTURE(counter_edge);
        REQUIRE(next_id_mate==next);
        REQUIRE(local_graph->head(next, counter_edge)==last);
        REQUIRE(local_graph->deg(next)>0);

        // stuck
        REQUIRE(down_last!=last);
        REQUIRE(down_next!=next);
        down_last = last;
        down_next = next;
        up_last = 0;
        up_next = 0;
        stack_level++;
        return true;
    };
    std::function<void (SFL_ID_SIZE, SFL_ID_SIZE, uint8_t)> climb_up = [&down_last, &down_next, &up_last, &up_next, &stack_level](SFL_ID_SIZE next, SFL_ID_SIZE last, uint8_t color) {
        // test if stuck
        REQUIRE(up_next!=next);
        REQUIRE(up_last!=last);
        up_next = next;
        up_last = last;
        down_last = 0;
        down_next = 0;
        stack_level--;
        REQUIRE(stack_level>=0);
    };
    std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, bool)> pre_processing = [&stack_level](SFL_ID_SIZE node, SFL_POS_SIZE size, bool is_root) {
        if (stack_level==0)
            REQUIRE(is_root);
        if (is_root)
            REQUIRE(stack_level==0);
        return true;
    };
    local_graph=std::make_shared<SFLGraph>(SFLGraph::create(9, graph1_edges, 9));
    dfs(*local_graph, 0, climb_down, climb_up, pre_processing);
    local_graph=std::make_shared<SFLGraph>(SFLGraph::create(9, graph2_edges, 11));
    dfs(*local_graph, 0, climb_down, climb_up, pre_processing);
    local_graph=std::make_shared<SFLGraph>(SFLGraph::create(16, graph3_edges, 20));
    dfs(*local_graph, 0, climb_down, climb_up, pre_processing);
#ifdef USE_BOOST
    std::stringstream oldgraph_stream(oldgraph, std::ios::in);

    local_graph=std::make_shared<SFLGraph>(SFLGraph::create_from_dot(oldgraph_stream));
    dfs(*local_graph, 0, climb_down, climb_up, pre_processing);
#endif
}
TEST_CASE( "Annotation algorithms", "[annotate_edges][annotate_links]") {
    SFLGraph local_graph = SFLGraph::create(9, graph1_edges, 9);
    annotated_edges_t annotated = annotate_edges(local_graph);
    RSBitmap test_single(annotated.size());

    CAPTURE( annotated.size() );
    std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, SFL_ID_SIZE, uint8_t)> climb_down = [&annotated, &local_graph, &test_single](SFL_ID_SIZE last, SFL_POS_SIZE edge, SFL_ID_SIZE next, uint8_t color) {
        CAPTURE(last);
        CAPTURE(edge);
        CAPTURE(next);
        CHECK_FALSE(test_single.get(annotated.get_pos(last, edge)));
        test_single.set(annotated.get_pos(last, edge), true);

        AdjEntry mate_ = local_graph.mate(last, edge);
        // too horrible broken, crash
        REQUIRE(std::get<0>(mate_)==next);
        // mate operation is broken if not working
        CHECK(local_graph.head(next, std::get<1>(mate_))==last);

        SFL_POS_SIZE counterpos = std::get<1>(mate_);
        CHECK(annotated.get<edge_marks>(annotated.get_pos(last, edge))==annotated.get<edge_marks>(annotated.get_pos(next, counterpos)));
        if (color==white)
            CHECK(annotated.get<edges_parent>(annotated.get_pos(next, counterpos)));
        if (annotated.get<edges_parent>(annotated.get_pos(next, counterpos)))
            CHECK_FALSE(annotated.get<edges_parent>(annotated.get_pos(last, edge)));
        return true;
    };
    std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, bool)> pre_processing = [&annotated](SFL_ID_SIZE node, SFL_POS_SIZE size, bool is_root) {
        if (!is_root && annotated.segment_size(node)>0)
            CHECK(annotated.select_segment_pos(std::get<edges_parent>(annotated.arrays), node, 1) != 0);
        if (is_root && annotated.segment_size(node)>0)
            CHECK(annotated.select_segment_pos(std::get<edges_parent>(annotated.arrays), node, 1) == 0);
        return true;
    };
    dfs(local_graph, 0, climb_down, dfs_placeholder, pre_processing);
    REQUIRE(test_single.ones()+std::get<edges_parent>(annotated.arrays).ones()==test_single.n());
}

TEST_CASE( "Cutvertice search", "[cutvertices]") {
    {
        SFLGraph local_graph = SFLGraph::create(9, graph1_edges, 9);
        annotated_edges_t annotated = annotate_edges(local_graph);
        REQUIRE(annotated.segments()==local_graph.n());
        const RSBitmap& cv = cutvertices(local_graph, annotated);
        REQUIRE(cv.is_static()==false);
        REQUIRE(cv.get(1)==true);
        REQUIRE(cv.get(2)==true);
        REQUIRE(cv.get(3)==true);
        REQUIRE(cv.ones()==3);
    }


#ifdef USE_BOOST
    {
        std::stringstream oldgraph_stream(oldgraph, std::ios::in);
        SFLGraph local_graph=SFLGraph::create_from_dot(oldgraph_stream);
        annotated_edges_t annotated = annotate_edges(local_graph);
        REQUIRE(annotated.segments()==local_graph.n());
        const RSBitmap& cv = cutvertices(local_graph, annotated);
        REQUIRE(cv.is_static()==false);
        REQUIRE(cv.get(135)==true);
        REQUIRE(cv.get(143)==true);
        REQUIRE(cv.get(147)==true);
        REQUIRE(cv.get(149)==true);
        REQUIRE(cv.get(151)==true);
        REQUIRE(cv.get(152)==true);
        REQUIRE(cv.get(184)==true);
        REQUIRE(cv.get(200)==true);
        REQUIRE(cv.get(202)==true);
        REQUIRE(cv.get(271)==true);
        REQUIRE(cv.get(335)==true);
        REQUIRE(cv.get(343)==true);
        REQUIRE(cv.get(347)==true);
        REQUIRE(cv.get(349)==true);
        REQUIRE(cv.get(469)==true);
        REQUIRE(cv.get(477)==true);
        REQUIRE(cv.get(481)==true);
        REQUIRE(cv.get(550)==true);
        REQUIRE(cv.get(627)==true);
        REQUIRE(cv.get(702)==true);
        REQUIRE(cv.get(769)==true);
        REQUIRE(cv.get(799)==true);
        REQUIRE(cv.get(863)==true);
        REQUIRE(cv.get(871)==true);
        REQUIRE(cv.get(875)==true);
        REQUIRE(cv.ones()==25);
    }
#endif
    /**std::cout << "cutvertices: ";
    bool firstprinted=false;
    for(size_t counter=1; counter<=cv.n(); counter++){
        if(cv.get(counter)){
            if (firstprinted)
                std::cout << ", ";
            std::cout << counter;
            firstprinted = true;
        }
    }
    std::cout << std::endl;*/

}
TEST_CASE( "Bi-connected component search", "[twice_cc]" ) {
    std::shared_ptr<SFLGraph> local_graph = std::make_shared<SFLGraph>(SFLGraph::create(9, graph1_edges, 9));
    annotated_edges_t annotated = annotate_edges(*local_graph);
    REQUIRE(annotated.segments()==local_graph->n());
    const std::vector<std::vector<SFL_ID_SIZE>>& cc_res = biconnected_components(*local_graph, annotated);


    /**bool firstprinted=false;
    std::cout << "twice connected components: " << std::endl;
    for(size_t component=1; component<=twice_cc_res.size(); component++){
        firstprinted=false;
        std::cout << "component Nr. " << component << ": ";
        for(size_t counter=1; counter<=twice_cc_res.at(component-1).n(); counter++){
            if(twice_cc_res.at(component-1).get(counter)){
                if (firstprinted)
                    std::cout << ", ";
                std::cout << counter;
                firstprinted = true;
            }
        }
        std::cout << std::endl;
    }*/
}
