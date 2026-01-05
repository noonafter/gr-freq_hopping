/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_SYMBOL_RECOVER_IMPL_H
#define INCLUDED_FREQ_HOPPING_SYMBOL_RECOVER_IMPL_H

#include <gnuradio/freq_hopping/symbol_recover.h>

namespace gr {
namespace freq_hopping {

class symbol_recover_impl : public symbol_recover
{
private:
    int d_sps;             // 每符号采样点数
    int d_counter;         // 采样点计数器
    float d_phase_corr;    // 当前相位补偿值
    bool d_is_synced;      // 是否已实现初始同步
    pmt::pmt_t d_tag_key;  // 目标标签名称 "phase_est"

    uint64_t last_tag_offset;
    float last_tag_value;

public:
    symbol_recover_impl(int sps);
    ~symbol_recover_impl();

    // Where all the action really happens
    void forecast(int noutput_items, gr_vector_int& ninput_items_required);

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items);
};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_SYMBOL_RECOVER_IMPL_H */
