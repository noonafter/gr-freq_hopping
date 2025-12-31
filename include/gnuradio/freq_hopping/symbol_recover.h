/* -*- c++ -*- */
/*
 * Copyright 2025 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_SYMBOL_RECOVER_H
#define INCLUDED_FREQ_HOPPING_SYMBOL_RECOVER_H

#include <gnuradio/block.h>
#include <gnuradio/freq_hopping/api.h>

namespace gr {
namespace freq_hopping {

/*!
 * \brief <+description of block+>
 * \ingroup freq_hopping
 *
 */
class FREQ_HOPPING_API symbol_recover : virtual public gr::block
{
public:
    typedef std::shared_ptr<symbol_recover> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of freq_hopping::symbol_recover.
     *
     * To avoid accidental use of raw pointers, freq_hopping::symbol_recover's
     * constructor is in a private implementation
     * class. freq_hopping::symbol_recover::make is the public interface for
     * creating new instances.
     */
    static sptr make(int sps = 1);
};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_SYMBOL_RECOVER_H */
