/* -*- c++ -*- */
/*
 * Copyright 2026 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_FRAME_RECOVER_H
#define INCLUDED_FREQ_HOPPING_FRAME_RECOVER_H

#include <gnuradio/block.h>
#include <gnuradio/freq_hopping/api.h>

namespace gr {
namespace freq_hopping {

/*!
 * \brief <+description of block+>
 * \ingroup freq_hopping
 *
 */
class FREQ_HOPPING_API frame_recover : virtual public gr::block
{
public:
    typedef std::shared_ptr<frame_recover> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of freq_hopping::frame_recover.
     *
     * To avoid accidental use of raw pointers, freq_hopping::frame_recover's
     * constructor is in a private implementation
     * class. freq_hopping::frame_recover::make is the public interface for
     * creating new instances.
     */
    static sptr make(int frame_len = 1);
};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_FRAME_RECOVER_H */
