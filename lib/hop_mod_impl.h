/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_HOP_MOD_IMPL_H
#define INCLUDED_FREQ_HOPPING_HOP_MOD_IMPL_H

#include <gnuradio/freq_hopping/hop_mod.h>

#include <liquid/liquid.h>


#include <random>

namespace gr {
namespace freq_hopping {

class hop_mod_impl : public hop_mod
{
private:
    double d_bw_hop;        // 跳频带宽
    double d_ch_sep;        // 信道间隔
    double d_freq_carrier;  // 载波中心频率
    double d_fsa_hop;       // 跳频采样率
    double d_hop_rate;
    int d_vlen;

    int d_num_ch;           // 信道数量
    std::vector<double> d_freq_vec;  // 频率表

    // 跳频参数
    double d_hop_period;    // 跳频周期（秒）
    uint64_t d_hop_count;   // 跳频计数器
    bool d_first_hop;       // 是否是第一跳
    uint64_t d_start_time;  // 起始时间

    // 随机数生成器
    std::mt19937 d_rng;
    std::vector<int> d_hop_sequence;  // 跳频序列

    // 帧长度
    int d_frame_len;
    bool d_initialized;

    // NCO对象
    nco_crcf d_nco;

    // 私有方法
    void initialize_frequency_table();
    void initialize_hop_sequence();
    double get_frequency_by_hop_count();
    // uint64_t get_current_usrp_time();
    std::pair<uint64_t, double> get_current_usrp_time();
    uint64_t align_to_time_slot(uint64_t current_time_ns);
    // double get_random_frequency();
    // std::vector<gr_complex> frequency_modulate(const std::vector<gr_complex>& input, double freq);

public:
    hop_mod_impl(double bw_hop, double ch_sep, double freq_carrier, double fsa_hop, double hop_rate, int vlen);
    ~hop_mod_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_HOP_MOD_IMPL_H */
