SMINI™ Tools
============================

This folder contains tools that are useful for working with and/or debugging
your SMINI™ device. Tools in this directory are not part of SHD. They are
either stand-alone programs or software to be used in third-party applications.

For SHD™ software tools, look in `shd/host/utils`.


## List of Tools

`__chdr-dissector/__`

This is a packet dissector for [Wireshark](http://www.wireshark.org/). It allows
you to view the details of a Compressed HeaDeR (CHDR) formatted-packet in
Wireshark. The SMINI™ B2xx and X3xx use the CHDR format.

`__shd_dump/__`

This tool can be used with `tcpdump` to make sense of packet dumps from your
network-connected SMINI™ device.

`__smini_x3xx_fpga_jtag_programmer.sh__`

This tool is to be used with the SMINI™ X300 and X310 devices. It allows you to
program the X3x0 FPGA via JTAG. Note that loading the FPGA image via JTAG does
**not** store the FPGA in the on-device flash storage. Thus, as soon as you
cycle power, the image will be lost. To permanently burn an FPGA image, please
refer to `shd/host/utils/smini_x3xx_fpga_burner`.

This tool requires that Xilinx iMPACT has been installed on your system.

`__kitchen_sink__`

This is a debugging tool designed to test and stress connections to SMINI
devices.
