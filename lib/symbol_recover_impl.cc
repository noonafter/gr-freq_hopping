#include "symbol_recover_impl.h"
#include <gnuradio/io_signature.h>
#include <complex>

namespace gr {
namespace freq_hopping {
symbol_recover::sptr symbol_recover::make(int sps)
{
    return gnuradio::make_block_sptr<symbol_recover_impl>(sps);
}

symbol_recover_impl::symbol_recover_impl(int sps)
    : block("symbol_recover",
            io_signature::make(1, 1, sizeof(gr_complex)), // 输入: 带有tag的complex
            io_signature::make(1, 1, sizeof(gr_complex))), // 输出: 抽样后的complex
      d_sps(sps),
      d_counter(0),
      d_phase_corr(0.0),
      d_is_synced(false),
      d_tag_key(pmt::mp("phase_est")),
      last_tag_offset(0),  // 修正拼写
      last_tag_value(0.0f)
{
    set_tag_propagation_policy(TPP_DONT);
}

symbol_recover_impl::~symbol_recover_impl() {}

void symbol_recover_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)
{
    // 粗略估计：输出 1 个点约需要 sps 个输入点
    ninput_items_required[0] = noutput_items * d_sps;
}

int symbol_recover_impl::general_work(int noutput_items,
                                      gr_vector_int &ninput_items,
                                      gr_vector_const_void_star &input_items,
                                      gr_vector_void_star &output_items)
{
    const gr_complex *in = (const gr_complex *)input_items[0];
    gr_complex *out = (gr_complex *)output_items[0];

    int ninput = ninput_items[0];
    int nproduced = 0;

    // 获取当前 buffer 范围内的所有标签
    std::vector<tag_t> tags, tags_corr;
    get_tags_in_range(tags, 0, nitems_read(0), nitems_read(0) + ninput, d_tag_key);
    get_tags_in_range(tags_corr, 0, nitems_read(0), nitems_read(0) + ninput, pmt::mp("corr_est"));

    // 为了方便处理，将标签按偏移量排序（通常已排序）
    std::sort(tags.begin(), tags.end(), [](const tag_t &a, const tag_t &b) {
        return a.offset < b.offset;
    });
    std::sort(tags_corr.begin(), tags_corr.end(), [](const tag_t &a, const tag_t &b) {
        return a.offset < b.offset;
    });

    auto tag_it = tags.begin();

    for (int i = 0; i < ninput; ++i) {
        uint64_t abs_offset = nitems_read(0) + i;
        bool has_tag = false;

        // 检查当前点是否有 phase_est 标签
        pmt::pmt_t pmt_val = pmt::get_PMT_NIL();
        if (tag_it != tags.end() && tag_it->offset == abs_offset) {
            // 提取相位值
            d_phase_corr = (float)pmt::to_double(tag_it->value);
            pmt_val = tag_it->value;

            // 查找同一位置的 corr_est 标签
            float d_corr_est = 0.0f;
            bool found_corr = false;
            for (const auto& corr_tag : tags_corr) {
                if (corr_tag.offset == abs_offset) {
                    d_corr_est = (float)pmt::to_double(corr_tag.value);
                    found_corr = true;
                    break;
                }
            }

            // 判断是否为假标签
            bool is_fake_tag = false;
            if (last_tag_offset != 0 && found_corr) {
                uint64_t distance = abs_offset - last_tag_offset;
                if (distance < (uint64_t)d_sps && d_corr_est < last_tag_value) {
                    // 说明这个phase_est标签是假的，是由于corr_est切断了信号而产生的
                    is_fake_tag = true;
                }
            }

            if (!is_fake_tag) {
                // 触发同步：重置计数器
                d_counter = 0;
                d_is_synced = true;
                has_tag = true;

                // 更新last_tag信息
                last_tag_offset = abs_offset;
                if (found_corr) {
                    last_tag_value = d_corr_est;
                }
            }

            tag_it++;
        }

        // 逻辑判断：如果是标签所在点，或者是同步后的 sps 整数倍点
        if (d_is_synced && (has_tag || d_counter == d_sps)) {
            if (nproduced < noutput_items) {
                // 执行相位纠正：out = in * exp(-j * phase)
                // gr_complex rot = std::exp(gr_complex(0.0, -d_phase_corr));
                // out[nproduced] = in[i] * rot;
                out[nproduced] = in[i]; // 在symbol中不纠相偏，放到costas环中进行

                // --- 手动转发标签 ---
                if (has_tag) {
                    // 将输入标签的 Key 和 Value 复制到当前的输出 Offset上
                    add_item_tag(0,                              // 输出端口 0
                                 nitems_written(0) + nproduced,  // 对应的输出绝对 Offset
                                 d_tag_key,                      // 标签 Key
                                 pmt_val                         // 标签 Value
                    );
                }
                // ------------------
                nproduced++;
                // 重置计数器（从当前抽样点开始重新数）
                d_counter = 0;
            } else {
                // 如果输出缓存满了，停止处理
                consume_each(i);
                return nproduced;
            }
        }

        d_counter++;
    }

    consume_each(ninput);
    return nproduced;
}

} // namespace freq_hopping
} // namespace gr
