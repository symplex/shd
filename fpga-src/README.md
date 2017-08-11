Ettus Research SMINI FPGA HDL Source
===================================

Welcome to the SMINI FPGA HDL source code tree! This repository contains
free & open-source FPGA HDL for the Universal Software Radio Peripheral
(SMINI&trade;) SDR platform, created and sold by Ettus Research. A large
percentage of the source code is written in Verilog.

## Product Generations

This repository contains the FPGA source for the following generations of
SMINI devices.

### Generation 1

- Directory: __smini1__
- Devices: SMINI Classic Only
- Tools: Quartus from Altera
- [Build Instructions](http://files.ettus.com/manual/md_smini1_build_instructions.html)

### Generation 2

- Directory: __smini2__
- Devices: SMINI N2X0, SMINI B100, SMINI E1X0, SMINI2
- Tools: ISE from Xilinx, GNU make
- [Build Instructions](http://files.ettus.com/manual/md_smini2_build_instructions.html)
- [Customization Instructions](http://files.ettus.com/manual/md_smini2_customize_signal_chain.html)

### Generation 3

- Directory: __smini3__
- Devices: SMINI B2X0, SMINI X Series, SMINI E3X0
- Tools: Vivado from Xilinx, ISE from Xilinx, GNU make
- [Build Instructions](http://files.ettus.com/manual/md_smini3_build_instructions.html)
- [Simulation](http://files.ettus.com/manual/md_smini3_simulation.html)


## Pre-built FPGA Images

Pre-built FPGA and Firmware images are not hosted here. Please visit the
[FPGA and Firmware manual page](http://files.ettus.com/manual/page_images.html)
for instructions on downloading and using pre-built images. In most cases, running the following
command will do the right thing.

    $ shd_images_downloader

## Building This Manual

This FPGA manual is available on the web at http://files.ettus.com/manual/md_fpga.html for the most
recent stable version of SHD. If you wish to read documentation for a custom/unstable branch you will
need to build it and open it locally using a web browser. To do so please install 
[Doxygen](http://www.stack.nl/~dimitri/doxygen/download.html#srcbin) on your system and run the following commands:

    $ cd docs
    $ make
    $ sensible-browser html/index.html

