#include "frame_recover_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
  namespace freq_hopping {

    frame_recover::sptr
    frame_recover::make(int frame_len)
    {
      return gnuradio::make_block_sptr<frame_recover_impl>(frame_len);
    }

    frame_recover_impl::frame_recover_impl(int frame_len)
      : gr::block("frame_recover",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(0, 1, sizeof(gr_complex))), // 输出可以为0
        d_frame_len(frame_len),
        d_count(0),
        d_is_active(false)
    {
        d_tag_key = pmt::mp("phase_est");
        set_tag_propagation_policy(TPP_DONT);
    }

    frame_recover_impl::~frame_recover_impl() {}

    void frame_recover_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)
    {
        // 即使没有输出，也需要输入来检测标签，所以至少需要 noutput 个输入
        ninput_items_required[0] = noutput_items;
    }

    int frame_recover_impl::general_work(int noutput_items,
                                         gr_vector_int &ninput_items,
                                         gr_vector_const_void_star &input_items,
                                         gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *)input_items[0];
        gr_complex *out = (gr_complex *)output_items[0];

        int ninput = ninput_items[0];
        int nproduced = 0;

        // 获取当前范围内的标签
        std::vector<tag_t> tags;
        get_tags_in_range(tags, 0, nitems_read(0), nitems_read(0) + ninput, d_tag_key);
        auto tag_it = tags.begin();

        for (int i = 0; i < ninput; ++i) {
            uint64_t abs_offset_in = nitems_read(0) + i;

            // 1. 标签检测：如果看到新标签，重置计数器
            while (tag_it != tags.end() && tag_it->offset == abs_offset_in) {
                d_is_active = true;
                d_count = d_frame_len;
                
                // 将起始标签转发到输出端
                if (nproduced < noutput_items) {
                    add_item_tag(0, nitems_written(0) + nproduced, d_tag_key,
                                 tag_it->value, pmt::mp("frame_recover"));
                }
                tag_it++;
            }

            // 2. 数据处理
            if (d_is_active) {
                if (nproduced < noutput_items) {
                    out[nproduced++] = in[i];
                    d_count--;

                    if (d_count <= 0) {
                        d_is_active = false; // 帧长达到，停止输出
                    }
                } else {
                    // 输出缓存已满，停止处理
                    consume_each(i);
                    return nproduced;
                }
            }
            // 如果 d_is_active 为 false，则该输入样点被跳过（丢弃）
        }

        consume_each(ninput);
        return nproduced;
    }
  }
}