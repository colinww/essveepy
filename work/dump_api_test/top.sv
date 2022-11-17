///////////////////////////////////////////////////////////////////////////////
//
// UCSD ISPG Group 2022
//
// Created on 12-Nov-22
// @author: Colin Weltin-Wu
//
// Description
// -----------
// Test all the permutations of the dumping tools.
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
  localparam real T_RUN = 10000;

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
  // Bit vectors
  bit [5:0] bit6;
  bit [7:0] bit8;
  bit [13:0] bit14;
  bit [1:0][20:0] bit21a2;
  bit [3:0][40:0] bit41a2;
  // Integers
  byte int8;
  shortint int16;
  int int32;
  longint int64;
  byte int8a4[4];
  // Reals
  real real_scalar;
  real real_arr[3];
  real real_async_scalar;
  // Simtime
  svp_sim_time_t simtime;

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

  ////////////////////////
  // Generate the stimulus
  ////////////////////////
  // Free-running counter
  longint ctr = 0;
  always @(negedge clk or negedge rstb) begin
    if (!rstb) begin
      ctr = 0;
    end else begin
      ctr += 1;
    end
  end

  // Bit bssignments
  assign bit6 = ctr % 64;
  assign bit8 = ctr % 256;
  assign bit14 = (ctr % 16384) - 8192;
  assign bit21a2[0] = ctr;
  assign bit21a2[1] = ctr - 1;
  assign bit41a2[0] = ctr;
  assign bit41a2[1] = ~ctr;
  // Integer assignments
  assign int8 = ctr;
  assign int16 = -ctr;
  assign int32 = -ctr;
  assign int64 = ctr;
  assign int8a4[0] = 4 * ctr;
  assign int8a4[1] = 4 * ctr + 1;
  assign int8a4[2] = 4 * ctr + 2;
  assign int8a4[3] = 4 * ctr + 3;
  // Real assignments
  assign real_scalar = ctr;
  assign real_arr[0] = ctr;
  assign real_arr[1] = real'(ctr) / 2.0;
  assign real_arr[2] = real'(ctr) / 4.0;
  assign real_async_scalar = -ctr;
  // Simtime
  assign simtime.ns = ctr;
  assign simtime.rem = real'(ctr) * 1e-9;

  ///////////////
  // Data writers
  ///////////////
  svpDumpFile fobj;
  svpBitDump #(.SIGNED(0), .WIDTH(6)) dump_bit6_unsigned;
  svpBitDump #(.SIGNED(1), .WIDTH(6)) dump_bit6_signed;
  svpBitDump #(.SIGNED(0), .WIDTH(8)) dump_bit8_unsigned;
  svpBitDump #(.SIGNED(1), .WIDTH(8)) dump_bit8_signed;
  svpBitDump #(.SIGNED(1), .WIDTH(14)) dump_bit14_signed;
  svpBitArrayDump #(.WIDTH(21), .SIZE(2)) dump_bit21a2;
  svpBitArrayDump #(.WIDTH(41), .SIZE(2)) dump_bit41a2;
  svpIntegerDump #(.T(byte)) dump_int8;
  svpIntegerDump #(.T(shortint)) dump_int16;
  svpIntegerDump #(.T(int)) dump_int32;
  svpIntegerDump #(.T(longint)) dump_int64;
  svpIntegerArrayDump #(.T(byte), .SIZE(4)) dump_int8a4;
  svpRealDump #(.ASYNC(0)) dump_real_scalar;
  svpRealDump #(.ASYNC(1)) dump_real_async_scalar;
  svpRealArrayDump #(.SIZE(3)) dump_real_arr3;
  svpTimeDump dump_time;

  initial begin
    // Initialize file
    fobj = new("sv_data_dump.h5");
    // Register all signals
    dump_bit6_unsigned = new(fobj, "top.bit_type.u6");
    dump_bit6_signed = new(fobj, "top.bit_type.s6");
    dump_bit8_unsigned = new(fobj, "top.bit_type.u8");
    dump_bit8_signed = new(fobj, "top.bit_type.s8");
    dump_bit14_signed = new(fobj, "top.bit_type.s14");
    dump_bit21a2 = new(fobj, "top.bit_type.arrays.s21a2");
    dump_bit41a2 = new(fobj, "top.bit_type.arrays.s41a2");
    dump_int8 = new(fobj, "top.integer_types.i8");
    dump_int16 = new(fobj, "top.integer_types.i16");
    dump_int32 = new(fobj, "top.integer_types.i32");
    dump_int64 = new(fobj, "top.integer_types.i64");
    dump_int8a4 = new(fobj, "top.integer_types.arrays.i8a4");
    dump_real_scalar = new(fobj, "top.real_type.sync_data");
    dump_real_async_scalar = new(fobj, "top.real_type.async_data");
    dump_real_arr3 = new(fobj, "top.real_type.arrays.arr3");
    dump_time = new(fobj, "top.timestamps");
  end
  final begin
    fobj.close();
  end

  /////////////////////
  // Write data to file
  /////////////////////

  always @(posedge clk) begin
    dump_bit6_unsigned.write(bit6);
    dump_bit6_signed.write(bit6);
    dump_bit8_unsigned.write(bit8);
    dump_bit8_signed.write(bit8);
    dump_bit14_signed.write(bit14);
    dump_bit21a2.write(bit21a2);
    dump_bit41a2.write(bit41a2);
    dump_int8.write(int8);
    dump_int16.write(int16);
    dump_int32.write(int32);
    dump_int64.write(int64);
    dump_int8a4.write(int8a4);
    dump_real_scalar.write(real_scalar);
    dump_real_async_scalar.write(real_async_scalar);
    dump_real_arr3.write(real_arr);
    dump_time.write(simtime);
  end


endmodule : top
`endif
