
#include "thirdparty/Catch2/include/catch.hpp"

#include "../src/misc.hpp"
#include "../src/fgraph.hpp"
#include "test_main.hpp"

#include <string>

TEST_CASE( "ConstTimeArray Misc Size", "[MiscSize]") {
    const SFLGraph& local_graph = SFLGraph::create(9, graph1_edges, 9);
    ConstTimeArray<uint32_t> t_array1(local_graph.n(), 1);
    t_array1.set(local_graph.n(), 32);
    ConstTimeArray<bool> t_array2(local_graph.n()*64);
    t_array2.set(local_graph.n(), true);
    ConstTimeArray<array_multi_bit<5>> t_array3(local_graph.n()*64);
    t_array3.set(local_graph.n(), 16);
    REQUIRE(t_array3.get(local_graph.n())==16);
    REQUIRE(t_array2.select(0)==local_graph.n());
    REQUIRE(t_array3.select(0)==local_graph.n());
    REQUIRE(t_array2.rank(local_graph.n()+1)==1);
    REQUIRE(t_array3.rank(local_graph.n()+1)==1);
    REQUIRE(t_array1.get(local_graph.n())==32);
    REQUIRE(t_array2.get(local_graph.n())==true);
    REQUIRE(t_array3.get(local_graph.n())==16);
}

TEST_CASE( "ConstTimeArray pure", "[ArraySize]") {
    std::shared_ptr<SFLGraph> local_graph = std::make_shared<SFLGraph>(SFLGraph::create(9, graph1_edges, 9));
    REQUIRE(local_graph.get()!=0);
    ConstTimeArray<uint32_t> t_array1(local_graph->n(), 1);
    t_array1.set(local_graph->n(), 32);
    REQUIRE(t_array1.get(local_graph->n())==32);
}

TEST_CASE( "RSBitmap Size", "[BitSize]") {
    std::shared_ptr<SFLGraph> local_graph = std::make_shared<SFLGraph>(SFLGraph::create(9, graph1_edges, 9));
    CAPTURE(local_graph->n());
    RSBitmap bit1(local_graph->n()*64);
    CAPTURE(bit1.n());
    REQUIRE(bit1.n()==local_graph->n()*64);
    CAPTURE(bit1.calc_blocks(bit1.n()));
    CAPTURE(bit1._get_size_slate());
    bit1.set(local_graph->n(), true);
    REQUIRE(bit1.select(0)==local_graph->n());
    REQUIRE(bit1.get(local_graph->n())==true);/**
    for(uint64_t c=1;c<bit1.n(); c++){
        CAPTURE(c);
        CAPTURE(bit1.find_slate(c));
        bit1.set(c, true);
        CAPTURE(bit1.ones());
        REQUIRE(bit1.get(c) == true);
        REQUIRE(bit1.get_n(c, 1) == 1);
        REQUIRE(bit1.rank(c) == c-1);
        REQUIRE(bit1.select(bit1.rank(c)) == c);
    }*/

    RSBitmap bit2(local_graph->n()*64*64);
    bit2.set_n(local_graph->n(), 64, 0xADF1EAEEull);
    REQUIRE(bit2.get_n(local_graph->n(), 64) == 0xADF1EAEEull);
    bit2.set_n(local_graph->n()*63, 64, 0xF1FBF1FAull);
    REQUIRE(bit2.get_n(local_graph->n()*63, 64) == 0xF1FBF1FAull);
    RSBitmap bit3(local_graph->n()*64*5);
    bit3.set_n(local_graph->n(), 5, 16);
    REQUIRE(bit3.get_n(local_graph->n(), 5) == 16);
    bit3.set_n(local_graph->n()*63, 5, 17);
    REQUIRE(bit3.get_n(local_graph->n()*63, 5) ==  17);
}
