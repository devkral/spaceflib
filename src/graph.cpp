
#include "graph.hpp"
#include <cmath>
#include <optional>
using namespace std;

bool dfs_placeholder(...){
    return true;
}

void dfs_restore(const SFL_ID_SIZE vertex, const SFLGraph &graph, SpinStack<AdjEntry> &Stack, RSBitmap &color, SFL_ID_SIZE restore, bool restore_step, const uint64_t q){
    SFL_ID_SIZE cur_node, next_node, parent=0;
    SFL_POS_SIZE cur_edge;
    SFL_POS_SIZE cached_deg;
    // for debug, use in asserts
    SFL_ID_SIZE graph_n = graph.n();
    assert (vertex > 0 && vertex <= graph_n);
    Stack.push_top(AdjEntry(vertex, 1));
    while(!Stack.empty()){
        AdjEntry tup_current = Stack.pop();
        std::tie(cur_node, cur_edge) = tup_current;
        cached_deg = graph.deg(cur_node);
        // restore_step 0
        if (!restore_step){
            color.set_n(cur_node, 2, gray);
        } else {
            // restore_step 1
            color.set_n(cur_node, 2, darkgray);
        }
        assert(cached_deg>0);
        assert(cached_deg<=graph_n);
        if (cur_edge <= cached_deg){
            // extract parent from stack if not root
            if (!Stack.empty())
                parent = std::get<0>(Stack.peek());
            else
                parent = 0;
            AdjEntry tup_next_edge(cur_node, cur_edge+1);
            // after extracting parent, push new edge
            Stack.push_top(tup_next_edge);
            next_node = graph.head(cur_node, cur_edge);
            assert(next_node > 0);
            assert(next_node<=graph_n);
            if (next_node==restore)
                return;
            // ignore parents for the cost of one extra Stack element
            // requires always 1 parent on the stack if not root
            if (parent!=next_node){
                if(!restore_step && color.get_n(next_node, 2) == darkgray){
                    Stack.push_top(AdjEntry(next_node, 1));
                } else if (restore_step && color.get_n(next_node, 2) == gray){
                    Stack.push_top(AdjEntry(next_node, 1));
                }
            }
        } else {
            return;
        }
        // +1 for having always a parent
        if (Stack.size()>2*q+1){
            Stack.drop_front(q);
        }
    }
}


void dfs_base(const SFL_ID_SIZE vertex, const SFLGraph &graph, SpinStack<AdjEntry> &Stack, RSBitmap &color, bool restore_step, const uint64_t q,
    const std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, SFL_ID_SIZE, uint8_t)> &preexplore,
    const std::function<void (SFL_ID_SIZE, SFL_ID_SIZE, uint8_t)> &postexplore,
    const std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, bool)> &preprocess,
    const std::function<void (SFL_ID_SIZE, SFL_POS_SIZE)> &postprocess){
    SFL_ID_SIZE cur_node, next_node, parent=0;
    SFL_POS_SIZE cur_edge;
    SFL_POS_SIZE cached_deg;
    // for debug, use in asserts
    SFL_ID_SIZE graph_n = graph.n();
    assert(vertex > 0 && vertex <= graph_n);
    Stack.push_top(AdjEntry(vertex, 1));
    bool firstrun=true;
    while(!Stack.empty()){
        AdjEntry tup_current = Stack.pop();
        // second case postexplore is called, needs current information
        if (!firstrun && color.get_n(cur_node, 2)==black)
            postexplore(std::get<0>(tup_current), cur_node, color.get_n(cur_node, 2));
        std::tie(cur_node, cur_edge) = tup_current;
        cached_deg = graph.deg(cur_node);
        if (color.get_n(cur_node, 2)==white){
            // preprocess false stops further processing of node
            if(!preprocess(cur_node, cached_deg, vertex==cur_node)){
                color.set_n(cur_node, 2, black);
                continue;
            }
        }
        // restore_step 0
        if (!restore_step){
            color.set_n(cur_node, 2, gray);
        } else {
            // restore_step 1
            color.set_n(cur_node, 2, darkgray);
        }
        assert(cached_deg>0);
        assert(cached_deg<=graph_n);
        if (cur_edge <= cached_deg){
            // extract parent from stack if not root
            if (!Stack.empty())
                parent = std::get<0>(Stack.peek());
            else
                parent = 0;
            AdjEntry tup_next_edge(cur_node, cur_edge+1);
            // after extracting parent, push new edge
            Stack.push_top(tup_next_edge);
            next_node = graph.head(cur_node, cur_edge);
            assert(next_node > 0);
            assert(next_node<=graph_n);
            // ignore parents for the cost of one extra Stack element
            // requires always 1 parent on the stack if not root
            if (parent!=next_node){
                if(!preexplore(cur_node, cur_edge, next_node, color.get_n(next_node, 2))){
                    color.set_n(next_node, 2, black);
                }
                if (color.get_n(next_node, 2) == white)
                    Stack.push_top(AdjEntry(next_node, 1));
                else {
                    // cannot explore, go direct to postexplore
                    postexplore(cur_node, next_node, color.get_n(next_node, 2));
                }
            }
        } else {
            color.set_n(cur_node, 2, black);
            // stack never runs empty if node doesn't turn black (always+1)
            // reserve+1 for having always a parent
            if (Stack.size() <= 1 && color.get_n(vertex, 2) != black) {
                // clean stack (always parent)
                if (!Stack.empty())
                    Stack.pop();
                // restore process should use different gray and the color should switch for the main routine
                restore_step = !restore_step;
                dfs_restore(vertex, graph, Stack, color, cur_node, restore_step, q);
            }
            postprocess(cur_node, cached_deg);
        }

        // +1 for having always a parent
        if (Stack.size()>2*q+1){
            Stack.drop_front(q);
        }
        firstrun=false;
    }
}

void dfs_base_parents(const SFL_ID_SIZE vertex, const SFLGraph &graph, std::optional<AdjEntry> &Element, const annotated_edges_t& annotated, RSBitmap &color,
    const std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, SFL_ID_SIZE, uint8_t)> &preexplore,
    const std::function<void (SFL_ID_SIZE, SFL_ID_SIZE, uint8_t)> &postexplore,
    const std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, bool)> &preprocess,
    const std::function<void (SFL_ID_SIZE, SFL_POS_SIZE)> &postprocess){
    SFL_ID_SIZE cur_node, next_node,parent=0;
    SFL_POS_SIZE cur_edge, parent_edge=0;
    SFL_POS_SIZE cached_deg;
    // for debug, use in asserts
    SFL_ID_SIZE graph_n = graph.n();
    assert(vertex > 0 && vertex <= graph_n);
    Element.emplace(vertex, 1);
    assert(Element);
    bool firstrun=true;
    while(Element.has_value()){
        AdjEntry tup_current = Element.value();
        Element.reset();
        // second case postexplore is called, needs current information
        if (!firstrun && color.get_n(cur_node, 2)==black)
            postexplore(std::get<0>(tup_current), cur_node, color.get_n(cur_node, 2));
        std::tie(cur_node, cur_edge) = tup_current;
        cached_deg = graph.deg(cur_node);
        if (color.get_n(cur_node, 2)==white){
            // preprocess false stops further processing of node
            if(!preprocess(cur_node, cached_deg, vertex==cur_node)){
                color.set_n(cur_node, 2, black);
                continue;
            }
        }
        // restore_step 0
        color.set_n(cur_node, 2, gray);

        assert(cached_deg>0);
        assert(cached_deg<=graph_n);
        if (cur_edge <= cached_deg){
            // extract edge to parent from annotated
            parent_edge = annotated.select_segment_pos(std::get<edges_parent>(annotated.arrays), cur_node, 1);
            AdjEntry tup_next_edge(cur_node, cur_edge+1);
            // push new edge
            Element = tup_next_edge;
            // compare edge to save one graph call
            if (parent_edge==0 || parent_edge!=cur_edge){
                // reorder to speedup
                next_node = graph.head(cur_node, cur_edge);
                assert(next_node > 0);
                assert(next_node<=graph_n);
                if(!preexplore(cur_node, cur_edge, next_node, color.get_n(next_node, 2))){
                    color.set_n(next_node, 2, black);
                }
                if (color.get_n(next_node, 2) == white){
                    Element.emplace(next_node, 1);
                }
                else {
                    // cannot explore, go direct to postexplore
                    postexplore(cur_node, next_node, color.get_n(next_node, 2));
                }
            }
        } else {
            color.set_n(cur_node, 2, black);
            if (!Element.has_value() && color.get_n(vertex, 2) != black) {
                // extract parent from annotated
                parent_edge = annotated.select_segment_pos(std::get<edges_parent>(annotated.arrays), cur_node, 1);
                if (parent_edge){
                    AdjEntry temp = graph.mate(cur_node, parent_edge);
                    // set to next edge
                    std::get<1>(temp)++;
                    Element = temp;
                }
            }
            postprocess(cur_node, cached_deg);
        }
        firstrun=false;
    }
}

void dfs(const SFLGraph &graph, const SFL_ID_SIZE vertex,
    const std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, SFL_ID_SIZE, uint8_t)> &preexplore,
    const std::function<void (SFL_ID_SIZE, SFL_ID_SIZE, uint8_t)> &postexplore,
    const std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, bool)> &preprocess,
    const std::function<void (SFL_ID_SIZE, SFL_POS_SIZE)> &postprocess){
    // for speedup
    SFL_ID_SIZE graph_n = graph.n();
    uint64_t q = graph_n/log(graph_n);
    SFLCHECK (vertex <= graph_n);
    // 0 is default, = white
    // *2 for block building
    RSBitmap color(graph_n*2);
    // +1 for algorithm, +1 for having always a parent
    SpinStack<AdjEntry> Stack(2+2*q);
    if (vertex == 0){
        for (SFL_ID_SIZE vcount=1; vcount<=graph_n; vcount++){
            // only white nodes
            if (color.get_n(vcount, 2)!=white)
                continue;
            // optimize single nodes
            if (graph.deg(vcount)==0){
                if(preprocess(vcount, 0, true))
                    postprocess(vcount, 0);
                continue;
            }
            dfs_base(vcount, graph, Stack, color, false, q, preexplore, postexplore, preprocess, postprocess);
        }
        return;
    }
    // optimize single nodes
    if (graph.deg(vertex)==0){
        if(preprocess(vertex, 0, true))
            postprocess(vertex, 0);
        return;
    }
    dfs_base(vertex, graph, Stack, color, false, q, preexplore, postexplore, preprocess, postprocess);
}

void dfs(const SFLGraph &graph, const annotated_edges_t& annotated, const SFL_ID_SIZE vertex,
    const std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, SFL_ID_SIZE, uint8_t)> &preexplore,
    const std::function<void (SFL_ID_SIZE, SFL_ID_SIZE, uint8_t)> &postexplore,
    const std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, bool)> &preprocess,
    const std::function<void (SFL_ID_SIZE, SFL_POS_SIZE)> &postprocess){
    // for speedup
    SFL_ID_SIZE graph_n = graph.n();
    SFLCHECK (vertex <= graph_n);
    // 0 is default, = white
    // *2 for block building
    RSBitmap color(graph_n*2);
    // requires 1 Entry
    std::optional<AdjEntry> Element;
    if (vertex == 0){
        for (SFL_ID_SIZE vcount=1; vcount<=graph_n; vcount++){
            // only white nodes
            if (color.get_n(vcount, 2)!=white)
                continue;
            // optimize single nodes
            if (graph.deg(vcount)==0){
                if(preprocess(vcount, 0, true))
                    postprocess(vcount, 0);
                continue;
            }
            dfs_base_parents(vcount, graph, Element, annotated, color, preexplore, postexplore, preprocess, postprocess);
        }
        return;
    }
    // optimize single nodes
    if (graph.deg(vertex)==0){
        if(preprocess(vertex, 0, true))
            postprocess(vertex, 0);
        return;
    }
    dfs_base_parents(vertex, graph, Element, annotated, color, preexplore, postexplore, preprocess, postprocess);
}


void mark_parents(const SFLGraph &graph, const annotated_edges_t &annotated, const parent_edges_t &parents, marked_edges_t &marks, AdjEntry entry,SFL_ID_SIZE stop_node){
    SFL_ID_SIZE parent_id;
    uint8_t last_mark;
    SFL_POS_SIZE edge_to_child_pos, edge_to_parent_pos;
    SFL_ID_SIZE current_node;
    SFL_POS_SIZE initial_edge;
    std::tie(current_node, initial_edge) = entry;

    assert(current_node!=0);
    // complicated stuff, TODO: documentation
    // use select_next_pos to get segment position of parent
    edge_to_parent_pos = annotated.select_segment_pos(parents, current_node, 1);
    // every node reached by explore has a parent, check this
    assert(edge_to_parent_pos!=0);
    // assert not invalid
    assert(graph.head(current_node, edge_to_parent_pos) != stop_node);
    // current_nodes must be marked too
    // find backlink node and edge to this node
    std::tie(parent_id, edge_to_child_pos) = graph.mate(current_node, initial_edge);
    // mark backlink black
    marks.set(annotated.get_pos(current_node, initial_edge), full_marked);
    marks.set(annotated.get_pos(parent_id, edge_to_child_pos), full_marked);
    // find parent node and edge to this node
    std::tie(parent_id, edge_to_child_pos) = graph.mate(current_node, edge_to_parent_pos);
    assert(parent_id!=0);
    assert(edge_to_child_pos!=0);
    //assert(last==graph.head(parent_id, edge_to_child_pos));

    uint64_t child_parent_arr, parent_child_arr;
    // mark until current_node or stop if marked
    while(current_node!=stop_node){
        // update cached values
        child_parent_arr = annotated.get_pos(current_node, edge_to_parent_pos);
        parent_child_arr = annotated.get_pos(parent_id, edge_to_child_pos);
        // get mark
        last_mark = marks.get(parent_child_arr);
        // for obversation purposes check marked here
        // don't change full marks
        if(last_mark!=full_marked) {
            // mark edges
            if (parent_id == stop_node){
                marks.set(child_parent_arr, half_marked);
                marks.set(parent_child_arr, half_marked);
            }
            else{
                marks.set(child_parent_arr, full_marked);
                marks.set(parent_child_arr, full_marked);
            }
        }

        if (last_mark==full_marked)
            break;

        // head backwards, simply use counter_id
        current_node = parent_id;
        assert(current_node!=0);
        // use select_next_pos to get segment position of parent
        edge_to_parent_pos = annotated.select_segment_pos(parents, current_node, 1);
        // next operations could corrupt if called on root node, so check and break if neccessary
        if (current_node==stop_node && edge_to_parent_pos==0)
            break;
        assert(edge_to_parent_pos!=0);
        // find parent node and edge to this node
        std::tie(parent_id, edge_to_child_pos) = graph.mate(current_node, edge_to_parent_pos);
        assert(parent_id!=0);
        assert(edge_to_child_pos!=0);
        //assert(graph.head(parent_id, edge_to_child_pos)!=0);
        //assert(current_node==graph.head(parent_id, edge_to_child_pos));
    }
}


void mark_edges(const SFLGraph &graph, const annotated_edges_t &annotated, marked_edges_t &marks){
    const parent_edges_t& parents = std::get<edges_parent>(annotated.arrays);
    std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, bool)> marking = [&graph, &annotated, &parents, &marks](SFL_ID_SIZE node, SFL_POS_SIZE size, bool){
        // empty nodes are not relevant
        if (size==0)
            return true;
        uint64_t pos = annotated.select_segment_pos(std::get<edges_backlink>(annotated.arrays), node, 1);
        while(pos!=0){
            mark_parents(graph, annotated, parents, marks, graph.mate(node, pos), node);
            pos = annotated.select_segment_pos(std::get<edges_backlink>(annotated.arrays), node, pos+1);
        }
        // ignore if completely unmarked (can only see here)
        if(annotated.select_segment_pos(marks, node, 1)==0)
            return false;
        return true;
    };
    dfs(graph, annotated, 0,
        dfs_placeholder, dfs_placeholder, marking);
}

// with ignored nodes
void update_edges(const SFLGraph &graph, annotated_edges_t &annotated, const RSBitmap &removednodes){
    if(std::get<edges_parent>(annotated.arrays).is_static())
        annotated.reset<edges_parent>();
    if(std::get<edges_backlink>(annotated.arrays).is_static())
        annotated.reset<edges_backlink>();
    if(std::get<edge_marks>(annotated.arrays).is_static())
        annotated.reset<edge_marks>();
#ifndef NDEBUG
    uint64_t roots=0;
    std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, bool)> preprocess = [&annotated, &roots, &removednodes](SFL_ID_SIZE node, SFL_POS_SIZE size, bool is_root) {
        if (is_root && annotated.segment_size(node)>0){
            // count root nodes with not empty segment
            roots++;
        }
#else
    std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, bool)> preprocess = [&annotated, &removednodes](SFL_ID_SIZE node, SFL_POS_SIZE size, bool is_root) {
#endif
        if (removednodes.get(node))
            return false;
        return true;
    };

    std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, SFL_ID_SIZE, uint8_t)> dfs_tree_climb_down = [&graph, &annotated, &removednodes](SFL_ID_SIZE last, SFL_POS_SIZE edge, SFL_ID_SIZE next, uint8_t color) {
        if (removednodes.get(next))
            return false;
        //uint64_t arrpos = backlinks_ret.get_pos(last, edge);
        uint64_t parentarrpos = annotated.get_pos(next,std::get<1>(graph.mate(last, edge)));
        if (color==white){
            // climb down; normal case
            annotated.set<edges_parent>(parentarrpos, true);
            //assert(backlinks_ret.get<edges_parent>(arrpos)==false);
        } else if(color!=black){
            // = gray or darkgray and not parent
            // mark in parent which edges are backlinks
            annotated.set<edges_backlink>(parentarrpos, true);
            //assert(backlinks_ret.get<edges_backlink>(parentarrpos)==false);
        }
        return true;
    };
    dfs(graph, annotated, 0, dfs_tree_climb_down, dfs_placeholder, preprocess);
    std::get<edges_parent>(annotated.arrays).make_static();
    std::get<edges_backlink>(annotated.arrays).make_static();
    // initialize mark array
    mark_edges(graph, annotated, std::get<edge_marks>(annotated.arrays));
    // make static
    std::get<edge_marks>(annotated.arrays).make_static();
    //
    assert(std::get<edges_parent>(annotated.arrays).is_static());
    assert(std::get<edge_marks>(annotated.arrays).is_static());
#ifndef NDEBUG
    if (removednodes.ones()==0){
        assert(annotated.segments()-annotated.all_empty() == std::get<edges_parent>(annotated.arrays).ones()+roots);
    }
#endif
}

annotated_edges_t annotate_edges(const SFLGraph &graph, const RSBitmap &removednodes){
    annotated_edges_t annotated(graph.begin_deg(), graph.end_deg(), false, false, array_multi_bit<2>());
    update_edges(graph, annotated, removednodes);
    return annotated;
}


const RSBitmap cutvertices(const SFLGraph &graph, const annotated_edges_t &annotated, const RSBitmap &removednodes){
    const marked_edges_t &marks = std::get<edge_marks>(annotated.arrays);
    SFL_POS_SIZE deg;
    SFL_ID_SIZE next_node;
    SFL_POS_SIZE back_edge;
    uint8_t mark;
    bool root;
    bool is_parent;
    uint8_t childrencounter;
    RSBitmap cutvertices_ret(graph.n());
    const parent_edges_t& parents = std::get<edges_parent>(annotated.arrays);
    for(SFL_ID_SIZE node=1; node<=graph.n(); node++){
        deg = graph.deg(node);
        if (deg<=1 || removednodes.get(node))
            continue;
        root = (annotated.select_segment_pos(parents, node, 1)==0);
        childrencounter=0;
        for (SFL_POS_SIZE edge=1; edge<=deg; edge++){
            // cache position
            uint64_t pos_array = annotated.get_pos(node, edge);
            // parents should be ignored. Causes error with half marked edges (parent is cutvertice, child not)
            if(parents.get(pos_array)){
                continue;
            }
            std::tie(next_node, back_edge) = graph.mate(node, edge);
            is_parent = (annotated.select_segment_pos(parents, next_node, 1)==back_edge);
            mark = marks.get(pos_array);

            if(is_parent)
                childrencounter++;
            if ((childrencounter>=2 && root) || (!root && mark!=full_marked && is_parent) ){
                //printf("node: %lu, edge: %lu, target: %lu, root: %u, mark: %u, parent: %u, childrencounter: %lu\n", node, edge, graph.head(node, edge), root, mark, is_parent, childrencounter);
                cutvertices_ret.set(node, true);
                break;
            }
        }
    }
    return cutvertices_ret;
}

void biconnected_components(const SFLGraph &graph, const annotated_edges_t &annotated, std::function<void (SFL_ID_SIZE, bool)>output_func, const RSBitmap &removednodes){
    SFL_ID_SIZE node, parent_node;
    uint64_t edge;
    const marked_edges_t &marks = std::get<edge_marks>(annotated.arrays);
    const parent_edges_t& parents = std::get<edges_parent>(annotated.arrays);

    std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, bool)> preprocess = [&annotated, &removednodes](SFL_ID_SIZE node, SFL_POS_SIZE size, bool is_root) {
        if (removednodes.get(node))
            return false;
        return true;
    };

    std::function<bool (SFL_ID_SIZE, SFL_POS_SIZE, SFL_ID_SIZE, uint8_t)> cc_and_stop = [&annotated, &marks, &output_func, &parent_node, &removednodes](SFL_ID_SIZE last, SFL_POS_SIZE edge, SFL_ID_SIZE next, uint8_t color) {
        if(removednodes.get(next))
            return false;
        // block real start node, as the algorithm could traverse in other biconnected components
        if(next==parent_node){
            return false;
        }
        // black and gray nodes should be ignored:
        // they are either already in the bitmap or unrelated or already processed
        // last 2 cases cause errors
        if (color!=white)
            return true;
        uint8_t mark = marks.get(annotated.get_pos(last, edge));
        if (mark==full_marked) {
            output_func(next, false);
            return true;
        }
        return false;
    };
    for(node=1; node<graph.n();node++){
        if (removednodes.get(node))
            continue;
        edge = annotated.select_segment_pos(parents, node, 1);
        // is empty
        if(edge==0)
            continue;
        if(marks.get(annotated.get_pos(node, edge))==half_marked){
            parent_node = graph.head(node, edge);
            // parent_node, won't be printed elsewise
            output_func(parent_node, true);
            // start_node (half marked is barrier)
            output_func(node, false);
            dfs(graph, annotated, node, cc_and_stop, dfs_placeholder, preprocess);
        }
    }
}

const std::vector<std::vector<SFL_ID_SIZE>> biconnected_components(const SFLGraph &graph, const annotated_edges_t &annotated, const RSBitmap &removednodes){
    std::vector<std::vector<SFL_ID_SIZE>> cc_ret;
    std::function<void (SFL_ID_SIZE, bool)> output_func = [&cc_ret](SFL_ID_SIZE node, bool new_component) {
        if (new_component){
            // create new component with 1 element (node)
            cc_ret.emplace_back(1, node);
        } else {
            // update component
            cc_ret.back().push_back(node);
        }
    };
    biconnected_components(graph, annotated, output_func, removednodes);
    return cc_ret;
}
