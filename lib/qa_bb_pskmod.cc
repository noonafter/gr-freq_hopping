/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/attributes.h>
#include <gnuradio/freq_hopping/bb_pskmod.h>
#include <gnuradio/freq_hopping/slot_frame.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <vector>
#include <complex>
#include "bb_pskmod_impl.h"

namespace gr {
namespace freq_hopping {

BOOST_AUTO_TEST_CASE(test_bb_pskmod_basic)
{
    // 测试参数
    int hop_rate = 5;      // 5 hops/s
    int M_order = 4;       // QPSK
    int Ksa_ch = 4;        // 过采样因子
    int num_frames = 5;    // 只处理5帧

    // 计算输入输出向量长度
    int input_length = bb_pskmod_impl::calculate_input_length(hop_rate);
    int output_length = bb_pskmod_impl::calculate_output_length(hop_rate, Ksa_ch);

    std::cout << "Input frame length: " << input_length << std::endl;
    std::cout << "Output frame length: " << output_length << std::endl;

    // 创建测试输入数据 - 生成多帧有效的符号索引
    std::vector<int> test_data(input_length * num_frames);
    for (int i = 0; i < test_data.size(); i++) {
        test_data[i] = i % M_order;  // 确保符号在有效范围内
    }

    // 创建块实例
    auto bb_pskmod_block = bb_pskmod::make(hop_rate, M_order, Ksa_ch);
    auto source = gr::blocks::vector_source_i::make(test_data, false, input_length);
    auto head = gr::blocks::head::make(sizeof(int) * input_length, num_frames);
    auto sink = gr::blocks::vector_sink_c::make(output_length, 1024);

    // 创建流图
    auto tb = gr::make_top_block("test_bb_pskmod");
    tb->connect(source, 0, head, 0);
    tb->connect(head, 0, bb_pskmod_block, 0);
    tb->connect(bb_pskmod_block, 0, sink, 0);

    // 运行流图
    tb->run();

    // 获取输出数据
    auto output_data = sink->data();

    // 基本验证
    BOOST_CHECK(!output_data.empty());
    BOOST_CHECK_EQUAL(output_data.size(), num_frames * output_length);

    std::cout << "Generated " << output_data.size() << " complex samples from " << num_frames << " frames" << std::endl;

    // 检查输出是否包含非零值（因为调制和滤波应该产生非零输出）
    bool has_non_zero = false;
    for (const auto& sample : output_data) {
        if (std::abs(sample) > 1e-6) {
            has_non_zero = true;
            break;
        }
    }
    BOOST_CHECK(has_non_zero);
}

BOOST_AUTO_TEST_CASE(test_bb_pskmod_different_modulations)
{
    // 测试不同调制方式
    int hop_rate = 10;
    int Ksa_ch = 4;
    int num_frames = 3;    // 只处理3帧

    // 测试 BPSK
    {
        int M_order = 2;
        int input_length = bb_pskmod_impl::calculate_input_length(hop_rate);
        int output_length = bb_pskmod_impl::calculate_output_length(hop_rate, Ksa_ch);

        std::vector<int> test_data(input_length * num_frames, 0); // 全0符号

        auto bb_pskmod_block = bb_pskmod::make(hop_rate, M_order, Ksa_ch);
        auto source = gr::blocks::vector_source_i::make(test_data, false, input_length);
        auto head = gr::blocks::head::make(sizeof(int) * input_length, num_frames);
        auto sink = gr::blocks::vector_sink_c::make(output_length, 1024);

        auto tb = gr::make_top_block("test_bpsk");
        tb->connect(source, 0, head, 0);
        tb->connect(head, 0, bb_pskmod_block, 0);
        tb->connect(bb_pskmod_block, 0, sink, 0);
        tb->run();

        auto output_data = sink->data();
        BOOST_CHECK_EQUAL(output_data.size(), num_frames * output_length);
        std::cout << "BPSK test: generated " << output_data.size() << " samples from " << num_frames << " frames" << std::endl;
    }

    // 测试 QPSK
    {
        int M_order = 4;
        int input_length = bb_pskmod_impl::calculate_input_length(hop_rate);
        int output_length = bb_pskmod_impl::calculate_output_length(hop_rate, Ksa_ch);

        std::vector<int> test_data(input_length * num_frames);
        for (int i = 0; i < test_data.size(); i++) {
            test_data[i] = i % M_order;
        }

        auto bb_pskmod_block = bb_pskmod::make(hop_rate, M_order, Ksa_ch);
        auto source = gr::blocks::vector_source_i::make(test_data, false, input_length);
        auto head = gr::blocks::head::make(sizeof(int) * input_length, num_frames);
        auto sink = gr::blocks::vector_sink_c::make(output_length, 1024);

        auto tb = gr::make_top_block("test_qpsk");
        tb->connect(source, 0, head, 0);
        tb->connect(head, 0, bb_pskmod_block, 0);
        tb->connect(bb_pskmod_block, 0, sink, 0);
        tb->run();

        auto output_data = sink->data();
        BOOST_CHECK_EQUAL(output_data.size(), num_frames * output_length);
        std::cout << "QPSK test: generated " << output_data.size() << " samples from " << num_frames << " frames" << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_bb_pskmod_with_slot_frame)
{
    // 测试与 slot_frame 块的连接
    int hop_rate = 5;
    int M_order = 4;
    int Ksa_ch = 4;
    int info_seed = 12345;
    int num_frames = 3;    // 只处理3帧

    // 计算向量长度
    int slot_frame_output_length = slot_frame_impl::cal_vector_len(FSY_CH_HOP, hop_rate);
    int bb_pskmod_output_length = bb_pskmod_impl::calculate_output_length(hop_rate, Ksa_ch);

    // 创建块实例
    auto slot_frame_block = slot_frame::make(hop_rate, M_order, info_seed);
    auto head1 = gr::blocks::head::make(sizeof(int) * slot_frame_output_length, num_frames);
    auto bb_pskmod_block = bb_pskmod::make(hop_rate, M_order, Ksa_ch);
    auto head2 = gr::blocks::head::make(sizeof(gr_complex) * bb_pskmod_output_length, num_frames);
    auto sink = gr::blocks::vector_sink_c::make(bb_pskmod_output_length, 1024);

    // 创建流图
    auto tb = gr::make_top_block("test_chain");
    tb->connect(slot_frame_block, 0, head1, 0);
    tb->connect(head1, 0, bb_pskmod_block, 0);
    tb->connect(bb_pskmod_block, 0, head2, 0);
    tb->connect(head2, 0, sink, 0);

    // 运行流图
    tb->run();

    // 获取输出数据
    auto output_data = sink->data();

    // 验证输出
    BOOST_CHECK(!output_data.empty());
    BOOST_CHECK_EQUAL(output_data.size(), num_frames * bb_pskmod_output_length);

    std::cout << "Chain test: generated " << output_data.size() << " complex samples from " << num_frames << " frames" << std::endl;

    // 检查输出是否包含非零值
    bool has_non_zero = false;
    for (const auto& sample : output_data) {
        if (std::abs(sample) > 1e-6) {
            has_non_zero = true;
            break;
        }
    }
    BOOST_CHECK(has_non_zero);
}

BOOST_AUTO_TEST_CASE(test_bb_pskmod_frame_length_calculation)
{
    // 测试帧长度计算函数
    int hop_rate = 5;
    int Ksa_ch = 4;

    int input_length = bb_pskmod_impl::calculate_input_length(hop_rate);
    int output_length = bb_pskmod_impl::calculate_output_length(hop_rate, Ksa_ch);

    std::cout << "Frame length calculation:" << std::endl;
    std::cout << "  Input length: " << input_length << std::endl;
    std::cout << "  Output length: " << output_length << std::endl;

    // 验证长度为正数
    BOOST_CHECK_GT(input_length, 0);
    BOOST_CHECK_GT(output_length, 0);

    // 验证输出长度是输入长度的倍数（由于过采样）
    BOOST_CHECK_GE(output_length, input_length * Ksa_ch);
}

} /* namespace freq_hopping */
} /* namespace gr */