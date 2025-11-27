/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "hop_demod_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace freq_hopping {

using input_type = gr_complex;
using output_type = gr_complex;
hop_demod::sptr hop_demod::make(
    double bw_hop, double ch_sep, double freq_carrier, double fsa_hop, double hop_rate)
{
    return gnuradio::make_block_sptr<hop_demod_impl>(
        bw_hop, ch_sep, freq_carrier, fsa_hop, hop_rate);
}


/*
 * The private constructor
 */
hop_demod_impl::hop_demod_impl(
    double bw_hop, double ch_sep, double freq_carrier, double fsa_hop, double hop_rate)
    : gr::sync_block("hop_demod",
                     gr::io_signature::make(1, 1, sizeof(input_type)),
                     gr::io_signature::make(1, 1, sizeof(output_type))),
      d_bw_hop(bw_hop),
      d_ch_sep(ch_sep),
      d_freq_carrier(freq_carrier),
      d_fsa_hop(fsa_hop),
      d_hop_rate(hop_rate),
      d_hop_period(1.0 / hop_rate),
      d_samples_per_hop(d_hop_period * fsa_hop),
      d_nco(nullptr),
      d_has_time_reference(false),
      d_ref_slot_idx(0),
      d_hop_count(0),
      d_elapsed_samples(0),
      d_current_freq(0)
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
    if (d_hop_rate <= 0) {
        throw std::invalid_argument("hop_rate must be positive");
    }

    // 初始化频率表（与发送端相同）
    initialize_frequency_table();

    // 初始化跳频序列（与发送端相同）
    initialize_hop_sequence();

    // 初始化随机数生成器（与发送端相同的种子）
    d_rng = std::mt19937(42);

    // 创建NCO用于下混频
    d_nco = nco_crcf_create(LIQUID_VCO);

    std::cout << "Hop Demod initialized: " << d_num_ch << " channels, "
              << "hop rate: " << d_hop_rate << " hops/s, "
              << "sample rate: " << d_fsa_hop << " Hz, "
              << "samples per hop: " << d_samples_per_hop << std::endl;
}

/*
 * Our virtual destructor.
 */
hop_demod_impl::~hop_demod_impl()
{
    if (d_nco) {
        nco_crcf_destroy(d_nco);
    }
}

void hop_demod_impl::initialize_frequency_table()
{
    // 计算信道数量（必须与发送端相同）
    d_num_ch = static_cast<int>(std::floor(d_bw_hop / d_ch_sep));
    if (d_num_ch < 1) {
        d_num_ch = 1;
    }

    // 生成频率表（必须与发送端相同）
    d_freq_vec.resize(d_num_ch);
    for (int i = 0; i < d_num_ch; ++i) {
        d_freq_vec[i] = (i - std::floor(d_num_ch / 2.0)) * d_ch_sep + d_freq_carrier;
    }
}

void hop_demod_impl::initialize_hop_sequence()
{
    // 生成跳频序列（必须与发送端相同）
    int sequence_length = d_num_ch * 2;
    d_hop_sequence.resize(sequence_length);

    // 使用相同的随机数生成器和分布
    std::uniform_int_distribution<int> dist(0, d_num_ch - 1);

    for (int i = 0; i < sequence_length; ++i) {
        d_hop_sequence[i] = dist(d_rng);
    }

    // // 输出跳频序列用于调试
    // std::cout << "Hop sequence: ";
    // for (int i = 0; i < sequence_length; ++i) {
    //     std::cout << d_hop_sequence[i] << " ";
    // }
    std::cout << std::endl;
}


int hop_demod_impl::work(int noutput_items,
                         gr_vector_const_void_star& input_items,
                         gr_vector_void_star& output_items)
{
    auto in = static_cast<const input_type*>(input_items[0]);
    auto out = static_cast<output_type*>(output_items[0]);

    const uint64_t nitems_passed = nitems_read(0);

    // 检查rx_time标签
    std::vector<tag_t> tags;
    get_tags_in_range(tags, 0, nitems_passed, nitems_passed + noutput_items, pmt::string_to_symbol("rx_time"));

    for (const auto& tag : tags) {
        if (pmt::is_tuple(tag.value)) {
            // 解析rx_time标签
            uint64_t sec = pmt::to_uint64(pmt::tuple_ref(tag.value, 0));
            double frac_sec = pmt::to_double(pmt::tuple_ref(tag.value, 1));

            // 计算距离当天0点的纳秒数
            const uint64_t nanoseconds_per_day = 24 * 3600 * 1000000000ULL;
            uint64_t rx_time_ns_since_midnight = (sec % (24 * 3600)) * 1000000000ULL +
                                                static_cast<uint64_t>(frac_sec * 1e9);

            // 计算时隙大小（纳秒）
            uint64_t slot_size_ns = static_cast<uint64_t>(d_hop_period * 1e9);

            // 计算当前slot索引
            d_ref_slot_idx = rx_time_ns_since_midnight / slot_size_ns;

            // 计算当前slot的起始纳秒值
            uint64_t ref_slot_ns = d_ref_slot_idx * slot_size_ns;

            // 初始化状态
            d_hop_count = 0;
            d_elapsed_samples = (rx_time_ns_since_midnight - ref_slot_ns) * d_fsa_hop / 1e9;
            d_has_time_reference = true;

            // 计算初始频率
            unsigned hop_seq_idx = (d_ref_slot_idx + d_hop_count) % d_hop_sequence.size();
            int freq_index = d_hop_sequence[hop_seq_idx];
            d_current_freq = d_freq_vec[freq_index];
            nco_crcf_set_frequency(d_nco, -2 * M_PI * d_current_freq / d_fsa_hop);

            std::cout << "RX: FIRST HOP: ref_slot_ns=" << ref_slot_ns
                      << ", seq_idx=" << hop_seq_idx
                      << ", elapsed_samples=" << d_elapsed_samples << std::endl;
        }
    }

    // 如果没有时间参考，直接复制数据
    if (!d_has_time_reference) {
        memcpy(out, in, noutput_items * sizeof(gr_complex));
        return noutput_items;
    }

    // 逐个样本处理
    for (int i = 0; i < noutput_items; ++i) {
        // 检查是否需要切换频率
        if (d_elapsed_samples >= d_samples_per_hop) {
            d_elapsed_samples -= d_samples_per_hop;
            d_hop_count++;

            // 计算新的频率索引
            unsigned hop_seq_idx = (d_ref_slot_idx + d_hop_count) % d_hop_sequence.size();
            int freq_index = d_hop_sequence[hop_seq_idx];
            d_current_freq = d_freq_vec[freq_index];
            nco_crcf_set_frequency(d_nco, 2 * M_PI * d_current_freq / d_fsa_hop);

            // //输出调试信息（可选，频率切换时输出）
            // std::cout << "Rx: Hop changed: hop_seq_idx=" << hop_seq_idx
            //           << ", freq=" << d_current_freq << " Hz" << std::endl;
        }

        // 下混频当前样本
        gr_complex sample;
        nco_crcf_step(d_nco);
        nco_crcf_mix_down(d_nco, in[i], &sample);
        out[i] = sample;

        // 更新已处理样本数
        d_elapsed_samples += 1.0;
    }

    return noutput_items;
}

} /* namespace freq_hopping */
} /* namespace gr */
