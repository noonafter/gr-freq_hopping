/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_SLOT_FRAME_H
#define INCLUDED_FREQ_HOPPING_SLOT_FRAME_H

#include <gnuradio/freq_hopping/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace freq_hopping {

/*!
 * \brief <+description of block+>
 * \ingroup freq_hopping
 *
 */
class FREQ_HOPPING_API slot_frame : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<slot_frame> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of freq_hopping::slot_frame.
     *
     * To avoid accidental use of raw pointers, freq_hopping::slot_frame's
     * constructor is in a private implementation
     * class. freq_hopping::slot_frame::make is the public interface for
     * creating new instances.
     */
    static sptr make(int hop_rate = 20, int M_order = 4, int info_seed = 0);
};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_SLOT_FRAME_H */
