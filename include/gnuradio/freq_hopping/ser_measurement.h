/* -*- c++ -*- */
/*
 * Copyright 2026 lc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_FREQ_HOPPING_SER_MEASUREMENT_H
#define INCLUDED_FREQ_HOPPING_SER_MEASUREMENT_H

#include <gnuradio/block.h>
#include <gnuradio/freq_hopping/api.h>

namespace gr {
namespace freq_hopping {

/*!
 * \brief <+description of block+>
 * \ingroup freq_hopping
 *
 */
class FREQ_HOPPING_API ser_measurement : virtual public gr::block
{
public:
    typedef std::shared_ptr<ser_measurement> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of freq_hopping::ser_measurement.
     *
     * To avoid accidental use of raw pointers, freq_hopping::ser_measurement's
     * constructor is in a private implementation
     * class. freq_hopping::ser_measurement::make is the public interface for
     * creating new instances.
     */
    static sptr make(const std::string& filename);
};

} // namespace freq_hopping
} // namespace gr

#endif /* INCLUDED_FREQ_HOPPING_SER_MEASUREMENT_H */
