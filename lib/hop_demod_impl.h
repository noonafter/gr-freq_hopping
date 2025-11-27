/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_HOP_DEMOD_IMPL_H
#define INCLUDED_FREQ_HOPPING_HOP_DEMOD_IMPL_H

#include <gnuradio/freq_hopping/hop_demod.h>
#include <liquid/liquid.h>
#include <random>

namespace gr {
namespace freq_hopping {

class hop_demod_impl : public hop_demod
{
private:
    // 参数
    double d_bw_hop;
    double d_ch_sep;
    double d_freq_carrier;
    double d_fsa_hop;
    double d_hop_rate;
    double d_hop_period;
    double d_samples_per_hop;

    // 频率表和跳频序列
    std::vector<double> d_freq_vec;
    std::vector<int> d_hop_sequence;
    int d_num_ch;

    // NCO和随机数生成器
    nco_crcf d_nco;
    std::mt19937 d_rng;

    // 状态变量
    bool d_has_time_reference;
    uint64_t d_ref_slot_idx;
    uint64_t d_hop_count;
    double d_elapsed_samples;
    double d_current_freq;

    // 内部方法
    void initialize_frequency_table();
    void initialize_hop_sequence();

public:
    hop_demod_impl(double bw_hop,
                   double ch_sep,
                   double freq_carrier,
                   double fsa_hop,
                   double hop_rate);
    ~hop_demod_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_HOP_DEMOD_IMPL_H */
