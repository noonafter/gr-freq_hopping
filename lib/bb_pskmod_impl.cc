/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "bb_pskmod_impl.h"

#include <gnuradio/io_signature.h>

namespace gr {
namespace freq_hopping {

using input_type = int;
using output_type = gr_complex;
bb_pskmod::sptr bb_pskmod::make(int hop_rate, int M_order, int Ksa_ch)
{
    return gnuradio::make_block_sptr<bb_pskmod_impl>(hop_rate, M_order, Ksa_ch);
}


bb_pskmod_impl::bb_pskmod_impl(int hop_rate, int M_order, int Ksa_ch)
    : gr::sync_block("bb_pskmod",
                     gr::io_signature::make(
                         1, 1, sizeof(input_type) * calculate_input_length(hop_rate)),
                     gr::io_signature::make(
                         1, 1, sizeof(output_type) * calculate_output_length(hop_rate,Ksa_ch))),
    d_hop_rate(hop_rate),
    d_M_order(M_order),
    d_Ksa_ch(Ksa_ch),
    rrc_span(0),
    d_rrc_filter(nullptr),
    d_initialized(false)
{
    // 验证参数
    if (d_M_order != 2 && d_M_order != 4 && d_M_order != 8) {
        throw std::invalid_argument("M_order must be 2, 4, or 8");
    }
    if (d_Ksa_ch <= 0) {
        throw std::invalid_argument("Ksa_ch must be positive");
    }

    // 计算输入输出向量长度
    d_input_frame_len = calculate_input_length(d_hop_rate);
    d_output_frame_len = calculate_output_length(d_hop_rate,d_Ksa_ch);

    // 设置输入输出向量大小，这个是控制缓冲区大小的，暂时不用
    // set_output_multiple(d_output_frame_length);
    // set_input_multiple(d_input_frame_length);

    // 初始化星座图
    initialize_constellation();

    // 设计RRC滤波器
    design_rrc_filter();

    d_initialized = true;
    // set_history(3);
}

bb_pskmod_impl::~bb_pskmod_impl()
{
    if (d_rrc_filter) {
        firinterp_crcf_destroy(d_rrc_filter);
    }
}

void bb_pskmod_impl::initialize_constellation()
{
    d_constellation.clear();

    switch (d_M_order) {
        case 2:  // BPSK
            d_constellation = {
            gr_complex(1.0f, 0.0f),   // 0: 1+0j
            gr_complex(-1.0f, 0.0f)   // 1: -1+0j
        };
            break;

        default:  // 4/8PSK
            for (int i = 0; i < d_M_order; ++i) {
                float phase = 2.0f * M_PI * i / d_M_order;
                d_constellation.push_back(gr_complex(std::cos(phase), std::sin(phase)));
            }
    }
}

void bb_pskmod_impl::design_rrc_filter()
{
    // RRC滤波器参数
    rrc_span = 8;
    const float rolloff = 0.25f;      // 滚降因子
    const int ntaps = rrc_span * d_Ksa_ch + 1; // 滤波器抽头数

    // 设计根升余弦滤波器
    d_rrc_taps = gr::filter::firdes::root_raised_cosine(
        1.0,    // 增益
        d_Ksa_ch,    // 采样率 (符号率的Ksa_ch倍)
        1.0,        // 符号率
        rolloff,     // 滚降因子
        ntaps        // 抽头数
    );

    // 重新创建滤波器
    if (d_rrc_filter) {
        firinterp_crcf_destroy(d_rrc_filter);
    }
    d_rrc_filter = firinterp_crcf_create(d_Ksa_ch,d_rrc_taps.data(),ntaps);
}

gr_complex bb_pskmod_impl::map_to_constellation(int symbol_index)
{
    if (symbol_index < 0 || symbol_index >= static_cast<int>(d_constellation.size())) {
        // 错误处理：返回第一个星座点
        return d_constellation[0];
    }
    return d_constellation[symbol_index];
}

int bb_pskmod_impl::work(int noutput_items,
                         gr_vector_const_void_star& input_items,
                         gr_vector_void_star& output_items)
{
    auto in = static_cast<const input_type*>(input_items[0]);
    auto out = static_cast<output_type*>(output_items[0]);
    // 通过测试发现，history()默认值为1,当使用set_histry(n+1)设置重叠区域后，
    // 每次in会有n个item是上一次in的末尾n个数据，即滑动窗会有n个item重叠
    // 即每次读入的数据中,in[0],...,in[n-1]是旧数据，一共n个（第一次调用时，为全0）
    // in[n],...,in[n+noutput_items-1]是新数据，一共noutput_items个
    // 具体实现：在读入niutput_items数据后（将rptr后移niutput_items），将rptr前移n
    // printf("bb_pskmod_impl::work, history: %d\n",history());
    // printf("new work, noutput_items: %d\n",noutput_items);
    // printf("bb_pskmod_impl::work, in[0]: %d\n",in[0]);
    // printf("bb_pskmod_impl::work, in[1]: %d\n",in[1]);
    // printf("bb_pskmod_impl::work, in[0+d_input_frame_len]: %d\n",in[0+d_input_frame_len]);
    // printf("bb_pskmod_impl::work, in[1+d_input_frame_len]: %d\n",in[1+d_input_frame_len]);
    // printf("bb_pskmod_impl::work, in[noutput_items+history()-3)*d_input_frame_len+0]: %d\n",in[(noutput_items+history()-3)*d_input_frame_len]);
    // printf("bb_pskmod_impl::work, in[noutput_items+history()-3)*d_input_frame_len+1]: %d\n",in[(noutput_items+history()-3)*d_input_frame_len+1]);
    // printf("bb_pskmod_impl::work, in[(noutput_items+history()-2)*d_input_frame_len]: %d\n",in[(noutput_items+history()-2)*d_input_frame_len]);
    // printf("bb_pskmod_impl::work, in[(noutput_items+history()-2)*d_input_frame_len+1]: %d\n",in[(noutput_items+history()-2)*d_input_frame_len+1]);

    // 首先将out@(noutput_items*d_output_frame_length)全部填0
    memset(out, 0, noutput_items * d_output_frame_len * sizeof(output_type));

    if (!d_initialized) {
        return 0;
    }

    int idx_frame = 0;
    int num_sym_transition = (rrc_span>>1)-1;
    output_type transition_out[d_Ksa_ch];
    for (; idx_frame < noutput_items; ++idx_frame) {
        const input_type* frame_in = in + idx_frame * d_input_frame_len;
        output_type* frame_out = out + idx_frame * d_output_frame_len;


        int idx_in_sym = 0;
        // 第一阶段：处理前num_sym_transition个符号，输出到transition_out（不使用）
        for (; idx_in_sym < num_sym_transition; ++idx_in_sym) {
            gr_complex symbol = map_to_constellation(frame_in[idx_in_sym]);
            firinterp_crcf_execute(d_rrc_filter, symbol, transition_out);
        }

        // 第二阶段：处理接下来的符号，直到输入帧的末尾
        for (; idx_in_sym < d_input_frame_len; ++idx_in_sym) {
            gr_complex symbol = map_to_constellation(frame_in[idx_in_sym]);
            // 计算输出位置：当前输出符号索引是(idx_in_sym - num_sym_transition)，所以乘以d_Ksa_ch
            firinterp_crcf_execute(d_rrc_filter, symbol, &frame_out[(idx_in_sym - num_sym_transition) * d_Ksa_ch]);
        }

        // 第三阶段：输入0，继续产生num_sym_transition个符号的输出（每个符号d_Ksa_ch个样本）
        for (int i = 0; i < num_sym_transition; ++i) {
            gr_complex zero(0.0f, 0.0f);
            // 输出位置从第二阶段结束的位置开始，即(d_input_frame_len - num_sym_transition) * d_Ksa_ch，然后每次递增d_Ksa_ch
            firinterp_crcf_execute(d_rrc_filter, zero, &frame_out[(d_input_frame_len - num_sym_transition) * d_Ksa_ch + i * d_Ksa_ch]);
        }

    }

    return idx_frame;
}


} /* namespace freq_hopping */
} /* namespace gr */
