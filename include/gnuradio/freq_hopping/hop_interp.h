/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_HOP_INTERP_H
#define INCLUDED_FREQ_HOPPING_HOP_INTERP_H

#include <gnuradio/freq_hopping/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace freq_hopping {

/*!
 * \brief <+description of block+>
 * \ingroup freq_hopping
 *
 */
class FREQ_HOPPING_API hop_interp : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<hop_interp> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of freq_hopping::hop_interp.
     *
     * To avoid accidental use of raw pointers, freq_hopping::hop_interp's
     * constructor is in a private implementation
     * class. freq_hopping::hop_interp::make is the public interface for
     * creating new instances.
     */
    static sptr make(int interp_fac = 1, int vlen_in = 1);
};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_HOP_INTERP_H */
