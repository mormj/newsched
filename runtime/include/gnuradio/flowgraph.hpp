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
#include <gnuradio/blocklib/graph.hpp>
#include <iostream>

namespace gr {

class flowgraph : graph
{
public:
    flowgraph();
    typedef std::shared_ptr<flowgraph> sptr;
    virtual ~flowgraph();

    void start() = 0;
    void stop() = 0;
    void wait() = 0;


};

typedef flowgraph::sptr flowgraph_sptr;
}

#endif