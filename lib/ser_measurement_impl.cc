/* -*- c++ -*- */
/*
 * Copyright 2026 <+YOU OR YOUR COMPANY+>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ser_measurement_impl.h"
#include <gnuradio/io_signature.h>
#include <fstream>
#include <iostream>
#include <numeric>

namespace gr {
namespace freq_hopping {

ser_measurement::sptr
ser_measurement::make(const std::string& filename)
{
    return gnuradio::make_block_sptr<ser_measurement_impl>(filename);
}

ser_measurement_impl::ser_measurement_impl(const std::string& filename)
    : gr::block("ser_measurement",
                gr::io_signature::make(1, 1, sizeof(char)),
                gr::io_signature::make(0, 1, sizeof(float))),  // 修改：0个或1个输出
      d_filename(filename),
      d_current_frame_idx(0),
      d_current_frame_errors(0),
      d_frame_length(0),
      d_total_frames(0)
{
    // 加载参考文件
    if (!load_reference_file()) {
        throw std::runtime_error("Failed to load reference file: " + filename);
    }

    d_frame_length = d_reference_data.size();

    // 设置标签传播策略
    set_tag_propagation_policy(TPP_DONT);

    // std::cout << "SER Measurement initialized with frame length: "
    //           << d_frame_length << std::endl;
}

ser_measurement_impl::~ser_measurement_impl()
{
}

bool ser_measurement_impl::load_reference_file()
{
    std::ifstream file(d_filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << d_filename << std::endl;
        return false;
    }

    // 读取文件到vector
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    d_reference_data.resize(file_size);
    file.read(reinterpret_cast<char*>(d_reference_data.data()), file_size);

    file.close();

    std::cout << "Loaded reference file: " << d_filename
              << " (" << file_size << " bytes)" << std::endl;

    return true;
}

double ser_measurement_impl::handle_new_frame()
{
    double current_ser = 0.0;

    // 如果当前帧有数据，计算SER
    if (d_current_frame_idx > 0) {
        current_ser = static_cast<double>(d_current_frame_errors) / d_current_frame_idx;

        // 添加到历史记录
        d_ser_history.push_back(current_ser);
        if (d_ser_history.size() > HISTORY_SIZE) {
            d_ser_history.pop_front();
        }
    }

    // 重置当前帧统计
    d_current_frame_idx = 0;
    d_current_frame_errors = 0;
    d_total_frames++;

    // 每30帧打印一次
    if (d_total_frames % PRINT_INTERVAL == 0) {
        // 计算最近30帧的平均SER
        double avg_ser = 0.0;
        if (!d_ser_history.empty()) {
            avg_ser = std::accumulate(d_ser_history.begin(),
                                     d_ser_history.end(),
                                     0.0) / d_ser_history.size();
        }

        std::cout << "========================================" << std::endl;
        std::cout << "Total Frames Received: " << d_total_frames << std::endl;
        std::cout << "Average SER (last " << d_ser_history.size()
                  << " frames): " << avg_ser << std::endl;
        // std::cout << "Current Frame SER: " << current_ser << std::endl;
    }

    return current_ser;
}

void ser_measurement_impl::forecast(int noutput_items,
                                    gr_vector_int& ninput_items_required)
{
    // 我们需要足够的输入来处理，但输出是稀疏的（只在标签处）
    // 设置一个合理的输入需求
    ninput_items_required[0] = noutput_items > 0 ? noutput_items * d_frame_length : 4096;
}

int ser_measurement_impl::general_work(int noutput_items,
                                       gr_vector_int& ninput_items,
                                       gr_vector_const_void_star& input_items,
                                       gr_vector_void_star& output_items)
{
    const char* in = static_cast<const char*>(input_items[0]);

    // 检查是否有输出连接
    bool has_output = (output_items.size() > 0);
    float* out = has_output ? static_cast<float*>(output_items[0]) : nullptr;

    int nin = ninput_items[0];
    int nout = 0;  // 实际输出的数量

    // 获取所有phase_est标签
    std::vector<tag_t> tags;
    get_tags_in_window(tags, 0, 0, nin, pmt::string_to_symbol("phase_est"));

    int consumed = 0;
    size_t tag_idx = 0;

    for (int i = 0; i < nin && (!has_output || nout < noutput_items); i++) {
        // 检查是否有phase_est标签
        if (tag_idx < tags.size() &&
            (tags[tag_idx].offset - nitems_read(0)) == static_cast<uint64_t>(i)) {

            // 新帧开始，输出上一帧的SER（如果有输出连接）
            if (has_output && (d_total_frames > 0 || d_current_frame_idx > 0)) {
                // 计算并输出平均SER
                double avg_ser = 0.0;
                if (!d_ser_history.empty()) {
                    avg_ser = std::accumulate(d_ser_history.begin(), 
                                            d_ser_history.end(), 
                                            0.0) / d_ser_history.size();
                } else if (d_current_frame_idx > 0) {
                    // 第一帧，使用当前帧的SER
                    avg_ser = static_cast<double>(d_current_frame_errors) / d_current_frame_idx;
                }
                
                out[nout] = static_cast<float>(avg_ser);
                
                // // 添加标签到输出
                // add_item_tag(0, nitems_written(0) + nout,
                //            pmt::string_to_symbol("phase_est"),
                //            pmt::from_long(d_total_frames),
                //            pmt::string_to_symbol(name()));
                
                nout++;
            }
            
            // 处理新帧
            handle_new_frame();
            tag_idx++;
        }
        
        // 比较数据
        if (d_current_frame_idx < d_frame_length) {
            if (in[i] != d_reference_data[d_current_frame_idx]) {
                d_current_frame_errors++;
            }
            d_current_frame_idx++;
        } else {
            // 超出帧长度，可能是同步问题
            // 继续处理但不计入统计
        }
        
        consumed = i + 1;
    }
    
    // 消费所有处理过的输入
    consume_each(consumed);
    
    // 返回实际产生的输出数量
    return nout;
}

} /* namespace freq_hopping */
} /* namespace gr */
