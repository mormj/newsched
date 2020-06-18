
#ifndef INCLUDED_GR_RUNTIME_GRAPH_H
#define INCLUDED_GR_RUNTIME_GRAPH_H

#include "api.h"
#include <gnuradio/blocklib/block.hpp>
#include <gnuradio/blocklib/node.hpp>
#include <iostream>
#include <utility>
#include <vector>

namespace gr {

// class flat_graph;

// typedef endpoint std::pair<node_sptr, port_sptr>
template <class A, class B>
class endpoint : public std::pair<A, B>
{
private:
    A _a;
    B _b;

public:
    endpoint(A a, B b) : std::pair<A, B>(a, b) {}
};

class node_endpoint : public endpoint<node_sptr, port_sptr>
{
private:
    node_sptr d_node;
    port_sptr d_port;

public:
    node_endpoint();
    node_endpoint(node_sptr node, port_sptr port)
        : endpoint<node_sptr, port_sptr>(node, port), d_node(node), d_port(port)
    {
    }

    node_endpoint(const node_endpoint& n)
        : endpoint<node_sptr, port_sptr>(n.node(), n.port())
    {
        d_node = n.node();
        d_port = n.port();
    }

    node_sptr node() const { return d_node; }
    port_sptr port() const { return d_port; }
    std::string identifier() const { return d_node->alias() + ":" + d_port->alias(); };
};

inline std::ostream& operator<<(std::ostream& os, const node_endpoint endp)
{
    os << endp.identifier();
    return os;
}


// class block_endpoint : node_endpoint
// {
// private:
//     block_sptr d_block;

// public:
//     node_endpoint(block_sptr block, port_sptr port)
//         : node_endpoint(block, port), d_block(node)
//     {
//     }
// };

class edge
{
protected:
    node_endpoint _src, _dst;

public:
    edge() {};
    edge(node_sptr src_blk, port_sptr src_port, node_sptr dst_blk, port_sptr dst_port)
        : _src(node_endpoint(src_blk, src_port)), _dst(node_endpoint(dst_blk, dst_port))
    {
    }
    node_endpoint src() { return _src; }
    node_endpoint dst() { return _dst; }

    std::string identifier() const
    {
        return _src.identifier() + "->" + _dst.identifier();
    }

    size_t itemsize() const
    {
        return _src.port()->itemsize();
    }
};

inline std::ostream& operator<<(std::ostream& os, const edge edge)
{
    os << edge.identifier();
    return os;
}

typedef std::vector<edge> edge_vector_t;
typedef std::vector<edge>::iterator edge_viter_t;


/**
 * @brief Represents a set of ports connected by edges
 *
 */

class graph : public node
{
protected:
    std::vector<node> _nodes;
    edge_vector_t _edges;

public:
    graph() {}
    std::vector<edge>& edges() { return _edges; }
    void connect(const node_endpoint& src, const node_endpoint& dst);
    void disconnect(const node_endpoint& src, const node_endpoint& dst);
    void validate();
    virtual void clear();

    /**
     * @brief Return a flattened graph (all subgraphs reduced to their constituent blocks
     * and edges)
     *
     * @return graph
     */
    // flat_graph flatten();


protected:
    node_vector_t d_nodes;
    edge_vector_t d_edges;
};

typedef std::shared_ptr<graph> graph_sptr;


} // namespace gr
#endif