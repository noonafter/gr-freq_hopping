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
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/top_block.h>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <vector>
#include <complex>

// 包含你的模块头文件
#include <gnuradio/freq_hopping/symbol_recover.h>
#include "symbol_recover_impl.h"

namespace gr {
namespace freq_hopping {

BOOST_AUTO_TEST_CASE(test_symbol_recover_sync_and_phase)
{
    // 1. 设置参数
    int sps = 4;
    double test_phase = M_PI / 4.0; // 45度相位偏移
    
    // 2. 构造输入数据：16个全为 (1,0) 的复数
    std::vector<gr_complex> in_data(16, gr_complex(1.0, 0.0));
    
    // 3. 构造标签：在第5个采样点 (offset=4) 处触发同步
    std::vector<tag_t> tags;
    tag_t t;
    t.offset = 4; 
    t.key = pmt::mp("phase_est");
    t.value = pmt::from_double(test_phase);
    t.srcid = pmt::mp("test_src");
    tags.push_back(t);

    // 4. 创建块
    auto src = blocks::vector_source_c::make(in_data, false, 1, tags);
    auto recover = symbol_recover::make(sps);
    auto sink = blocks::vector_sink_c::make();

    // 5. 构建流图并运行
    auto tb = gr::make_top_block("test_sync");
    tb->connect(src, 0, recover, 0);
    tb->connect(recover, 0, sink, 0);
    tb->run();

    // 6. 验证结果
    auto out_data = sink->data();

    // 预期逻辑：
    // offset 4 触发第一次输出
    // 随后每 4 个点输出一次：offset 8, 12 (16停止)
    // 总共应该有 3 个输出点
    BOOST_CHECK_EQUAL(out_data.size(), 3);

    // 验证相位纠正：输入(1,0) 纠正后应为 exp(-j * test_phase)
    gr_complex expected_val = std::exp(gr_complex(0.0, -test_phase));
    
    if (!out_data.empty()) {
        BOOST_CHECK_CLOSE(out_data[0].real(), expected_val.real(), 1e-4f);
        BOOST_CHECK_CLOSE(out_data[0].imag(), expected_val.imag(), 1e-4f);
        
        // 验证后续点的相位保持一致
        BOOST_CHECK_CLOSE(out_data[1].real(), expected_val.real(), 1e-4f);
        BOOST_CHECK_CLOSE(out_data[2].imag(), expected_val.imag(), 1e-4f);
    }
}

BOOST_AUTO_TEST_CASE(test_symbol_recover_multi_tag_reset)
{
    // 测试：当标签多次出现时，计数器是否重新对齐
    int sps = 4;
    std::vector<gr_complex> in_data(21, gr_complex(1.0, 0.0)); // 将21改为20,sink只有4个点，可以详细研究gerenal_work的机制
    std::vector<tag_t> tags;

    // 标签 1: offset 2
    tag_t t1;
    t1.offset = 2;
    t1.key = pmt::mp("phase_est");
    t1.value = pmt::from_double(0.0);
    tags.push_back(t1);

    // 标签 2: offset 5 (这不是 sps 的倍数，应该强制重置计数器)
    tag_t t2;
    t2.offset = 5;
    t2.key = pmt::mp("phase_est");
    t2.value = pmt::from_double(M_PI / 2.0); // 变为 90度纠正
    tags.push_back(t2);

    auto src = blocks::vector_source_c::make(in_data, false, 1, tags);
    auto recover = symbol_recover::make(sps);
    auto sink = blocks::vector_sink_c::make();

    auto tb = gr::make_top_block("test_multi_tag");
    tb->connect(src, 0, recover, 0);
    tb->connect(recover, 0, sink, 0);
    tb->run();

    auto out_data = sink->data();

    // 预期输出位置分析：
    // 1. offset 2 (由 t1 触发) -> 纠正 0度
    // 2. offset 5 (由 t2 触发，重置) -> 纠正 90度 (out[1])
    // 3. offset 9 (5 + 4)
    // 4. offset 13 (9 + 4)
    // 5. offset 17 (13 + 4)
    BOOST_CHECK_EQUAL(out_data.size(), 5);

    if (out_data.size() >= 2) {
        // 第一个点相位应接近 (1, 0)
        BOOST_CHECK_CLOSE(out_data[0].real(), 1.0f, 1e-4f);
        // 第二个点相位应接近 (0, -1) [因为纠正了 90度]
        BOOST_CHECK_SMALL(out_data[1].real(), 1e-4f);
        BOOST_CHECK_CLOSE(out_data[1].imag(), -1.0f, 1e-4f);
    }
}

} /* namespace freq_hopping */
} /* namespace gr */