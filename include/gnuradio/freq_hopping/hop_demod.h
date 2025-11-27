/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_HOP_DEMOD_H
#define INCLUDED_FREQ_HOPPING_HOP_DEMOD_H

#include <gnuradio/freq_hopping/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace freq_hopping {

/*!
 * \brief <+description of block+>
 * \ingroup freq_hopping
 *
 */
class FREQ_HOPPING_API hop_demod : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<hop_demod> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of freq_hopping::hop_demod.
     *
     * To avoid accidental use of raw pointers, freq_hopping::hop_demod's
     * constructor is in a private implementation
     * class. freq_hopping::hop_demod::make is the public interface for
     * creating new instances.
     */
    static sptr make(double bw_hop = 12000,
                     double ch_sep = 3000,
                     double freq_carrier = 0,
                     double fsa_hop = 12000,
                     double hop_rate = 5);
};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_HOP_DEMOD_H */
