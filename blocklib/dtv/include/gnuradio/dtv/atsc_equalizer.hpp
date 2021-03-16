/* -*- c++ -*- */
/*
 * Copyright 2014 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include <gnuradio/block.hpp>
#include <gnuradio/dtv/atsc_consts.hpp>

namespace gr {
namespace dtv {

/*!
 * \brief ATSC Receiver Equalizer
 *
 * \ingroup dtv_atsc
 */
class atsc_equalizer : virtual public gr::block
{
public:
    // gr::dtv::atsc_equalizer::sptr
    typedef std::shared_ptr<atsc_equalizer> sptr;

    /*!
     * \brief Make a new instance of gr::dtv::atsc_equalizer.
     */
    static sptr make();

    atsc_equalizer();

    std::vector<float> taps() const;
    std::vector<float> data() const;

    work_return_code_t work(std::vector<block_work_input>& work_input,
                            std::vector<block_work_output>& work_output) override;

private:
    static constexpr int NTAPS = 64;
    static constexpr int NPRETAPS =
        (int)(NTAPS * 0.8); // probably should be either .2 or .8

    // the length of the field sync pattern that we know unequivocally
    static constexpr int KNOWN_FIELD_SYNC_LENGTH = 4 + 511 + 3 * 63;

    float training_sequence1[KNOWN_FIELD_SYNC_LENGTH];
    float training_sequence2[KNOWN_FIELD_SYNC_LENGTH];

    void filterN(const float* input_samples, float* output_samples, int nsamples);
    void adaptN(const float* input_samples,
                const float* training_pattern,
                float* output_samples,
                int nsamples);

    std::vector<float> d_taps;

    float data_mem[ATSC_DATA_SEGMENT_LENGTH + NTAPS]; // Buffer for previous data packet
    float data_mem2[ATSC_DATA_SEGMENT_LENGTH];
    unsigned short d_flags;
    short d_segno;

    bool d_buff_not_filled = true;

};

} /* namespace dtv */
} /* namespace gr */
