/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "hop_mod_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace freq_hopping {

using input_type = gr_complex;
using output_type = gr_complex;
hop_mod::sptr hop_mod::make(double bw_hop, double ch_sep, double freq_carrier, double fsa_hop, int vlen)
{
    return gnuradio::make_block_sptr<hop_mod_impl>(bw_hop, ch_sep, freq_carrier, fsa_hop, vlen);
}


/*
 * The private constructor
 */
hop_mod_impl::hop_mod_impl(double bw_hop, double ch_sep, double freq_carrier, double fsa_hop,int vlen)
    : gr::sync_block("hop_mod",
                     gr::io_signature::make(1, 1, vlen*sizeof(input_type)),
                     gr::io_signature::make(1, 1, vlen*sizeof(output_type))),
    d_bw_hop(bw_hop),
    d_ch_sep(ch_sep),
    d_freq_carrier(freq_carrier),
    d_fsa_hop(fsa_hop),
    d_vlen(vlen),
    d_initialized(false),
    d_nco(nullptr)
{
    // 参数验证
    if (d_bw_hop <= 0) {
        throw std::invalid_argument("bw_hop must be positive");
    }
    if (d_ch_sep <= 0) {
        throw std::invalid_argument("ch_sep must be positive");
    }
    if (d_fsa_hop <= 0) {
        throw std::invalid_argument("fsa_hop must be positive");
    }

    // 初始化频率表
    initialize_frequency_table();

    // 初始化随机数生成器
    std::random_device rd;
    d_rng = std::mt19937(rd());
    d_nco = nco_crcf_create(LIQUID_VCO);

    d_initialized = true;
}

/*
 * Our virtual destructor.
 */
hop_mod_impl::~hop_mod_impl()
{
    if (d_nco) {
        nco_crcf_destroy(d_nco);
    }
}

void hop_mod_impl::initialize_frequency_table()
{
    // 计算信道数量
    d_num_ch = static_cast<int>(std::floor(d_bw_hop / d_ch_sep));
    if (d_num_ch < 1) {
        d_num_ch = 1;
    }

    // 生成频率表
    d_freq_vec.resize(d_num_ch);
    for (int i = 0; i < d_num_ch; ++i) {
        d_freq_vec[i] = (i - std::floor(d_num_ch / 2.0)) * d_ch_sep + d_freq_carrier;
    }

    // 初始化均匀分布
    d_dist = std::uniform_int_distribution<int>(0, d_num_ch - 1);
}

double hop_mod_impl::get_random_frequency()
{
    int freq_idx = d_dist(d_rng);
    return d_freq_vec[freq_idx];
}

std::vector<gr_complex> hop_mod_impl::frequency_modulate(const std::vector<gr_complex>& input, double freq)
{
    std::vector<gr_complex> output(input.size());

    // 生成复指数信号进行频率调制
    for (size_t i = 0; i < input.size(); ++i) {
        double t = static_cast<double>(i) / d_fsa_hop;  // 时间
        double phase = 2.0 * M_PI * freq * t;          // 相位

        // 频率调制：乘以复指数
        gr_complex carrier(std::cos(phase), std::sin(phase));
        output[i] = input[i] * carrier;
    }

    return output;
}

int hop_mod_impl::work(int noutput_items,
                       gr_vector_const_void_star& input_items,
                       gr_vector_void_star& output_items)
{
    auto in = static_cast<const input_type*>(input_items[0]);
    auto out = static_cast<output_type*>(output_items[0]);

    // 处理每一帧
    int idx_vec = 0;
    for (; idx_vec < noutput_items; ++idx_vec) {
        // 获取当前帧的输入和输出位置
        const input_type* frame_in = in + idx_vec * d_vlen;
        output_type* frame_out = out + idx_vec * d_vlen;

        // 为当前帧选择随机频率
        double freq_tb = get_random_frequency();

        // 设置 NCO 频率
        nco_crcf_set_phase(d_nco, 0);
        nco_crcf_set_frequency(d_nco, 2*M_PI*freq_tb/d_fsa_hop);

        // 使用 NCO 进行频率调制（上混频）
        nco_crcf_mix_block_up(d_nco, const_cast<input_type*>(frame_in), frame_out, d_vlen);

        // // 调试信息（可选）
        // if (idx_vec == 0) {
        //     std::cout << "First frame: selected frequency = " << freq_tb << " Hz" << std::endl;
        // }
    }

    // Tell runtime system how many output items we produced.
    return idx_vec;
}

} /* namespace freq_hopping */
} /* namespace gr */
