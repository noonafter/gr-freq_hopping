/* -*- c++ -*- */
/*
 * Copyright 2026 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_FRAME_RECOVER_IMPL_H
#define INCLUDED_FREQ_HOPPING_FRAME_RECOVER_IMPL_H

#include <gnuradio/freq_hopping/frame_recover.h>

namespace gr {
namespace freq_hopping {

class frame_recover_impl : public frame_recover
{
private:
    int d_frame_len;
    int d_count;          // 剩余待输出的计数
    bool d_is_active;     // 是否处于输出激活状态
    pmt::pmt_t d_tag_key; // 缓存 phase_est 的 key

public:
    frame_recover_impl(int frame_len);
    ~frame_recover_impl();

    // Where all the action really happens
    void forecast(int noutput_items, gr_vector_int& ninput_items_required);

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items);
};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_FRAME_RECOVER_IMPL_H */
