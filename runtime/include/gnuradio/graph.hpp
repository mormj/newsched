
#ifndef INCLUDED_GR_RUNTIME_FLOWGRAPH_H
#define INCLUDED_GR_RUNTIME_FLOWGRAPH_H

#include "api.h"
#include <gnuradio/blocklib/node.hpp>
#include <iostream>
#include <utility>

namespace gr {

// typedef endpoint std::pair<node_sptr, port_sptr>
template <class A, class B>
class endpoint : public std::pair<A, B>
{
    private:
    A _a;
    B _b;
public:
    endpoint(A a, B b) : std::pair<A,B>(a,b) {}

    std::string identifier() const { return _a->alias() + ":" + _b->alias(); };
};

class node_endpoint : endpoint<node_sptr, port_sptr>
{
private:
    node_sptr d_node;
    port_sptr d_port;

public:
    node_endpoint(node_sptr node, port_sptr port)
        : endpoint<node_sptr, port_sptr>(node, port),
          d_node(node),
          d_port(port)
    {
    }

    node_sptr node() { return d_node; }
    port_sptr port() { return d_port; }
};

class block_endpoint : node_endpoint
{
private:
    block_sptr d_block;

public:
    node_endpoint(block_sptr block, port_sptr port)
        : node_endpoint(block, port), d_block(node)
    {
    }
};

class edge
{
protected:
    node_endpoint _src, _snk;

public:
    edge(node_sptr src_blk,
               port_sptr src_port,
               node_sptr snk_blk,
               port_sptr snk_port)
        : _src(node_endpoint(src_blk, snk_port)),
          _snk(node_endpoint(snk_blk, snk_port))
    {
    }
    port_sptr src() { return _src; }
    port_sptr snk() { return _snk; }

    std::string identifier() const
    {
        return d_src.identifier() + "->" + d_dst.identifier();
    }
}

typedef std::vector<edge>
    edge_vector_t;
typedef std::vector<edge>::iterator edge_viter_t;


/**
 * @brief Represents a set of ports connected by edges
 *
 */

class graph : node
{
protected:
    std::vector<node> _nodes;
    edge_vector_t _edges;

public:
    graph() {}
    std_vector<graph_edge>& edges() { return _edges; }
    void connect(const endpoint& src, const endpoint& dst);
    void disconnect(const endpoint& src, const endpoint& dst);
    void validate();
    void clear();

    /**
     * @brief Return a flattened graph (all subgraphs reduced to their constituent blocks and edges)
     *
     * @return graph
     */
    flat_graph flatten();


protected:
    block_vector_t d_blocks;
    edge_vector_t d_edges;

};

typedef std::shared_ptr<graph> graph_sptr;


#endif