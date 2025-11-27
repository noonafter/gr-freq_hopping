/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gnuradio/attributes.h>
#include <gnuradio/freq_hopping/hop_demod.h>
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace gr {
namespace freq_hopping {

BOOST_AUTO_TEST_CASE(test_hop_demod_initialization)
{
    std::cout << "=== Test 1: Module Initialization ===" << std::endl;

    // 测试参数
    double bw_hop = 2e6;      // 100 MHz 跳频带宽
    double ch_sep = 3e3;       // 10 MHz 信道间隔
    double freq_carrier = 0;  // 1 GHz 载波频率
    double fsa_hop = 2.4576e6;       // 1 MHz 采样率
    double hop_rate = 5;     // 1000 跳/秒

    // 创建模块
    auto demod = hop_demod::make(bw_hop, ch_sep, freq_carrier, fsa_hop, hop_rate);

    // 验证模块创建成功
    BOOST_CHECK(demod != nullptr);

    // 验证输入输出签名
    BOOST_CHECK_EQUAL(demod->input_signature()->sizeof_stream_item(0), sizeof(gr_complex));
    BOOST_CHECK_EQUAL(demod->output_signature()->sizeof_stream_item(0), sizeof(gr_complex));

    std::cout << "Module initialized successfully" << std::endl;
    std::cout << "Input signature: " << demod->input_signature()->sizeof_stream_item(0) << " bytes" << std::endl;
    std::cout << "Output signature: " << demod->output_signature()->sizeof_stream_item(0) << " bytes" << std::endl;
}

BOOST_AUTO_TEST_CASE(test_hop_demod_parameter_validation)
{
    std::cout << "\n=== Test 2: Parameter Validation ===" << std::endl;

    // 测试有效参数不应该抛出异常
    BOOST_CHECK_NO_THROW(
        hop_demod::make(100e6, 10e6, 1e9, 1e6, 1000)
    );

    std::cout << "Valid parameters accepted correctly" << std::endl;
}

BOOST_AUTO_TEST_CASE(test_hop_demod_interface)
{
    std::cout << "\n=== Test 3: Interface Verification ===" << std::endl;

    // 创建模块
    double bw_hop = 100e6;
    double ch_sep = 10e6;
    double freq_carrier = 1e9;
    double fsa_hop = 1e6;
    double hop_rate = 1000;

    auto demod = hop_demod::make(bw_hop, ch_sep, freq_carrier, fsa_hop, hop_rate);

    // 验证基本接口方法
    BOOST_CHECK(demod->name().find("hop_demod") != std::string::npos);

    // 验证输入输出端口数量
    BOOST_CHECK_GE(demod->input_signature()->min_streams(), 1);
    BOOST_CHECK_GE(demod->output_signature()->min_streams(), 1);

    std::cout << "Interface verification passed" << std::endl;
    std::cout << "Block name: " << demod->name() << std::endl;
    std::cout << "Min input streams: " << demod->input_signature()->min_streams() << std::endl;
    std::cout << "Min output streams: " << demod->output_signature()->min_streams() << std::endl;
}

BOOST_AUTO_TEST_CASE(test_hop_demod_construction_destruction)
{
    std::cout << "\n=== Test 4: Construction/Destruction ===" << std::endl;

    // 测试多次创建和销毁
    for (int i = 0; i < 5; i++) {
        auto demod = hop_demod::make(100e6, 10e6, 1e9, 1e6, 1000);
        BOOST_CHECK(demod != nullptr);
    }

    std::cout << "Multiple construction/destruction cycles completed successfully" << std::endl;
}

BOOST_AUTO_TEST_CASE(test_hop_demod_different_parameters)
{
    std::cout << "\n=== Test 5: Different Parameter Sets ===" << std::endl;

    // 测试不同的参数组合
    struct TestParams {
        double bw_hop, ch_sep, freq_carrier, fsa_hop, hop_rate;
    };

    TestParams test_cases[] = {
        {50e6, 5e6, 900e6, 500e3, 500},     // 低频，低采样率
        {200e6, 20e6, 2e9, 2e6, 2000},      // 高频，高采样率
        {80e6, 8e6, 1.5e9, 800e3, 800},     // 中等参数
    };

    for (const auto& params : test_cases) {
        BOOST_CHECK_NO_THROW(
            hop_demod::make(params.bw_hop, params.ch_sep, params.freq_carrier,
                           params.fsa_hop, params.hop_rate)
        );
    }

    std::cout << "All parameter sets accepted successfully" << std::endl;
}

} /* namespace freq_hopping */
} /* namespace gr */