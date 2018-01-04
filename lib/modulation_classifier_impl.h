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

#ifndef INCLUDED_CBMC_MODULATION_CLASSIFIER_IMPL_H
#define INCLUDED_CBMC_MODULATION_CLASSIFIER_IMPL_H

#include <cbmc/modulation_classifier.h>

namespace gr {
  namespace cbmc {

    class modulation_classifier_impl : public modulation_classifier
    {
     private:
      const int                   d_decimation;
      const float                 a_factor = 1.0/d_decimation;
      const double                pi = std::acos(-1);
      const bool                  d_probe_enabled;  // If enabled store last determined Modulations
      std::vector<unsigned int>   d_stored_mod;     // Used to store last determined Modulations
      std::vector<float>          d_stored_cumu;     // Used to store last calculated cumulants

     public:
      modulation_classifier_impl(int decimation, bool probe);
      ~modulation_classifier_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);

      // Get functions
      std::vector<unsigned int> get_stored_mod() const
      {
        return d_stored_mod;
      }

      std::vector<float> get_stored_cumu() const
      {
        return d_stored_cumu;
      }

      // Reset
      void reset()
      {
        d_stored_mod.clear();
        d_stored_cumu.clear();
      }

      //
      float phaseEstim(unsigned int r, float my, const gr_complex* samples);
      void phaseShift(gr_complex* samples_shifted, const gr_complex* samples, float phi);
      void computeCumulant_4_0_u_4_2(gr_complex &c_4_0, gr_complex &c_4_2, const gr_complex* samples, gr_complex c_2_1);
      gr_complex computeCumulant_2_1(const gr_complex* samples);
      unsigned int detMod1(gr_complex* samples_shifted, const gr_complex* samples);
      unsigned int detMod2(gr_complex* samples_shifted, const gr_complex* samples);
    };

  } // namespace cbmc
} // namespace gr

#endif /* INCLUDED_CBMC_MODULATION_CLASSIFIER_IMPL_H */

