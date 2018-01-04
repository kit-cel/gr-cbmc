/* -*- c++ -*- */
/* 
 * Copyright 2016 <+YOU OR YOUR COMPANY+>.
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


#ifndef INCLUDED_CBMC_MY_PFB_CLOCK_SYNC_IMPL_H
#define INCLUDED_CBMC_MY_PFB_CLOCK_SYNC_IMPL_H

#include <cbmc/my_pfb_clock_sync.h>

using namespace gr::filter;

namespace gr {
  namespace cbmc {

    class my_pfb_clock_sync_impl : public my_pfb_clock_sync
    {
    private:
      unsigned int d_det_block_size;
      bool   d_updated;
      double d_sps;
      double d_last_sps;
      double d_sample_num;
      float  d_loop_bw;
      float  d_damping;
      float  d_alpha;
      float  d_beta;

      int                                  d_nfilters;
      int                                  d_taps_per_filter;
      std::vector<kernel::fir_filter_ccf*> d_filters;
      std::vector<kernel::fir_filter_ccf*> d_diff_filters;
      std::vector<float>                   d_init_taps;
      std::vector< std::vector<float> >    d_taps;
      std::vector< std::vector<float> >    d_dtaps;
      std::vector<float>                   d_updated_taps;

      float d_init_phase;
      float d_k;
      float d_rate;
      float d_rate_i;
      float d_rate_f;
      float d_max_dev;
      int   d_filtnum;
      int   d_osps;
      float d_error;
      int   d_out_idx;

      void create_diff_taps(const std::vector<float> &newtaps,
			    std::vector<float> &difftaps);

    public:
      my_pfb_clock_sync_impl(double sps, float loop_bw,
			      unsigned int filter_size=32,
			      float init_phase=0,
			      float max_rate_deviation=1.5,
			      int osps=1,
			      unsigned int det_block_size=10000);
      ~my_pfb_clock_sync_impl();

      void setup_rpc();

      void update_gains();

      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      void update_taps(const std::vector<float> &taps);

      void set_taps(const std::vector<float> &taps,
		    std::vector< std::vector<float> > &ourtaps,
		    std::vector<kernel::fir_filter_ccf*> &ourfilter);

      void set_sps(double new_sps);

      std::vector< std::vector<float> > taps() const;
      std::vector< std::vector<float> > diff_taps() const;
      std::vector<float> channel_taps(int channel) const;
      std::vector<float> diff_channel_taps(int channel) const;
      std::string taps_as_string() const;
      std::string diff_taps_as_string() const;

      void set_loop_bandwidth(float bw);
      void set_damping_factor(float df);
      void set_alpha(float alpha);
      void set_beta(float beta);
      void set_max_rate_deviation(float m)
      {
	d_max_dev = m;
      }

      float loop_bandwidth() const;
      float damping_factor() const;
      float alpha() const;
      float beta() const;
      float clock_rate() const;

      float error() const;
      float rate() const;
      float phase() const;


      /*******************************************************************
       *******************************************************************/

      bool check_topology(int ninputs, int noutputs);

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);
    };

  } // namespace cbmc
} // namespace gr

#endif /* INCLUDED_CBMC_MY_PFB_CLOCK_SYNC_IMPL_H */

