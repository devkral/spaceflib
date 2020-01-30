#include "fgraph.hpp"
#include <iostream>

SFL_ID_SIZE SFLGraph::n() const{
    return nodes.size();
}
SFL_POS_SIZE SFLGraph::deg(SFL_ID_SIZE nodeid) const{
    if (nodeid == 0 || nodeid > n())
        return 0;
    return nodes[nodeid-1].edges.size();
}
SFL_ID_SIZE SFLGraph::head(SFL_ID_SIZE nodeid, SFL_POS_SIZE edge_position) const{
    if (nodeid == 0 || nodeid > n() || edge_position==0 || edge_position > nodes[nodeid-1].edges.size())
        return 0;
    return std::get<0>(nodes[nodeid-1].edges[edge_position-1]);
}

sfl_node &SFLGraph::node(SFL_ID_SIZE nodeid){
    SFLCHECK (nodeid > 0 && nodeid <= n())
    return nodes[nodeid-1];
}

AdjEntry SFLGraph::mate(SFL_ID_SIZE nodeid, SFL_POS_SIZE edge_position) const{
    // Mate: input: node, position, output: reverse node, own position
    if (nodeid == 0 || nodeid > n() || edge_position==0 || edge_position > nodes[nodeid-1].edges.size())
        return AdjEntry(0,0);
    return nodes[nodeid-1].edges[edge_position-1];
}
