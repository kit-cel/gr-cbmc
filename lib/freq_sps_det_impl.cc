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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "freq_sps_det_impl.h"
#include <volk/volk.h>
#include <complex>

namespace gr {
  namespace cbmc {

    freq_sps_det::sptr
    freq_sps_det::make(int decimation, int fft_size)
    {
      return gnuradio::get_initial_sptr
        (new freq_sps_det_impl(decimation, fft_size));
    }

    freq_sps_det_impl::freq_sps_det_impl(int decimation, int fft_size)
      : gr::sync_block("freq_sps_det",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
    d_decimation(decimation), d_nsubdiv(fft_size)
    {
    d_fft_size = d_decimation;
    d_fft = new fft::fft_complex(d_fft_size, true, 1);
    d_goertzel = new fft::goertzel(1, d_fft_size, 0);
    set_output_multiple(d_decimation);
    }

    /*
     * Our virtual destructor.
     */
    freq_sps_det_impl::~freq_sps_det_impl()
    {
      delete d_fft;
    }

    int
    freq_sps_det_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      size_t i = 0;
      while ( i < noutput_items )
      {
        // Set pointer 'samples'
        const gr_complex* samples = in + i;

        // Frequency estimation
        float f_offset;
        float sps;
        calc_f_offset_and_sps(f_offset, sps, samples);
        d_stored_freqs.push_back(f_offset);

        // Set streamtag with detected sps
        add_item_tag(0, nitems_written(0) + i, pmt::intern("det_sps"), pmt::from_float(sps));

        // Apply frequency correction and write samples the into output 
        f_shift_samples(out + i, samples, f_offset);

        // Increase counter of written samples
        i += d_decimation;
      }

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

    // Returns the offset number of points from d_fft_size
    // consumes first d_decimation items of samples
    void
    freq_sps_det_impl::calc_f_offset_and_sps(float &f_offset, float &sps, const gr_complex* samples)
    { 
      // Samples to the power of 2
      gr_complex samples_2[d_fft_size];
      volk_32fc_s32f_power_32fc(samples_2, samples, 2, d_decimation);
      
      // Samples to the power of 4
      gr_complex samples_4[d_fft_size];
      volk_32fc_s32f_power_32fc(samples_4, samples_2, 2, d_decimation);
      
      // Samples to the power of 8
      gr_complex samples_8[d_fft_size];
      volk_32fc_s32f_power_32fc(samples_8, samples_4, 2, d_decimation);
      
      
      // Calculate FFTs
      gr_complex samples_2_fft[d_fft_size];
      memcpy(d_fft->get_inbuf(), samples_2, d_fft_size*sizeof(gr_complex));
      d_fft->execute();
      memcpy(samples_2_fft, d_fft->get_outbuf(), d_fft_size*sizeof(gr_complex));
      
      gr_complex samples_4_fft[d_fft_size];
      memcpy(d_fft->get_inbuf(), samples_4, d_fft_size*sizeof(gr_complex));
      d_fft->execute();
      memcpy(samples_4_fft, d_fft->get_outbuf(), d_fft_size*sizeof(gr_complex));
      
      gr_complex samples_8_fft[d_fft_size];
      memcpy(d_fft->get_inbuf(), samples_8, d_fft_size*sizeof(gr_complex));
      d_fft->execute();
      memcpy(samples_8_fft, d_fft->get_outbuf(), d_fft_size*sizeof(gr_complex));
      
      // Magnitude of FFTs
      float samples_2_abs_fft[d_fft_size];
      volk_32fc_magnitude_32f(samples_2_abs_fft, samples_2_fft, d_fft_size);
      
      float samples_4_abs_fft[d_fft_size];
      volk_32fc_magnitude_32f(samples_4_abs_fft, samples_4_fft, d_fft_size);
      
      float samples_8_abs_fft[d_fft_size];
      volk_32fc_magnitude_32f(samples_8_abs_fft, samples_8_fft, d_fft_size);
      
      
      // Calculate Maxima of FFTs
      short unsigned int maxIndex_2;
      volk_32f_index_max_16u(&maxIndex_2, samples_2_abs_fft, d_fft_size);
      
      short unsigned int maxIndex_4;
      volk_32f_index_max_16u(&maxIndex_4, samples_4_abs_fft, d_fft_size);
      
      short unsigned int maxIndex_8;
      volk_32f_index_max_16u(&maxIndex_8, samples_8_abs_fft, d_fft_size);
      
      // Calculate quality criterion
      float samples_2_qc;
      volk_32f_accumulator_s32f(&samples_2_qc, samples_2_abs_fft, d_fft_size);
      samples_2_qc = samples_2_abs_fft[maxIndex_2] / samples_2_qc;
      
      float samples_4_qc;
      volk_32f_accumulator_s32f(&samples_4_qc, samples_4_abs_fft, d_fft_size);
      samples_4_qc = samples_4_abs_fft[maxIndex_4] / samples_4_qc;
      
      float samples_8_qc;
      volk_32f_accumulator_s32f(&samples_8_qc, samples_8_abs_fft, d_fft_size);
      samples_8_qc = samples_8_abs_fft[maxIndex_8] / samples_8_qc;
      
      
      if (samples_2_qc > samples_4_qc && samples_2_qc > samples_8_qc)
      {
        f_offset = calc_offset(samples_2, maxIndex_2, 0.5);
        sps = calc_sps(samples_2_abs_fft, maxIndex_2);
      }
      else if (samples_4_qc > samples_2_qc && samples_4_qc > samples_8_qc)
      {
        f_offset = calc_offset(samples_4, maxIndex_4, 0.25);
        sps = calc_sps(samples_4_abs_fft, maxIndex_4);
      }
      else
      {
        f_offset = calc_offset(samples_8, maxIndex_8, 0.125);
        sps = calc_sps(samples_8_abs_fft, maxIndex_8);
      }

    }

  inline void
  freq_sps_det_impl::f_shift_samples(gr_complex* output, const gr_complex* samples, float f_offset)
  {
    complexd exp_factor = d_m_j_2pi * complexd(f_offset);
    for ( int k = 0; k < d_decimation; k++)
    {
      d_phase += exp_factor;
      *(output + k) = samples[k] * gr_complex(std::exp( d_phase ));
    }
    // Ignore turns by multiple of 2 pi
    d_phase = gr_complex(0, fmod (d_phase.imag(), float(2)*pi));
  }

  inline float
  freq_sps_det_impl::calc_offset(const gr_complex* samples_x, short unsigned int maxIndex, float factor)
  {
    float f_offset;

    // Check if offset is negative
    if (maxIndex > d_fft_size/2)
    {
      f_offset = (float) maxIndex - (float) d_fft_size;
    }
    else
    {
      f_offset = (float) maxIndex;
    }

    float fine_offset = 0;
    if (d_nsubdiv > 1) {
      fine_offset = ft_refinement(maxIndex, samples_x, d_nsubdiv);
    }

    f_offset = ((float)f_offset + (float)fine_offset) * factor;

    return f_offset/float(d_fft_size);
  }

  // Calculation of samples per symbol
  // 'destroys' samples_abs_fft
  inline float
  freq_sps_det_impl::calc_sps(float* samples_abs_fft, short unsigned int maxIndex)
  {
    float sps;

    // Find second highest peak
    samples_abs_fft[maxIndex] = 0;

    // Do not find peaks near f_offset
      int frame =  10 * ceil(d_fft_size / d_decimation);

      for (int i = -frame; i < frame; i++)
      {
        if ((int) maxIndex + i < 0)
        {
          samples_abs_fft[(int) maxIndex + i + d_fft_size] = 0;
        }
        else
        {
          samples_abs_fft[(int) maxIndex + i] = 0;
        }
      }

    short unsigned int sps_index;
    volk_32f_index_max_16u(&sps_index, samples_abs_fft, d_fft_size);

    // Check if sps is negative
    if (sps_index > d_fft_size/2) { sps = ((float) sps_index - (float) d_fft_size); }
    else { sps = (float) sps_index; }

    // only works, if d_fft_size == d_decimation
    int f_offset_index;
    if (maxIndex > d_fft_size/2)
    { f_offset_index = (int) maxIndex - (int) d_fft_size; }
    else { f_offset_index = (int) maxIndex; }

    return std::abs(d_fft_size / (f_offset_index - sps));
  }

  float
  freq_sps_det_impl::ft_refinement(short unsigned int rough_index, const gr_complex* samples, const short unsigned int refinem_f)
  {
    float samples_real[d_decimation];
    volk_32fc_deinterleave_real_32f(samples_real, samples, d_decimation);
    float samples_imag[d_decimation];
    volk_32fc_deinterleave_imag_32f(samples_imag, samples, d_decimation);

    short unsigned int n_points = refinem_f;
    if ( refinem_f % 2 == 0 ) { n_points += 1; }
    gr_complex ans_goertzel[n_points];
    int d;
    gr_complex j(0,1);
    for (int i = 0; i < n_points; i++){
      d = (int)refinem_f * rough_index - (int)((float)(n_points-1) * 0.5)  + i;
      if (d < 0) { d += (int)refinem_f * d_decimation; }
      else if (d > refinem_f * d_decimation) { d -= (int)refinem_f * d_decimation; }
      d_goertzel->set_params(d_decimation*refinem_f, d_decimation, d);
      gr_complex bin_r = d_goertzel->batch(samples_real);
      gr_complex bin_i = d_goertzel->batch(samples_imag);
      ans_goertzel[i] = bin_r + j * bin_i;
    }
    short unsigned int maxIndex;
    float ans_goertzel_abs[n_points];
    volk_32fc_magnitude_32f(ans_goertzel_abs, ans_goertzel, n_points);
    volk_32f_index_max_16u(&maxIndex, ans_goertzel_abs, n_points);

    return ((float) maxIndex - (float)(n_points-1) * 0.5)/(float)refinem_f;
  }

  } /* namespace cbmc */
} /* namespace gr */

