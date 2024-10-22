// -*- verilog -*-
//
//  SMINI - Universal Software Radio Peripheral
//
//  Copyright (C) 2003 Matt Ettus
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Boston, MA  02110-1301  USA
//


module cic_decim
  ( clock,reset,enable,rate,strobe_in,strobe_out,signal_in,signal_out);
   parameter bw = 16;
   parameter N = 4;
   parameter log2_of_max_rate = 7;
   parameter maxbitgain = N * log2_of_max_rate;
   
   input clock;
   input reset;
   input enable;
   input [7:0] rate;
   input strobe_in,strobe_out;	
   input [bw-1:0] signal_in;
   output [bw-1:0] signal_out;
   reg [bw-1:0] signal_out;
   wire [bw-1:0] signal_out_unreg;
   
   wire [bw+maxbitgain-1:0] signal_in_ext;
   reg [bw+maxbitgain-1:0]  integrator [0:N-1];
   reg [bw+maxbitgain-1:0] differentiator [0:N-1];
   reg [bw+maxbitgain-1:0] pipeline [0:N-1];
   reg [bw+maxbitgain-1:0] sampler;
   
   integer i;
   
   sign_extend #(bw,bw+maxbitgain) 
      ext_input (.in(signal_in),.out(signal_in_ext));
   
   always @(posedge clock)
     if(reset)
       for(i=0;i<N;i=i+1)
	 integrator[i] <= #1 0;
     else if (enable && strobe_in)
       begin
	  integrator[0] <= #1 integrator[0] + signal_in_ext;
	  for(i=1;i<N;i=i+1)
	    integrator[i] <= #1 integrator[i] + integrator[i-1];
       end	
   
   always @(posedge clock)
     if(reset)
       begin
	  sampler <= #1 0;
	  for(i=0;i<N;i=i+1)
	    begin
	       pipeline[i] <= #1 0;
	       differentiator[i] <= #1 0;
	    end
       end
     else if (enable && strobe_out)
       begin
	  sampler <= #1 integrator[N-1];
	  differentiator[0] <= #1 sampler;
	  pipeline[0] <= #1 sampler - differentiator[0];
	  for(i=1;i<N;i=i+1)
	    begin
	       differentiator[i] <= #1 pipeline[i-1];
	       pipeline[i] <= #1 pipeline[i-1] - differentiator[i];
	    end
       end // if (enable && strobe_out)
      
   wire [bw+maxbitgain-1:0] signal_out_unnorm = pipeline[N-1];

   cic_dec_shifter #(bw)
	cic_dec_shifter(rate,signal_out_unnorm,signal_out_unreg);

   always @(posedge clock)
     signal_out <= #1 signal_out_unreg;
   
endmodule // cic_decim

