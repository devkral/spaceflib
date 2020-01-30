#define CATCH_CONFIG_RUNNER
#define CLARA_CONFIG_MAIN
#include "thirdparty/Catch2/single_include/catch2/catch.hpp"
#include "test_main.hpp"

//planar,outerplanar, edges: 9, nodes:9, ZHK: 2, 2ZHK: 5, cut:[1,2,3]
const SFL_ID_SIZE graph1_edges[]={1,2, 2,3, 3,4, 4,5, 5,6, 6,3, 3,5, 7,8, 9,1};
//planar,outerplanar, edges: 11, nodes:9, ZHK: 2, 2ZHK: 2, cut:[]
const SFL_ID_SIZE graph2_edges[]={1,2, 1,3, 7,9, 9,3, 9,1, 2,7, 4,6, 4,8, 5,6, 5,8, 5,4};
//planar, edges: 20, nodes:16, ZHK: 1, 2ZHK: 3, cut:[4,6,8,10]
const SFL_ID_SIZE graph3_edges[]={3,6, 3,2, 2,15, 15,13, 6,12, 12,13, 2,1, 1,14, 13,16,
14,16, 14,10, 10,8, 8,9, 11,8, 11,9, 5,4, 7,4, 7,5, 4,6, 10,6};

int main( int argc, char* argv[] ) {
    return Catch::Session().run( argc, argv );
}
