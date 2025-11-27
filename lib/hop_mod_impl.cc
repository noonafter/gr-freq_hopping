/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "hop_mod_impl.h"
#include <gnuradio/io_signature.h>
// #include <gnuradio/uhd/usrp/multi_usrp.hpp>

const std::string TX_TIME_TAG_KEY = "tx_time";

namespace gr {
namespace freq_hopping {

using input_type = gr_complex;
using output_type = gr_complex;
hop_mod::sptr hop_mod::make(double bw_hop, double ch_sep, double freq_carrier, double fsa_hop, double hop_rate, int vlen)
{
    return gnuradio::make_block_sptr<hop_mod_impl>(bw_hop, ch_sep, freq_carrier, fsa_hop, hop_rate, vlen);
}


/*
 * The private constructor
 */
hop_mod_impl::hop_mod_impl(double bw_hop, double ch_sep, double freq_carrier, double fsa_hop, double hop_rate, int vlen)
    : gr::sync_block("hop_mod",
                     gr::io_signature::make(1, 1, vlen*sizeof(input_type)),
                     gr::io_signature::make(1, 1, vlen*sizeof(output_type))),
    d_bw_hop(bw_hop),
    d_ch_sep(ch_sep),
    d_freq_carrier(freq_carrier),
    d_fsa_hop(fsa_hop),
    d_hop_rate(hop_rate),
    d_vlen(vlen),
    d_hop_period(1.0/d_hop_rate),
    d_hop_count(0),
    d_first_hop(true),
    d_start_time(0),
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
    if (d_vlen <= 0) {
        throw std::invalid_argument("vlen must be positive");
    }

    // 初始化频率表
    initialize_frequency_table();


    // 初始化跳频序列
    initialize_hop_sequence();

    // 初始化随机数生成器
    // std::random_device rd;
    d_rng = std::mt19937(42);
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

}

void hop_mod_impl::initialize_hop_sequence()
{
    // 生成跳频序列 - 长度为 num_channels * 2 的随机序列
    int sequence_length = d_num_ch * 2;
    d_hop_sequence.resize(sequence_length);

    // 使用均匀分布生成随机索引
    std::uniform_int_distribution<int> dist(0, d_num_ch - 1);

    for (int i = 0; i < sequence_length; ++i) {
        d_hop_sequence[i] = dist(d_rng);
    }
}

double hop_mod_impl::get_frequency_by_hop_count()
{
    // 基于跳频计数器计算频率
    int freq_index = d_hop_sequence[d_hop_count % d_hop_sequence.size()];
    return d_freq_vec[freq_index];
}

std::pair<uint64_t, double> hop_mod_impl::get_current_usrp_time()
{
    // 获取当前时间（模拟 USRP 时间）
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch(); // ns

    // 转换为秒
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto fractional = duration - seconds;
    double fractional_seconds = std::chrono::duration<double>(fractional).count();

    return std::make_pair(seconds.count(), fractional_seconds);
}

uint64_t hop_mod_impl::align_to_time_slot(uint64_t current_time_ns)
{
    // 计算从当天0点开始的纳秒数
    const uint64_t nanoseconds_per_day = 24 * 3600 * 1000000000ULL;
    uint64_t time_since_midnight = current_time_ns % nanoseconds_per_day;

    // 计算时隙大小（纳秒）
    uint64_t slot_size_ns = static_cast<uint64_t>(d_hop_period * 1e9);

    // 当前slot的结尾时刻的编号，即下一slot的开始时刻的编号
    uint64_t current_slot_end_idx = (time_since_midnight+slot_size_ns) / slot_size_ns;

    // 真实发送时刻的编号，在current_slot_end基础上+1,是为了至少留1个slot处理
    uint64_t real_tx_slot_idx = current_slot_end_idx + 1;

    // 这一段很重要！需要按照真实发送时刻的编号来初始化d_hop_count
    // 后续依次走。这样接收端就能知道任意时刻的freq_tab
    d_hop_count = real_tx_slot_idx % (d_hop_sequence.size());
    std::cout << "TX: FIRST HOP: idx: " << d_hop_count << std::endl;

    // 下一个时隙的开始时间（从当天0点开始）
    uint64_t real_tx_start = real_tx_slot_idx * slot_size_ns;
    std::cout << "TX: FIRST HOP: real_tx_start: " << real_tx_start << std::endl;

    // 计算绝对时间（从epoch开始）
    uint64_t base_time = current_time_ns - time_since_midnight; // today 0:0:0
    uint64_t aligned_time = base_time + real_tx_start;

    return aligned_time;
}


int hop_mod_impl::work(int noutput_items,
                       gr_vector_const_void_star& input_items,
                       gr_vector_void_star& output_items)
{
    auto in = static_cast<const input_type*>(input_items[0]);
    auto out = static_cast<output_type*>(output_items[0]);

    // 如果是第一跳，初始化起始时间
    if (d_first_hop) {
        // 获取当前 USRP 时间（纳秒）
        auto current_time = get_current_usrp_time();
        uint64_t current_time_ns = current_time.first * 1000000000ULL +
                                  static_cast<uint64_t>(current_time.second * 1e9);

        // 对齐到下一个时隙开始
        d_start_time = align_to_time_slot(current_time_ns);

        d_first_hop = false;
        // std::cout << "current_time_ns: " << current_time_ns << std::endl;
        // std::cout << "d_start_time: " << d_start_time << std::endl;

        // 转换为秒，并拆分为整数部分和小数部分
        uint64_t integer_sec = d_start_time / 1000000000ULL;
        double fractional_sec = (d_start_time % 1000000000ULL) / 1e9;

        // 创建时间元组
        pmt::pmt_t time_tuple = pmt::make_tuple(
            pmt::from_uint64(integer_sec),
            pmt::from_double(fractional_sec)
        );

        // 添加 tx_time 标签
        this->add_item_tag(0,
                          this->nitems_written(0),
                          pmt::string_to_symbol(TX_TIME_TAG_KEY),
                          time_tuple);


        // std::cout << "TX: first hop time: " << d_start_time
        //           << " ns (" << integer_sec << " + " << fractional_sec << " s)" << std::endl;
    }

    // 处理每一帧
    int idx_vec = 0;
    for (; idx_vec < noutput_items; ++idx_vec) {
        // 获取当前帧的输入和输出位置
        const input_type* frame_in = in + idx_vec * d_vlen;
        output_type* frame_out = out + idx_vec * d_vlen;

        // 为当前帧选择频率
        double freq_tb = get_frequency_by_hop_count();
        // 设置 NCO 频率
        nco_crcf_set_phase(d_nco, 0);
        nco_crcf_set_frequency(d_nco, 2 * M_PI * freq_tb / d_fsa_hop);

        // 使用 NCO 进行频率调制（上混频）
        nco_crcf_mix_block_up(d_nco, const_cast<input_type*>(frame_in), frame_out, d_vlen);

        // 增加跳频计数器
        d_hop_count++;
    }

    // Tell runtime system how many output items we produced.
    return idx_vec;
}

} /* namespace freq_hopping */
} /* namespace gr */
