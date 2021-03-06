/* -*- c++ -*- */
/*
 * Copyright 2004,2008,2010,2013,2018,2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include <gnuradio/sync_block.hpp>

namespace gr {
namespace blocks {

class null_sink : virtual public sync_block
{

public:
    enum params : uint32_t { id_itemsize, id_nports, num_params };
    typedef std::shared_ptr<null_sink> sptr;

    static sptr make(const size_t itemsize, const size_t nports=1)
    {
        auto ptr = std::make_shared<null_sink>(itemsize, nports);

        for (size_t i = 0; i < nports; i++) {
            ptr->add_port(untyped_port::make("input" + std::to_string(i),
                                             port_direction_t::INPUT,
                                             itemsize));
        }

        return ptr;
    }

    null_sink(const size_t itemsize, const size_t nports)
        : sync_block("null_sink"), _itemsize(itemsize), _nports(nports){};
    // ~null_sink() {};

    work_return_code_t work(std::vector<block_work_input>& work_input,
                            std::vector<block_work_output>& work_output)
    {
        return work_return_code_t::WORK_OK;
    }


private:
    size_t _itemsize;
    size_t _nports;
};

} /* namespace blocks */
} /* namespace gr */
