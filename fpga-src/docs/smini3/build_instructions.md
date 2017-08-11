# Generation 3 SMINI Build Documentation

## Dependencies and Requirements

### Dependencies

The SMINI FPGA build system requires a UNIX-like environment with the following dependencies

- [Xilinx Vivado 2015.4](http://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/2015-4.html) (For 7 Series FPGAs)
- [Xilinx ISE 14.7](http://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/design-tools/v2012_4---14_7.html) (For all other FPGAs)
- [GNU Make 3.6+](https://www.gnu.org/software/make/)
- [GNU Bash 4.0+](https://www.gnu.org/software/bash/)
- [Python 2.7.x](https://www.python.org/)
- [Doxygen](http://www.stack.nl/~dimitri/doxygen/index.html) (Optional: To build the manual)
- [ModelSim](https://www.mentor.com/products/fv/modelsim/) (Optional: For simulation)

### What FPGA does my SMINI have?

- SMINI B200: Spartan 6 XC6SLX75
- SMINI B200mini: Spartan 6 XC6SLX75
- SMINI B210: Spartan 6 XC6SLX150
- SMINI X300: Kintex 7 XC7K325T (7 Series)
- SMINI X310: Kintex 7 XC7K410T (7 Series)
- SMINI E310: Zynq-7000 XC7Z020 (7 Series)

### Requirements

- [Xilinx Vivado Release Notes](http://www.xilinx.com/support/documentation/sw_manuals/xilinx2015_4/ug973-vivado-release-notes-install-license.pdf)
- [Xilinx ISE Platform Requirements](http://www.xilinx.com/support/documentation/sw_manuals/xilinx14_7/irn.pdf)

## Build Environment Setup

### Download and Install Xilinx Tools

Download and install Xilinx Vivado or Xilinx ISE based on the target SMINI.
- The recommended installation directory is `/opt/Xilinx/` for Linux and `C:\Xilinx` in Windows
- Please check the Xilinx Requirements document above for the FPGA technology used by your SMINI device.
- You may need to acquire a synthesis and implementation license from Xilinx to build some SMINI designs.
- You may need to acquire a simulation license from Xilinx to run some testbenches

### Download and Install ModelSim (Optional)

Download and install Mentor ModelSim using the link above.
- The recommended installation directory is `/opt/mentor/modelsim` for Linux and `C:\mentor\modelsim` in Windows
- Supported versions are PE, DE, SE, DE-64 and SE-64
- You may need to acquire a license from Mentor Graphics to run ModelSim

### Setting up build dependencies on Ubuntu

You can install all the dependencies through the package manager:

    sudo apt-get install python bash build-essential doxygen

Your actual command may differ.

### Setting up build dependencies on Fedora

You can install all the dependencies through the package manager:

    sudo yum -y install python bash make doxygen

Your actual command may differ.

### Setting up build dependencies on Windows (using Cygwin)

**NOTE**: Windows is only supported with Vivado. The build system does not support Xilinx ISE in Windows.

Download the latest version on [Cygwin](https://cygwin.com/install.html) (64-bit is preferred on a 64-bit OS)
and install it using [these instructions](http://x.cygwin.com/docs/ug/setup-cygwin-x-installing.html).
The following additional packages are also required and can be selected in the GUI installer

    python patch patchutils bash make doxygen

## Build Instructions (Xilinx Vivado only)

### Makefile based Builder

- Navigate to `smini3/top/{project}` where project is:
  + x300: For SMINI X300 and SMINI X310
  + e300: For SMINI E310

- To add vivado to the PATH and to setup up the Ettus Xilinx build environment run
  + `source setupenv.sh` (If Vivado is installed in the default path /opt/Xilinx/Vivado) _OR_
  + `source setupenv.sh --vivado-path=<VIVADO_PATH>` (where VIVADO_PATH is a non-default installation path)

- To build a binary configuration bitstream run `make <target>`
  where the target is specific to each product. To get a list of supported targets run
  `make help`.

- The build output will be specific to the product and will be located in the
  `smini3/top/{project}/build` directory. Run `make help` for more information.

### Environment Utilies

The build environment also defines many ease-of-use utilites. Please use the \subpage md_smini3_vivado_env_utils "Vivado Utility Reference" page for
a list and usage information

## Build Instructions (Xilinx ISE only)

### Makefile based Builder

- To add xtclsh to the PATH and to setup up the Xilinx build environment run
  + `source <install_dir>/Xilinx/14.7/ISE_DS/settings64.sh` (64-bit platform)
  + `source <install_dir>/Xilinx/14.7/ISE_DS/settings32.sh` (32-bit platform)

- Navigate to `smini3/top/{project}` where project is:
  + b200: For SMINI B200 and SMINI B210
  + b200mini: For SMINI B200mini

- To build a binary configuration bitstream run `make <target>`
  where the target is specific to each product. To get a list of supported targets run
  `make help`.

- The build output will be specific to the product and will be located in the
  `smini3/top/{project}/build` directory. Run `make help` for more information.

## Targets and Outputs

### B2x0 Targets and Outputs

#### Supported Targets
- B200:  Builds the SMINI B200 design.
- B210:  Builds the SMINI B210 design.

#### Outputs
- `build/smini_<product>_fpga.bit` : Configuration bitstream with header
- `build/smini_<product>_fpga.bin` : Configuration bitstream without header
- `build/smini_<product>_fpga.syr` : Xilinx system report
- `build/smini_<product>_fpga.twr` : Xilinx timing report

### X3x0 Targets and Outputs

#### Supported Targets
- X310_1G:  SMINI X310. 1GigE on both SFP+ ports. DRAM TX FIFO (experimental!).
- X300_1G:  SMINI X300. 1GigE on both SFP+ ports. DRAM TX FIFO (experimental!).
- X310_HG:  SMINI X310. 1GigE on SFP+ Port0, 10Gig on SFP+ Port1. DRAM TX FIFO (experimental!).
- X300_HG:  SMINI X300. 1GigE on SFP+ Port0, 10Gig on SFP+ Port1. DRAM TX FIFO (experimental!).
- X310_XG:  SMINI X310. 10GigE on both SFP+ ports. DRAM TX FIFO (experimental!).
- X300_XG:  SMINI X300. 10GigE on both SFP+ ports. DRAM TX FIFO (experimental!).
- X310_HGS: SMINI X310. 1GigE on SFP+ Port0, 10Gig on SFP+ Port1. SRAM TX FIFO.
- X300_HGS: SMINI X300. 1GigE on SFP+ Port0, 10Gig on SFP+ Port1. SRAM TX FIFO.
- X310_XGS: SMINI X310. 10GigE on both SFP+ ports. SRAM TX FIFO.
- X300_XGS: SMINI X300. 10GigE on both SFP+ ports. SRAM TX FIFO.

#### Outputs
- `build/smini_<product>_fpga_<image_type>.bit` :    Configuration bitstream with header
- `build/smini_<product>_fpga_<image_type>.bin` :    Configuration bitstream without header
- `build/smini_<product>_fpga_<image_type>.lvbitx` : Configuration bitstream for PCIe (NI-RIO)
- `build/smini_<product>_fpga_<image_type>.rpt` :    System, utilization and timing summary report

### E310 Targets and Outputs

#### Supported Targets
- E310:  Builds the SMINI E310 design.

#### Outputs
- `build/smini_<product>_fpga.bit` : Configuration bitstream with header
- `build/smini_<product>_fpga.bin` : Configuration bitstream without header
- `build/smini_<product>_fpga.rpt` : System, utilization and timing summary report

### Additional Build Options

It is possible to make a target and specific additional options in the form VAR=VALUE in
the command. For example: `make B210 PROJECT_ONLY=1`

Here are the supported options:

- `PROJECT_ONLY=1` : Only create a Xilinx project for the specified target(s). Useful for use with the ISE GUI. (*NOTE*: this option is only valid for Xilinx ISE)
- `EXPORT_ONLY=1` :  Export build targets from a GUI build to the build directory. Requires the project in build-\*_\* to be built. (*NOTE*: this option is only valid for Xilinx ISE)
- `GUI=1` : Run the Vivado build in GUI mode instead of batch mode. After the build is complete, Vivado provides an option to save the fully configured project for customization (*NOTE*: this option is only valid for Xilinx Vivado)

