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
#include <gnuradio/freq_hopping/hop_mod.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

namespace gr {
namespace freq_hopping {

BOOST_AUTO_TEST_CASE(test_hop_mod_basic)
{
    // 测试参数
    double bw_hop = 1e6;          // 1 MHz 跳频带宽
    double ch_sep = 100e3;        // 100 kHz 信道间隔
    double freq_carrier = 4e6;    // 4 MHz 载波频率
    double fsa_hop = 10e6;        // 10 MHz 采样率
    double hop_rate = 5;
    int vlen = 1000;              // 向量长度
    int num_frames = 3;           // 测试3帧

    // 创建测试输入数据 - 常数信号（便于验证）
    std::vector<gr_complex> test_data(vlen*num_frames, gr_complex(1.0f, 0.0f));

    // 创建块实例
    auto hop_mod_block = hop_mod::make(bw_hop, ch_sep, freq_carrier, fsa_hop, hop_rate, vlen);
    auto source = gr::blocks::vector_source_c::make(test_data, false, vlen);
    auto head = gr::blocks::head::make(sizeof(gr_complex) * vlen, num_frames);
    auto sink = gr::blocks::vector_sink_c::make(vlen, 1024);

    // 创建流图
    auto tb = gr::make_top_block("test_hop_mod");
    tb->connect(source, 0, head, 0);
    tb->connect(head, 0, hop_mod_block, 0);
    tb->connect(hop_mod_block, 0, sink, 0);

    // 运行流图
    tb->run();

    // 获取输出数据
    auto output_data = sink->data();

    // 基本验证
    BOOST_CHECK(!output_data.empty());
    BOOST_CHECK_EQUAL(output_data.size(), num_frames * vlen);

    std::cout << "Generated " << output_data.size() << " complex samples from " << num_frames << " frames" << std::endl;

    // 检查输出是否与输入不同（因为进行了频率调制）
    bool output_different = false;
    for (const auto& sample : output_data) {
        if (std::abs(sample - gr_complex(1.0f, 0.0f)) > 1e-6) {
            output_different = true;
            break;
        }
    }
    BOOST_CHECK(output_different);
}

BOOST_AUTO_TEST_CASE(test_hop_mod_frequency_table)
{
    // 测试频率表生成
    double bw_hop = 1e6;          // 1 MHz 跳频带宽
    double ch_sep = 200e3;        // 200 kHz 信道间隔
    double freq_carrier = 900e6;  // 900 MHz 载波频率
    double fsa_hop = 10e6;        // 10 MHz 采样率
    double hop_rate = 50;
    int vlen = 100;

    // 创建块实例
    auto hop_mod_block = hop_mod::make(bw_hop, ch_sep, freq_carrier, fsa_hop, hop_rate, vlen);

    // 创建测试数据
    std::vector<gr_complex> test_data(vlen, gr_complex(1.0f, 0.0f));
    auto source = gr::blocks::vector_source_c::make(test_data, false, vlen);
    auto head = gr::blocks::head::make(sizeof(gr_complex) * vlen, 1);
    auto sink = gr::blocks::vector_sink_c::make(vlen, 1024);

    // 创建流图并运行
    auto tb = gr::make_top_block("test_freq_table");
    tb->connect(source, 0, head, 0);
    tb->connect(head, 0, hop_mod_block, 0);
    tb->connect(hop_mod_block, 0, sink, 0);
    tb->run();

    auto output_data = sink->data();

    // 验证输出
    BOOST_CHECK_EQUAL(output_data.size(), vlen);
    std::cout << "Frequency table test: generated " << output_data.size() << " samples" << std::endl;
}

BOOST_AUTO_TEST_CASE(test_hop_mod_different_parameters)
{
    // 测试不同参数配置
    int vlen = 500;
    int num_frames = 2;

    // 测试配置1：窄带宽，大间隔
    {
        double bw_hop = 500e3;        // 500 kHz
        double ch_sep = 100e3;        // 100 kHz
        double freq_carrier = 2.4e9;  // 2.4 GHz
        double fsa_hop = 5e6;         // 5 MHz
        double hop_rate = 50;


        std::vector<gr_complex> test_data(vlen*num_frames, gr_complex(0.5f, 0.5f));

        auto hop_mod_block = hop_mod::make(bw_hop, ch_sep, freq_carrier, fsa_hop, hop_rate, vlen);
        auto source = gr::blocks::vector_source_c::make(test_data, false, vlen);
        auto head = gr::blocks::head::make(sizeof(gr_complex) * vlen, num_frames);
        auto sink = gr::blocks::vector_sink_c::make(vlen, 1024);

        auto tb = gr::make_top_block("test_params1");
        tb->connect(source, 0, head, 0);
        tb->connect(head, 0, hop_mod_block, 0);
        tb->connect(hop_mod_block, 0, sink, 0);
        tb->run();

        auto output_data = sink->data();
        BOOST_CHECK_EQUAL(output_data.size(), num_frames * vlen);
        std::cout << "Parameter set 1: generated " << output_data.size() << " samples" << std::endl;
    }

    // 测试配置2：宽带宽，小间隔
    {
        double bw_hop = 10e6;         // 10 MHz
        double ch_sep = 50e3;         // 50 kHz
        double freq_carrier = 5.8e9;  // 5.8 GHz
        double fsa_hop = 20e6;        // 20 MHz
        double hop_rate = 50;

        std::vector<gr_complex> test_data(vlen*num_frames, gr_complex(0.0f, 1.0f));

        auto hop_mod_block = hop_mod::make(bw_hop, ch_sep, freq_carrier, fsa_hop, hop_rate, vlen);
        auto source = gr::blocks::vector_source_c::make(test_data, false, vlen);
        auto head = gr::blocks::head::make(sizeof(gr_complex) * vlen, num_frames);
        auto sink = gr::blocks::vector_sink_c::make(vlen, 1024);

        auto tb = gr::make_top_block("test_params2");
        tb->connect(source, 0, head, 0);
        tb->connect(head, 0, hop_mod_block, 0);
        tb->connect(hop_mod_block, 0, sink, 0);
        tb->run();

        auto output_data = sink->data();
        BOOST_CHECK_EQUAL(output_data.size(), num_frames * vlen);
        std::cout << "Parameter set 2: generated " << output_data.size() << " samples" << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_hop_mod_with_modulated_input)
{
    // 测试使用调制后的输入信号（模拟来自 bb_pskmod 的输出）
    double bw_hop = 2e6;
    double ch_sep = 100e3;
    double freq_carrier = 1e9;
    double fsa_hop = 8e6;
    double hop_rate = 50;
    int vlen = 800;
    int num_frames = 2;

    // 创建调制输入信号（类似 BPSK 调制）
    std::vector<gr_complex> test_data(vlen*num_frames);
    for (int i = 0; i < vlen*num_frames; ++i) {
        // 简单的 BPSK 信号：+1 和 -1 交替
        float symbol = (i / 10) % 2 == 0 ? 1.0f : -1.0f;
        test_data[i] = gr_complex(symbol, 0.0f);
    }

    auto hop_mod_block = hop_mod::make(bw_hop, ch_sep, freq_carrier, fsa_hop, hop_rate, vlen);
    auto source = gr::blocks::vector_source_c::make(test_data, false, vlen);
    auto head = gr::blocks::head::make(sizeof(gr_complex) * vlen, num_frames);
    auto sink = gr::blocks::vector_sink_c::make(vlen, 1024);

    auto tb = gr::make_top_block("test_modulated_input");
    tb->connect(source, 0, head, 0);
    tb->connect(head, 0, hop_mod_block, 0);
    tb->connect(hop_mod_block, 0, sink, 0);
    tb->run();

    auto output_data = sink->data();

    // 验证输出
    BOOST_CHECK_EQUAL(output_data.size(), num_frames * vlen);

    // 检查输出是否包含复数信号（频率调制后）
    bool has_complex_output = false;
    for (const auto& sample : output_data) {
        if (std::abs(std::imag(sample)) > 1e-6) {
            has_complex_output = true;
            break;
        }
    }
    BOOST_CHECK(has_complex_output);

    std::cout << "Modulated input test: generated " << output_data.size() << " complex samples" << std::endl;
}

BOOST_AUTO_TEST_CASE(test_hop_mod_frame_consistency)
{
    // 测试帧处理的一致性
    double bw_hop = 1e6;
    double ch_sep = 100e3;
    double freq_carrier = 900e6;
    double fsa_hop = 10e6;
    double hop_rate = 50;
    int vlen = 200;
    int num_frames = 5;

    // 创建测试数据
    std::vector<gr_complex> test_data(vlen * num_frames);
    for (int i = 0; i < test_data.size(); ++i) {
        test_data[i] = gr_complex(std::cos(i * 0.1f), std::sin(i * 0.1f));
    }

    auto hop_mod_block = hop_mod::make(bw_hop, ch_sep, freq_carrier, fsa_hop, hop_rate, vlen);
    auto source = gr::blocks::vector_source_c::make(test_data, false, vlen);
    auto sink = gr::blocks::vector_sink_c::make(vlen, 1024);

    auto tb = gr::make_top_block("test_frame_consistency");
    tb->connect(source, 0, hop_mod_block, 0);
    tb->connect(hop_mod_block, 0, sink, 0);
    tb->run();

    auto output_data = sink->data();

    // 验证输出帧数量正确
    BOOST_CHECK_EQUAL(output_data.size(), num_frames * vlen);

    // 检查每帧都有输出（没有丢失帧）
    BOOST_CHECK(output_data.size() > 0);

    std::cout << "Frame consistency test: processed " << num_frames << " frames, output " << output_data.size() << " samples" << std::endl;
}

} /* namespace freq_hopping */
} /* namespace gr */