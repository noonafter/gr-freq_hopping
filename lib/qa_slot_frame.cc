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
#include <gnuradio/freq_hopping/slot_frame.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/top_block.h>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <vector>
#include <gnuradio/blocks/head.h>
#include "slot_frame_impl.h"

namespace gr {
namespace freq_hopping {

BOOST_AUTO_TEST_CASE(test_slot_frame_basic)
{
    // 测试参数
    int hop_rate = 5;      // 5 hops/s
    int M_order = 4;       // QPSK-like
    int info_seed = 12345; // 随机种子

    int expected_length = slot_frame_impl::cal_vector_len(FSY_CH_HOP, hop_rate);
    // 创建块实例
    auto slot_frame_block = slot_frame::make(hop_rate, M_order, info_seed);

    // 创建head块，只取10个向量
    auto head = gr::blocks::head::make(sizeof(int) * expected_length, 10);

    // 创建向量接收器
    auto sink = gr::blocks::vector_sink_i::make(expected_length, 1024);

    // 创建流图
    auto tb = gr::make_top_block("test");
    tb->connect(slot_frame_block, 0, head, 0);
    tb->connect(head, 0, sink, 0);

    // 运行流图
    tb->run();

    // 获取输出数据
    auto output_data = sink->data();

    // 基本验证
    BOOST_CHECK(!output_data.empty());
    BOOST_CHECK_EQUAL(output_data.size(), 10 * expected_length); // 10个向量，每个432个int

    std::cout << "Generated " << output_data.size() << " samples" << std::endl;
}

BOOST_AUTO_TEST_CASE(test_slot_frame_reproducibility)
{
    // 测试可重现性 - 相同参数应该产生相同输出
    int hop_rate = 10;
    int M_order = 8;
    int info_seed = 54321;

    // 第一次运行
    int expected_length = slot_frame_impl::cal_vector_len(FSY_CH_HOP, hop_rate);
    auto block1 = slot_frame::make(hop_rate, M_order, info_seed);
    auto head1 = gr::blocks::head::make(sizeof(int) * expected_length, 2);
    auto sink1 = gr::blocks::vector_sink_i::make(expected_length, 1024);
    auto tb1 = gr::make_top_block("test1");
    tb1->connect(block1, 0, head1, 0);
    tb1->connect(head1, 0, sink1, 0);
    tb1->run();
    auto data1 = sink1->data();

    // 第二次运行（相同参数）
    auto block2 = slot_frame::make(hop_rate, M_order, info_seed);
    auto head2 = gr::blocks::head::make(sizeof(int) * expected_length, 2);
    auto sink2 = gr::blocks::vector_sink_i::make(expected_length, 1024);
    auto tb2 = gr::make_top_block("test2");
    tb2->connect(block2, 0, head2, 0);
    tb2->connect(head2, 0, sink2, 0);
    tb2->run();
    auto data2 = sink2->data();

    // 验证输出相同
    BOOST_CHECK_EQUAL(data1.size(), data2.size());

    if (data1.size() == data2.size()) {
        for (size_t i = 0; i < data1.size(); i++) {
            BOOST_CHECK_EQUAL(data1[i], data2[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_slot_frame_value_range)
{
    // 测试输出值在正确范围内
    int hop_rate = 20;
    int M_order = 16;  // 值应该在 0-15 范围内
    int info_seed = 999;

    int expected_length = slot_frame_impl::cal_vector_len(FSY_CH_HOP, hop_rate);
    auto block = slot_frame::make(hop_rate, M_order, info_seed);
    auto head = gr::blocks::head::make(sizeof(int) * expected_length, 2);
    auto sink = gr::blocks::vector_sink_i::make(expected_length, 1024);
    auto tb = gr::make_top_block("test_range");
    tb->connect(block, 0, head, 0);
    tb->connect(head, 0, sink, 0);
    tb->run();

    auto data = sink->data();

    // 验证所有值都在 0 到 M_order-1 范围内
    for (const auto& value : data) {
        BOOST_CHECK_GE(value, 0);
        BOOST_CHECK_LT(value, M_order);
    }

    std::cout << "All " << data.size() << " values are in range [0, " << (M_order-1) << "]" << std::endl;
}

} /* namespace freq_hopping */
} /* namespace gr */