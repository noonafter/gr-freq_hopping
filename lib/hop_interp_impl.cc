/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "hop_interp_impl.h"
#include <gnuradio/io_signature.h>
#include "bb_pskmod_impl.h"

namespace gr {
namespace freq_hopping {

using input_type = gr_complex;
using output_type = gr_complex;
hop_interp::sptr hop_interp::make(int interp_fac,int vlen_in)
{
    return gnuradio::make_block_sptr<hop_interp_impl>(interp_fac,vlen_in);
}


/*
 * The private constructor
 */
hop_interp_impl::hop_interp_impl(int interp_fac,int vlen_in)
    : gr::sync_block("hop_interp",
                     gr::io_signature::make(1, 1, sizeof(input_type)*vlen_in),
                     gr::io_signature::make(1, 1, sizeof(output_type)*vlen_in*interp_fac)),
d_interp_fac(interp_fac),
d_vlen_in(vlen_in),
resampler(nullptr)
{
    resampler = rresamp_crcf_create_default(d_interp_fac, 1);
}

/*
 * Our virtual destructor.
 */
hop_interp_impl::~hop_interp_impl()
{
    if (resampler) {
        rresamp_crcf_destroy(resampler);
    }
}

int hop_interp_impl::work(int noutput_items,
                          gr_vector_const_void_star& input_items,
                          gr_vector_void_star& output_items)
{
    auto in = static_cast<const input_type*>(input_items[0]);
    auto out = static_cast<output_type*>(output_items[0]);

    if (resampler == nullptr) {
        return 0;
    }

    // 这是rrsample的bug，由于使用了pfb会小，乘以sqrt(P)补回来
    gr_complex scale = gr_complex(std::sqrt(d_interp_fac),0);

    // Do <+signal processing+>
    int idx_item = 0;
    for (; idx_item < noutput_items; ++idx_item) {
        auto frame_in = const_cast<input_type*>(in + idx_item * d_vlen_in);
        auto frame_out = out + idx_item * d_vlen_in * d_interp_fac;

        for (int i=0; i<d_vlen_in; i++) {
            auto val_in = *(frame_in+i) * scale;
            rresamp_crcf_execute(resampler, &val_in, frame_out+i*d_interp_fac);
        }
        rresamp_crcf_reset(resampler);
    }



    // Tell runtime system how many output items we produced.
    return idx_item;
}

} /* namespace freq_hopping */
} /* namespace gr */
