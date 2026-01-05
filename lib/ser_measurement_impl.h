/* -*- c++ -*- */
/*
 * Copyright 2026 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_SER_MEASUREMENT_IMPL_H
#define INCLUDED_FREQ_HOPPING_SER_MEASUREMENT_IMPL_H

#include <gnuradio/freq_hopping/ser_measurement.h>

namespace gr {
namespace freq_hopping {

class ser_measurement_impl : public ser_measurement
{
private:
    static constexpr size_t HISTORY_SIZE = 100;      // 保存最近N帧
    static constexpr size_t PRINT_INTERVAL = 100;    // 每N帧打印一次统计

    std::vector<char> d_reference_data;  // 参考数据（一帧）
    std::string d_filename;

    // 当前帧统计
    size_t d_current_frame_idx;          // 当前帧内的索引位置
    size_t d_current_frame_errors;       // 当前帧的错误数
    size_t d_frame_length;               // 帧长度

    // 历史统计
    std::deque<double> d_ser_history;    // 最近30帧的SER
    size_t d_total_frames;               // 总帧数



    // 读取参考文件
    bool load_reference_file();

    // 处理新帧开始，返回当前帧的SER
    double handle_new_frame();

    // 计算并更新SER
    void update_ser();

public:
    ser_measurement_impl(const std::string& filename);
    ~ser_measurement_impl();

    // Where all the action really happens
    void forecast(int noutput_items, gr_vector_int& ninput_items_required);

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items);
};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_SER_MEASUREMENT_IMPL_H */
