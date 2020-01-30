/*! \file graph.hpp
    \author Alexander Kaftan
    \brief Graph routines
*/

#ifndef spacefgraph
#define spacefgraph

#include "commondefinitions.h"
// c++ headers

#include "fgraph.hpp"
#include "misc.hpp"
#include <tuple>
#include <vector>
#include <functional>

//! color state of nodes
/*! \enum node_color_state
*/
typedef enum{
    white=0, // not-visited
    gray=1, // visited 1
    darkgray=2, // visited 2
    black=3 // complete
} node_color_state;

//! state of tree nodes with ZFS algorithm
/*! \enum dfs_tree_state

*/
typedef enum{
    unmarked=0,
    half_marked=1,
    full_marked=2,
} dfs_tree_state;

//! position of various arrays in annotated_edges_t
/*! \enum annotated_edges_enum
    0 (edges_parent): boolean array with marked parent positions
    1 (edges_backlink): backlinks
    2 (edges_marks): marks of edges (multibit)
*/
typedef enum{
    edges_parent=0,
    edges_backlink=1,
    edge_marks=2
} annotated_edges_enum;

//! annotated edges object (SegmentedBitmap)
/*!
    annotates all edges with following attributes:
    0 (edges_parent): boolean array with marked parent positions
    1 (edges_backlink): backlinks
    2 (edges_marks): marks of edges (multibit)

    Edges are accessed via SegmentedBitmap methods:
    segment = node
    pos = edge position

*/
typedef SegmentedArray<bool, bool, array_multi_bit<2>> annotated_edges_t;
//! edge marks
typedef ConstTimeArray<array_multi_bit<2>> marked_edges_t;
//! parent edges
typedef ConstTimeArray<bool> parent_edges_t;
//! placeholder bitmap returning 0, size is of a normal bitmap in this size
const RSBitmap null_bitmap(1);

//! placeholder for function parameter in dfs functions
bool dfs_placeholder(...);

//! depth-first-search over full graph or node
/*! \param graph Graph object
    \param vertex start point for depth first search (0=search full graph)
    \param preexplore prexplore hook, takes current node, current edge, next node, color next node
    \param postexplore postexplore hook, takes parent node, current node, color current node
    \param preprocess preprocess hook, returning false stops processing, executed before node is explored, takes current node, size, isroot
    \param postprocess postprocess hook, executed after node is fully explored, takes current node, size
*/
void dfs(const SFLGraph &graph, const SFL_ID_SIZE vertex=0,
    const std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, SFL_ID_SIZE, uint8_t)> &preexplore=dfs_placeholder,
    const std::function<void (SFL_ID_SIZE, SFL_ID_SIZE, uint8_t)> &postexplore=dfs_placeholder,
    const std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, bool)> &preprocess=dfs_placeholder,
    const std::function<void (SFL_ID_SIZE, SFL_POS_SIZE)> &postprocess=dfs_placeholder);

//! depth-first-search over full graph or node (faster version)
/*! \param graph Graph object
    \param annotated annotated array containing correct parents till current node (parents can be updated in preexplore, see mark_edges for an example)
    \param vertex start point for depth first search (0=search full graph)
    \param preexplore prexplore hook, takes current node, current edge, next node, color next node
    \param postexplore postexplore hook, takes parent node, current node, color current node
    \param preprocess preprocess hook, returning false stops processing, executed before node is explored, takes current node, size, isroot
    \param postprocess postprocess hook, executed after node is fully explored, takes current node, size
*/
void dfs(const SFLGraph &graph, const annotated_edges_t &annotated, const SFL_ID_SIZE vertex=0,
    const std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, SFL_ID_SIZE, uint8_t)> &preexplore=dfs_placeholder,
    const std::function<void (SFL_ID_SIZE, SFL_ID_SIZE, uint8_t)> &postexplore=dfs_placeholder,
    const std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, bool)> &preprocess=dfs_placeholder,
    const std::function<void (SFL_ID_SIZE, SFL_POS_SIZE)> &postprocess=dfs_placeholder);

//! find node in adjacencearray of connected node
/*! \param graph graph object
    \param base base node in which array the node should be searched
    \param search searched node
    \return 1-based position or 0
*/
SFL_POS_SIZE find_pos_for_id(const SFLGraph &graph, const SFL_ID_SIZE base, const SFL_ID_SIZE search){
    for(SFL_POS_SIZE counter=1; counter<=graph.deg(base); counter++){
        if (graph.head(base, counter)==search)
            return counter;
    }
    return 0;
}

//! initialize array with edge marks
/*! \param graph Graph object
    \param annotated cached annotated_edges result object
    \param update_object object to update_object
*/
void mark_edges(const SFLGraph &graph, const annotated_edges_t &annotated, marked_edges_t &update_object);

//! update annotated_edges
/*! \param graph Graph object
    \param annotated cached annotated_edges to update
    \param removednodes RSBitmap with removed nodes
*/
void update_edges(const SFLGraph &graph, annotated_edges_t &annotated, const RSBitmap &removednodes=null_bitmap);

//! generate annotate_edges_t from a depth-first-search for backlinks over the full graph
/*! \param graph Graph object
    \param removednodes RSBitmap with removed nodes
    \return annotate_edges_t
*/
annotated_edges_t annotate_edges(const SFLGraph &graph, const RSBitmap &removednodes=null_bitmap);

//! cutvertices
/*! \param graph Graph object
    \param annotated cached annotated_edges result object
    \param removednodes RSBitmap with removed nodes (requires updated annotated_edges) (experimental!)
    \return pointer to RSBitmap with cutvertices

*/
const RSBitmap cutvertices(const SFLGraph &graph, const annotated_edges_t &annotated, const RSBitmap &removednodes=null_bitmap);
//! Bi-Connected Components
/*! \param graph Graph object
    \param annotated cached annotated_edges result object
    \param output_func function which takes node, is it a new component
    \param removednodes RSBitmap with removed nodes (requires updated annotated_edges) (experimental!)
*/
void biconnected_components(const SFLGraph &graph, const annotated_edges_t &annotated, std::function<void (SFL_ID_SIZE, bool)>output_func, const RSBitmap &removednodes=null_bitmap);
//! Bi-Connected Components
/*! \param graph Graph object
    \param annotated cached annotated_edges result object
    \param removednodes RSBitmap with removed nodes (requires updated annotated_edges) (experimental!)
    \return vector with RSBitmap with twice connected components
*/
const std::vector<std::vector<SFL_ID_SIZE>> biconnected_components(const SFLGraph &graph, const annotated_edges_t &annotated, const RSBitmap &removednodes=null_bitmap);


#endif
