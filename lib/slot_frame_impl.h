/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_SLOT_FRAME_IMPL_H
#define INCLUDED_FREQ_HOPPING_SLOT_FRAME_IMPL_H

#include <gnuradio/freq_hopping/slot_frame.h>
#include <vector>
#include <random>

const int FSY_CH_HOP = 2400;

namespace gr {
namespace freq_hopping {

class slot_frame_impl : public slot_frame
{
private:
    int d_hop_rate;
    int d_M_order;
    int d_info_seed;

    int num_sym_head;
    int num_sym_pld;

    std::vector<int> d_cnt_frame;
    int d_hops_count;

    // 生成帧数据的辅助函数
    void generate_frame();

public:
    slot_frame_impl(int hop_rate, int M_order, int info_seed);
    ~slot_frame_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);

    // 输出向量的长度， in sym
    static int cal_vector_len(int samp_rate, int hop_rate)
    {
        // 使用静态函数获取参数
        try {
            auto params = get_samp1hop(samp_rate, hop_rate);
            int head_pld = params[2]; // head_pld 是第三个参数

            // 确保长度不超过500
            if (head_pld > 500) {
                head_pld = 500;
            }

            // 确保长度至少为1
            if (head_pld < 1) {
                head_pld = 1;
            }

            return head_pld;
        }
        catch (const std::runtime_error& e) {
            // 如果hop_rate未找到，使用默认长度
            return 432;
        }
    };

    static std::array<int,5> get_samp1hop(int samp_rate, int hop_rate)
    {
        // 创建映射表 - hop_rate 到参数数组
        static const std::map<int, std::array<double, 5>> map_hop = {
            {5,   {45.0,   135.0,  180.0,   20.0,    200.0}},
            {10,  {22.5,   67.5,   90.0,    10.0,    100.0}},
            {20,  {11.25,  33.75,  45.0,    5.0,     50.0}},
            {50,  {4.167,  12.5,   16.667,  3.332,   20.0}},
            {100, {2.083,  6.25,   8.333,   1.667,   10.0}},
            {110, {2.5,    5.0,    7.5,     1.563,   9.063}}
        };

        // 查找hop_rate
        auto it = map_hop.find(hop_rate);
        if (it == map_hop.end()) {
            throw std::runtime_error("hop_rate not found");
        }

        // 获取参数并计算最终值
        const auto& params = it->second;
        std::array<int, 5> result;

        for (size_t i = 0; i < 5; i++) {
            // 乘以 samp_rate/1000 并四舍五入到最接近的整数
            result[i] = static_cast<int>(std::round(params[i] * samp_rate / 1000.0));
        }

        return result;

    };
};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_SLOT_FRAME_IMPL_H */
