#include <gnuradio/graph.hpp>
#include <gnuradio/blocklib/block.hpp>

// All the things that happen to a graph once it is flat
// Works only with block_sptr and block_edges
namespace gr
{

class flat_graph
{
    block_vector_t d_blocks;
    edge_vector_t d_edges;

    void validate();
    block_vector_t calc_used_blocks();
    block_vector_t topological_sort(block_vector_t& blocks);
    std::vector<block_vector_t> partition();

    edge find_edge(block_sptr b, int port_index, block::io io_type)
    {
        for (auto& e : edges()) {
            endpoint tmp(b, port_index);
            if (io_type == block::io::INPUT) {
                if (e.dst().identifier() == tmp.identifier()) {
                    return e;
                }
            } else {
                if (e.src().identifier() == tmp.identifier()) {
                    return e;
                }
            }
        }

        throw std::invalid_argument("edge not found");
    }

protected:
    block_vector_t d_blocks;
    edge_vector_t d_edges;

    std::vector<int> calc_used_ports(block_sptr block, bool check_inputs);
    block_vector_t calc_downstream_blocks(block_sptr block, int port);
    edge_vector_t calc_upstream_edges(block_sptr block);
    bool has_block_p(block_sptr block);
    edge calc_upstream_edge(block_sptr block, int port);

private:
    void check_valid_port(gr::io_signature& sig, int port);
    void check_dst_not_used(const endpoint& dst);
    void check_type_match(const endpoint& src, const endpoint& dst);
    edge_vector_t calc_connections(block_sptr block,
                                   bool check_inputs); // false=use outputs
    void check_contiguity(block_sptr block,
                          const std::vector<int>& used_ports,
                          bool check_inputs);

    block_vector_t calc_downstream_blocks(block_sptr block);
    block_vector_t calc_reachable_blocks(block_sptr blk, block_vector_t& blocks);
    void reachable_dfs_visit(block_sptr blk, block_vector_t& blocks);
    block_vector_t calc_adjacent_blocks(block_sptr blk, block_vector_t& blocks);
    block_vector_t sort_sources_first(block_vector_t& blocks);
    bool source_p(block_sptr block);
    void topological_dfs_visit(block_sptr blk, block_vector_t& output);

};

}