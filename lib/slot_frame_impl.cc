/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "slot_frame_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace freq_hopping {

#pragma message("set the following appropriately and remove this warning")
using output_type = int;
slot_frame::sptr slot_frame::make(int hop_rate, int M_order, int info_seed)
{
    return gnuradio::make_block_sptr<slot_frame_impl>(hop_rate, M_order, info_seed);
}


/*
 * The private constructor
 */
slot_frame_impl::slot_frame_impl(int hop_rate, int M_order, int info_seed)
    : gr::sync_block("slot_frame",
                     gr::io_signature::make(0, 0, 0),
                     gr::io_signature::make(1 , 1 , sizeof(output_type) * cal_vector_len(FSY_CH_HOP,hop_rate))),
    d_hop_rate(hop_rate),
    d_M_order(M_order),
    d_info_seed(info_seed),
    d_hops_count(0)
{
    // 使用静态函数获取参数
    try {
        auto params = get_samp1hop(FSY_CH_HOP, hop_rate);
        num_sym_head = params[0];
        num_sym_pld = params[1];
    }
    catch (const std::runtime_error& e) {
        // 如果hop_rate未找到，使用默认长度 5hops/s
        num_sym_head = 108;
        num_sym_pld = 324;
    }
}

/*
 * Our virtual destructor.
 */
slot_frame_impl::~slot_frame_impl() {}

int slot_frame_impl::work(int noutput_items,
                          gr_vector_const_void_star& input_items,
                          gr_vector_void_star& output_items)
{
    auto out = static_cast<output_type*>(output_items[0]);
    int vec_len = num_sym_head + num_sym_pld;

    int vectors_produced = 0;
    for (int i = 0; i < noutput_items; i++) {
        // 生成下一帧数据
        generate_frame();
        // 复制当前帧数据到输出缓冲区
        std::copy(d_cnt_frame.begin(), d_cnt_frame.end(), out+(i * vec_len));
        vectors_produced++;
        d_hops_count++;
    }

    // 返回产生的向量数量
    return vectors_produced;
}

// 生成帧数据的辅助函数
void slot_frame_impl::generate_frame()
{
    size_t vector_length = num_sym_head + num_sym_pld;
    if (d_cnt_frame.size() != vector_length) {
        d_cnt_frame.resize(vector_length);
    }

    // 1. 生成同步头 (使用固定seed=0)
    std::mt19937 head_gen(2025); // 固定seed
    std::uniform_int_distribution<int> head_dist(0, d_M_order - 1);

    for (int i = 0; i < num_sym_head; i++) {
        d_cnt_frame[i] = head_dist(head_gen);
    }

    // 2. 生成信息序列 (每帧相同)
    std::mt19937 pld_gen(d_info_seed);
    std::uniform_int_distribution<int> pld_dist(0, d_M_order - 1);

    for (int i = 0; i < num_sym_pld; i++) {
        d_cnt_frame[num_sym_head + i] = pld_dist(pld_gen);
    }


}



} /* namespace freq_hopping */
} /* namespace gr */
