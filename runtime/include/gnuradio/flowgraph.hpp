/* -*- c++ -*- */
/*
 * Copyright 2006,2007,2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_GR_RUNTIME_FLOWGRAPH_H
#define INCLUDED_GR_RUNTIME_FLOWGRAPH_H

#include "api.h"
#include <iostream>
#include <memory>

#include <gnuradio/blocklib/block.hpp>
#include <gnuradio/graph.hpp>
#include <gnuradio/scheduler.hpp>

namespace gr {

typedef std::tuple<scheduler_sptr, std::vector<block_sptr>> partition_conf;
typedef std::vector<partition_conf> partition_conf_vec;
class flowgraph : public graph
{
private:
    std::vector<scheduler_sptr> d_schedulers;
    flat_graph_sptr d_flat_graph;
    std::vector<graph> d_subgraphs;

public:
    flowgraph(){};
    typedef std::shared_ptr<flowgraph> sptr;
    virtual ~flowgraph(){};
    void set_scheduler(scheduler_sptr sched) { d_schedulers = std::vector<scheduler_sptr>{sched}; }
    void set_schedulers(std::vector<scheduler_sptr> sched) { d_schedulers = sched; }
    void add_scheduler(scheduler_sptr sched) { d_schedulers.push_back(sched); }
    void clear_schedulers() { d_schedulers.clear(); }
    void partition(partition_conf_vec& confs)
    {
        // Create new subgraphs based on the partition configuration
        d_subgraphs.clear();

        std::vector<edge> domain_crossings;

        for (auto conf : confs) {
            graph g; // create a new subgraph
            // Go through the blocks assigned to this scheduler
            // See whether they connect to the same graph or account for a domain crossing
            
            auto blocks = std::get<1>(conf);
            for (auto b : blocks) // for each of the blocks in the tuple
            {
                for (auto input_port : b->input_stream_ports()) {
                    auto edges = find_edge(input_port);
                    // There should only be one edge connected to an input port
                    auto e = edges[0];
                    auto other_block = e.src().node();

                    // Is the other block in our current partition
                    if (std::find(blocks.begin(), blocks.end(), other_block) !=
                        blocks.end()) {
                        g.connect(e.src(), e.dst());
                    } else {
                        // add this edge to the list of domain crossings
                        domain_crossings.push_back(e);
                    }
                }
            }

            d_subgraphs.push_back(g);
        }
    }
    void validate()
    {
        d_flat_graph = flat_graph::make_flat(base());
        for (auto sched : d_schedulers)
            sched->initialize(d_flat_graph);
    }
    void start()
    {
        for (auto s : d_schedulers) {
            s->start();
        }
    }
    void stop()
    {
        for (auto s : d_schedulers) {
            s->stop();
        }
    }
    void wait()
    {
        for (auto s : d_schedulers) {
            s->wait();
        }
    }
};

typedef flowgraph::sptr flowgraph_sptr;
} // namespace gr

#endif