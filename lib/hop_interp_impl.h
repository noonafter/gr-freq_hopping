/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_HOP_INTERP_IMPL_H
#define INCLUDED_FREQ_HOPPING_HOP_INTERP_IMPL_H

#include <gnuradio/freq_hopping/hop_interp.h>
#include "liquid/liquid.h"

namespace gr {
namespace freq_hopping {

class hop_interp_impl : public hop_interp
{
private:
    int d_interp_fac;
    int d_vlen_in;
    rresamp_crcf resampler ;

public:
    hop_interp_impl(int interp_fac, int vlen_in = 1);
    ~hop_interp_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_HOP_INTERP_IMPL_H */
