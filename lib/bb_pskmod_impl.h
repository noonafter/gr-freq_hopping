/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_BB_PSKMOD_IMPL_H
#define INCLUDED_FREQ_HOPPING_BB_PSKMOD_IMPL_H

#include <gnuradio/freq_hopping/bb_pskmod.h>
#include <gnuradio/filter/interp_fir_filter.h>
#include <gnuradio/filter/fir_filter.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/digital/constellation.h>
#include <vector>
#include <complex>
#include "liquid/liquid.h"
#include "slot_frame_impl.h"

namespace gr {
namespace freq_hopping {

class bb_pskmod_impl : public bb_pskmod
{
private:
    int d_hop_rate;
    int d_M_order;
    int d_Ksa_ch;

    // 帧长度
    int d_input_frame_len;
    int d_output_frame_len;

    // 星座图
    std::vector<gr_complex> d_constellation;

    // 成形滤波器
    int rrc_span;
    std::vector<float> d_rrc_taps;
    firinterp_crcf d_rrc_filter;

    bool d_initialized;

    void initialize_constellation();
    void design_rrc_filter();
    gr_complex map_to_constellation(int symbol_index);

public:
    bb_pskmod_impl(int hop_rate, int M_order, int Ksa_ch);
    ~bb_pskmod_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);

    // 计算向量长度的辅助函数
    static int calculate_input_length(int hop_rate) {
        return slot_frame_impl::cal_vector_len(FSY_CH_HOP, hop_rate);
    }

    static int calculate_output_length(int hop_rate, int Ksa_ch) {
        // 使用静态函数获取参数
        try {
            auto params = slot_frame_impl::get_samp1hop(FSY_CH_HOP*Ksa_ch, hop_rate);
            int head_pld_pad = params[4]; // totle 是第5个参数

            // 确保长度不超过500
            if (head_pld_pad > 500*Ksa_ch) {
                head_pld_pad = 500*Ksa_ch;
            }

            // 确保长度至少为1
            if (head_pld_pad < 1) {
                head_pld_pad = 1;
            }

            return head_pld_pad;
        }
        catch (const std::runtime_error& e) {
            // 如果hop_rate未找到，使用默认长度
            return 480*Ksa_ch;
        }
    }

};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_BB_PSKMOD_IMPL_H */
