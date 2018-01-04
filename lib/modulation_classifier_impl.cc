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
#include "modulation_classifier_impl.h"
#include <numeric> // for accumulate
#include <volk/volk.h>

namespace gr {
  namespace cbmc {

    modulation_classifier::sptr
    modulation_classifier::make(int decimation, bool probe)
    {
      return gnuradio::get_initial_sptr
        (new modulation_classifier_impl(decimation, probe));
    }

    /*
     * The private constructor
     */
    modulation_classifier_impl::modulation_classifier_impl(int decimation, bool probe)
      : gr::sync_block("modulation_classifier",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
        d_decimation(decimation), d_probe_enabled(probe)
    {
    set_output_multiple(d_decimation);
    }

    /*
     * Our virtual destructor.
     */
    modulation_classifier_impl::~modulation_classifier_impl()
    {
    }

    int
    modulation_classifier_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      
      size_t i = 0;
      while ( i < noutput_items )
      {
        // set pointer 'samples'
        const gr_complex* samples = in + i;

        // Decide which modulation has been received and phase shift
        gr_complex samples_shifted[d_decimation];
        unsigned int det_mod_index = detMod2(samples_shifted, samples);
        if (d_probe_enabled) {
          d_stored_mod.push_back(det_mod_index);
        }

        // Assignment of Modulations to Indexes
        std::string det_mod = "";
        switch(det_mod_index)
        {
          case 0:
            det_mod = "8PSK";
            break;
          case 1:
            det_mod = "16AM";
            break;
          case 2:
            det_mod = "QPSK";
            break;
          case 3:
            det_mod = "BPSK";
        }


        // set output
        std::memcpy(out + i, samples_shifted, d_decimation * sizeof(gr_complex));

        // Set streamtag to the first item of the sample
        add_item_tag(0, // Port number
                nitems_written(0) + i, // Offset
                pmt::mp("det_mod"), // Key
                pmt::mp(det_mod) // Value
        );

        /*
        // Verification of modulation detection
        // Compares the detected Modulation with the one specified in a stream tag "sent_mod"
        
        // Get Tags: sent modulation
        std::vector<tag_t> sent_mod_tags;
        get_tags_in_window( // Note the different method name
            sent_mod_tags, // Tags will be saved here
            0, // Port 0
            0, // Start of range (relative to nitems_read(0))
            d_decimation, // End of relative range
            pmt::mp("sent_mod")
        );
        
        if ( !(sent_mod_tags.empty()) )
        {
          std::string verif = "";
          
          if ( pmt::symbol_to_string(sent_mod_tags[0].value) == det_mod )
          {
                  verif = "Passed";
          }
          else
          {
                  verif = "Failed";
          }

          // Check, if modulation changes within samples (assume, that it does not switch twice)
          for (int j = 1; j < sent_mod_tags.size(); j++)
          {
                  if ( !pmt::equal(sent_mod_tags[0].value, sent_mod_tags[j].value) )
                  {
                          verif = "Modulation Changed";
                          j = sent_mod_tags.size();
                  }
          }
          
          add_item_tag(0, // Port number
                  nitems_written(0) + i, // Offset
                  pmt::mp("Verified"), // Key
                  pmt::mp( verif ) // Value
          );
        }
        */
        
        i += d_decimation;
      }
      
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }
  
    // Computes normalized cumulants
    // real part would be sufficient
    // consumes d_decimation samples and c_2_1
    void
    modulation_classifier_impl::computeCumulant_4_0_u_4_2(gr_complex &c_4_0, gr_complex &c_4_2, const gr_complex* samples, gr_complex c_2_1)
    {
      // Samples squared
      gr_complex samples_pot[d_decimation];
      volk_32fc_s32f_power_32fc(samples_pot, samples, 2, d_decimation);
      
      gr_complex c_2_0 = std::accumulate(samples_pot, samples_pot + d_decimation, gr_complex{0}) * a_factor;
      
      // Cumulant 4_2
      //
      // Complex conjugate of samples
      gr_complex samples_con[d_decimation];
      volk_32fc_conjugate_32fc(samples_con, samples, d_decimation);
      
      // Conjugate samples sqared
      volk_32fc_s32f_power_32fc(samples_con, samples_con, 2, d_decimation);
      
      // Mean of conjugate samples squared
      gr_complex mean_c_sq = std::accumulate(samples_con, samples_con + d_decimation, gr_complex{0}) * a_factor;
      
      // Conjugate samples squared multiplied by samples squared: stored -> samples_con
      volk_32fc_x2_multiply_32fc(samples_con, samples_con, samples_pot, d_decimation);
      
      // Mean of (samples squared multiplied by conjugate samples squared)
      gr_complex mean_c_sq_sq = std::accumulate(samples_con, samples_con + d_decimation, gr_complex{0}) * a_factor;
      
      c_4_2 = (mean_c_sq_sq - c_2_0 * mean_c_sq - (gr_complex) 2 * pow(c_2_1,2));
      
      // Cumulant 4_0
      //
      // Samples to the power of 4
      volk_32fc_s32f_power_32fc(samples_pot, samples_pot, 2, d_decimation);
      
      // Mean of samples to the power of 4
      gr_complex mean_tdpo4 = std::accumulate(samples_pot, samples_pot + d_decimation, gr_complex{0}) * a_factor;
      
      c_4_0 = (mean_tdpo4 - (gr_complex) 3 * pow(c_2_0,2))/pow(c_2_1,2);//normalized with c_2_1
      
      
    }

    gr_complex
    modulation_classifier_impl::computeCumulant_2_1(const gr_complex* samples)
    {
      gr_complex samples_norm[d_decimation];
      volk_32fc_x2_multiply_conjugate_32fc(samples_norm, samples, samples, d_decimation);

      return std::accumulate(samples_norm, samples_norm + d_decimation, gr_complex{0}) * a_factor;
    }
    
    // Returns estimated phase, input: r = {2,4,8}
    float
    modulation_classifier_impl::phaseEstim(unsigned int r, float my, const gr_complex* samples)
    {
      gr_complex samples_c[d_decimation];
      // square first time
      volk_32fc_s32f_power_32fc(samples_c, samples, 2, d_decimation);
      
      switch(r){
        case 8:
          // square second time
          volk_32fc_s32f_power_32fc(samples_c, samples_c, 2, d_decimation);
        case 4:
          // square third time
          volk_32fc_s32f_power_32fc(samples_c, samples_c, 2, d_decimation);
        case 2:
          break;
        default:
          return(0);
      }

      gr_complex sum_to_the_r = std::accumulate(samples_c, samples_c + d_decimation, gr_complex{0});
      return 1 / (float) r * std::arg(my * sum_to_the_r);;
    }

    void
    modulation_classifier_impl::phaseShift(gr_complex* samples_shifted, const gr_complex* samples, float phi)
    {
      lv_32fc_t scalar = lv_cmake((float)std::cos(phi), (float)std::sin(phi));
      volk_32fc_s32fc_multiply_32fc(samples_shifted, samples, scalar, d_decimation);
    }

    // use real part of the cumulant, does not work properly with phase shift
    unsigned int
    modulation_classifier_impl::detMod1(gr_complex* samples_shifted, const gr_complex* samples)
    {
      // set boundries
      const float b1 = -1.36;
      const float b2 = -0.34;
      const float b3 = 0.5;
      
      float phi = 0;
      gr_complex c_4_0 = 0;
      gr_complex c_4_2 = 0;
      gr_complex c_2_1 = computeCumulant_2_1(samples);
      
      // Asume BPSK
      phi = phaseEstim(2, 1, samples);
      
      phaseShift(samples_shifted, samples, -phi);
      
      computeCumulant_4_0_u_4_2(c_4_0, c_4_2, samples_shifted, c_2_1); // Compute Cumulants
      if ( c_4_0.real() < b1)
      {
        return 3; //"BPSK"
      }
      
      // Asume 16QAM
      phi = phaseEstim(4, -0.68, samples);
      
      phaseShift(samples_shifted, samples, -phi);
      
      computeCumulant_4_0_u_4_2(c_4_0, c_4_2, samples_shifted, c_2_1); // Compute Cumulants
      if ( c_4_0.real() > b1 && c_4_0.real() < b2)
      {
        return 1; //"16QAM"
      }
      
      // Asume QPSK
      phi = phaseEstim(4, 1, samples);
      
      phaseShift(samples_shifted, samples, -phi);
      
      computeCumulant_4_0_u_4_2(c_4_0, c_4_2, samples_shifted, c_2_1); // Compute Cumulants
      if ( c_4_0.real() > b3) 
      {
        return 2; //"QPSK"
      }
      
      // Asume 8PSK
      phi = phaseEstim(8, 1, samples);
      
      phaseShift(samples_shifted, samples, -phi);
      
      computeCumulant_4_0_u_4_2(c_4_0, c_4_2, samples_shifted, c_2_1); // Compute Cumulants
      return 0; //"8PSK"
      
    }

    // RECOMENDED:
    // use the absolute value of the cumulant
    // No phase shift in the first place
    unsigned int
    modulation_classifier_impl::detMod2(gr_complex* samples_shifted, const gr_complex* samples)
    {
      // set boundries
      const float b1 = 0.34;
      const float b2 = 0.84;
      const float b3 = 1.5;
      
      float phi = 0;
      gr_complex c_4_0 = 0;
      gr_complex c_4_2 = 0;
      gr_complex c_2_1 = computeCumulant_2_1(samples);

      computeCumulant_4_0_u_4_2(c_4_0, c_4_2, samples, c_2_1); // Compute Cumulants
      if (d_probe_enabled==true) {
        d_stored_cumu.push_back(abs(c_4_0));
      }

      // Asume 8PSK
      if ( abs(c_4_0) < b1)
      {
        phi = phaseEstim(8, 1, samples);
        phaseShift(samples_shifted, samples, -phi);
        return 0; //"8PSK"
      }
      
      // Asume 16QAM
      if ( abs(c_4_0) >= b1 && abs(c_4_0) < b2)
      {
        phi = phaseEstim(4, -0.68, samples);
        phaseShift(samples_shifted, samples, -phi);
        return 1; //"16QAM"
      }
      
      // Asume QPSK
      if ( abs(c_4_0) >= b2 && abs(c_4_0) < b3) 
      {
        phi = phaseEstim(4, 1, samples);
        phaseShift(samples_shifted, samples, -phi);
        return 2; //"QPSK"
      }
      
      // Asume BPSK
      phi = phaseEstim(2, 1, samples);
      phaseShift(samples_shifted, samples, -phi);
      return 3; //"BPSK"
    }

  } /* namespace cbmc */
} /* namespace gr */

