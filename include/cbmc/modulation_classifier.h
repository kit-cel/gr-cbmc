/* -*- c++ -*- */
/* 
 * Copyright 2016 Douglas Weber.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */


#ifndef INCLUDED_CBMC_MODULATION_CLASSIFIER_H
#define INCLUDED_CBMC_MODULATION_CLASSIFIER_H

#include <cbmc/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace cbmc {

    /*!
     * \brief <+description of block+>
     * \ingroup cbmc
     *
     */
    class CBMC_API modulation_classifier : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<modulation_classifier> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of cbmc::modulation_classifier.
       *
       * To avoid accidental use of raw pointers, cbmc::modulation_classifier's
       * constructor is in a private implementation
       * class. cbmc::modulation_classifier::make is the public interface for
       * creating new instances.
       */
      static sptr make(int decimation, bool probe);

      virtual std::vector<unsigned int> get_stored_mod() const = 0;
      virtual std::vector<float> get_stored_cumu() const = 0;
      virtual void reset() = 0;
    };

  } // namespace cbmc
} // namespace gr

#endif /* INCLUDED_CBMC_MODULATION_CLASSIFIER_H */

