FPGA Manual
===========

Welcome to the SMINI FPGA HDL source code tree! This repository contains
free & open-source FPGA HDL for the Universal Software Radio Peripheral
(SMINI&trade;) SDR platform, created and sold by Ettus Research. A large
percentage of the source code is written in Verilog.

## Product Generations

This repository contains the FPGA source for the following generations of
SMINI devices.

### Generation 1

\li Directory: __smini1__
\li Devices: SMINI Classic Only
\li Tools: Quartus from Altera
\li \subpage md_smini1_build_instructions "Build Instructions"


### Generation 2

\li Directory: __smini2__
\li Devices: SMINI N2X0, SMINI B100, SMINI E1X0, SMINI2
\li Tools: ISE from Xilinx, GNU make
\li \subpage md_smini2_build_instructions "Build Instructions"
\li \subpage md_smini2_customize_signal_chain "Customization Instructions"

### Generation 3

\li Directory: __smini3__
\li Devices: SMINI B2X0, SMINI X Series, SMINI E3X0
\li Tools: ISE from Xilinx, GNU make
\li \subpage md_smini3_build_instructions "Build Instructions"
\li \subpage md_smini3_simulation "Simulation"

## Pre-built FPGA Images

Pre-built FPGA and Firmware images are not hosted here. Please visit \ref page_images
for instructions on downloading and using pre-built images. In most cases, running

    $ shd_images_downloader

will do the right thing.

