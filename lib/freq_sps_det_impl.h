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

#ifndef INCLUDED_CBMC_FREQ_SPS_DET_IMPL_H
#define INCLUDED_CBMC_FREQ_SPS_DET_IMPL_H

#include <cbmc/freq_sps_det.h>
#include <gnuradio/fft/fft.h>
#include <gnuradio/fft/goertzel.h>

namespace gr {
  namespace cbmc {

    class freq_sps_det_impl : public freq_sps_det
    {
     private:
      typedef std::complex<double>     complexd;
      const double            pi = std::acos(-1);
      // assign minus j two pi
      const complexd          d_m_j_2pi = (gr_complex(0,-2) * gr_complex(pi));
      const int               d_decimation;
      short unsigned int      d_fft_size;
      fft::fft_complex       *d_fft;
      fft::goertzel          *d_goertzel;
      complexd                d_phase;
      short unsigned int      d_nsubdiv;
      std::vector<float>      d_stored_freqs;

     public:
      freq_sps_det_impl(int decimation, int fft_size);
      ~freq_sps_det_impl();

      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);

      std::vector<float> get_stored_freqs() const
      {
        return d_stored_freqs;
      }

      void discard_stored_freqs()
      {
        d_stored_freqs.clear();
      }

      void calc_f_offset_and_sps(float &f_offset, float &sps, const gr_complex* samples);
      inline void f_shift_samples(gr_complex* output, const gr_complex* samples, float f_offset);
      inline float calc_offset(const gr_complex* samples_x, short unsigned int MaxIndex, float factor);
      inline float calc_sps(float* samples_abs_fft, short unsigned int maxIndex);
      float ft_refinement(short unsigned int rough_freq, const gr_complex* samples, const short unsigned int refinem_f);
    };

  } // namespace cbmc
} // namespace gr

#endif /* INCLUDED_CBMC_FREQ_SPS_DET_IMPL_H */

