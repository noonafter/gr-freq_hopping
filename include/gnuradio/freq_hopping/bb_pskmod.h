/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_BB_PSKMOD_H
#define INCLUDED_FREQ_HOPPING_BB_PSKMOD_H

#include <gnuradio/freq_hopping/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace freq_hopping {

/*!
 * \brief <+description of block+>
 * \ingroup freq_hopping
 *
 */
class FREQ_HOPPING_API bb_pskmod : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<bb_pskmod> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of freq_hopping::bb_pskmod.
     *
     * To avoid accidental use of raw pointers, freq_hopping::bb_pskmod's
     * constructor is in a private implementation
     * class. freq_hopping::bb_pskmod::make is the public interface for
     * creating new instances.
     */
    static sptr make(int hop_rate = 5, int M_order = 4, int Ksa_ch = 4);
};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_BB_PSKMOD_H */
