///////////////////////////////////////////////////////////////////////////////
//
// UCSD ISPG Group 2022
//
// Created on 12-Nov-22
// @author: Colin Weltin-Wu
//
// Description
// -----------
// Testing noise generation functions.
//
// Version History
// ---------------
// 12-Nov-22: Initial version
//
///////////////////////////////////////////////////////////////////////////////

`ifndef TOP
`define TOP

import svp_pkg::*;

module top;

  //////////////////////////
  // Testbench configuration
  //////////////////////////
  localparam real T_RUN = 10000000;

  localparam real RANDN_BND_MIN = -1.5;
  localparam real RANDN_BND_MAX = 1;


  localparam real FMIN = 1e4;
  localparam real FMAX = 1e8;
  localparam real FSPOT = 1e6;
  localparam real ASPOT = 1e-18;

  //////////
  // Signals
  //////////
  logic clk;
  logic rstb;
  real rand_samps[3];
  real flicker_sig;

  /////////////////////////////
  // Clock and reset generation
  /////////////////////////////
  // Reset
  initial begin
    rstb = 0;
    #(0.75) rstb = 1;
  end
  // Clock
  initial begin
    clk = 1;
    forever begin
      #(0.5) clk = !clk;
    end
  end
  // Run simulation
  initial begin
    #(T_RUN) $finish();
  end

  ////////////////////////////////
  // Allocate the noise generators
  ////////////////////////////////

  svpRandom gen;
  svpFlicker flkr;
  initial begin
    gen = new();
    flkr = new(FMIN, FMAX, FSPOT, $sqrt(ASPOT), 1e9);
  end
  final begin
    flkr.free();
  end

  ///////////////
  // Data writers
  ///////////////
  svpDumpFile fobj;
  svpRealArrayDump#(.SIZE(3)) dump_rand;
  svpRealDump dump_flicker;
  initial begin
    fobj = new("sv_data_dump.h5");
    dump_rand = new(fobj, "top.random");
    dump_flicker = new(fobj, "top.flicker");
  end
  final begin
    fobj.close();
  end

  ///////////////////////////
  // Generate the random data
  ///////////////////////////



  always @(posedge clk or negedge rstb) begin
    if (!rstb) begin
      rand_samps = '{default:0};
      flicker_sig = 0;
    end else begin
      rand_samps[0] = gen.urand();
      rand_samps[1] = gen.randn();
      rand_samps[2] = gen.randn_bnd(RANDN_BND_MIN, RANDN_BND_MAX);
      flicker_sig = flkr.samp();
      // Write array samples out
      dump_rand.write(rand_samps);
      dump_flicker.write(flicker_sig);
    end
  end


endmodule : top
`endif
