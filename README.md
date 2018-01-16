# gr-cbmc
A Cumulant-Based Modulation Classification module for GNU Radio

## Features
* Classification of the modulation of a signal
  * Detection of BPSK, QPSK, 16-QAM and 8-PSK possible
  * Classification and phase correction is done by one block
* The necessary synchronisation is perfomed by custom-built blocks
  * Frequency and Symbolrate estimation
  * Time synchronization (modified version of pfb_clock_sync)
* The result is a stream of symbols (with 1 sample per symbol) without phase-/frequency-/timing-offset, which includes the current modulation as stream tags

## Usage
There is a flowgraph in examples/ which demonstrates the classification receiver chain. The receiver of the flowgraph is displayed here:

![Example flowgraph](https://github.com/kit-cel/gr-cbmc/raw/master/pic_example_flowgraph.png "Example flowgraph")

Further information is available in this [bachelor thesis (german).](https://github.com/kit-cel/gr-cbmc/raw/master/Bachelor_thesis_Douglas_Weber.pdf) This module was built as part of the thesis. An additional interesting source is the doctoral thesis by Michael Sebastian Mühlhaus: [Automatische Modulationsartenerkennung in MIMO-Systemen.](https://publikationen.bibliothek.kit.edu/1000039383)

## Installation
To install this module, run these commands:

    $ mkdir build
    $ cd build
    $ cmake ../
    $ make
    $ sudo make install
    $ sudo ldconfig

The build process has been tested with
  * cppunit, version 1.14.0
  * gnuradio, version 3.7.10
  * doxygen, version 1.8.13

## Current Constraints
* Just setting stream tags, no actual demodulation of the signal
* If there is only noise, always 8PSK will be classified

