/* -*- c++ -*- */

#define CBMC_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "cbmc_swig_doc.i"

%{
#include "cbmc/modulation_classifier.h"
#include "cbmc/freq_sps_det.h"
#include "cbmc/my_pfb_clock_sync.h"
%}


%include "cbmc/modulation_classifier.h"
GR_SWIG_BLOCK_MAGIC2(cbmc, modulation_classifier);
%include "cbmc/freq_sps_det.h"
GR_SWIG_BLOCK_MAGIC2(cbmc, freq_sps_det);
%include "cbmc/my_pfb_clock_sync.h"
GR_SWIG_BLOCK_MAGIC2(cbmc, my_pfb_clock_sync);
