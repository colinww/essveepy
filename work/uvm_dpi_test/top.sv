///////////////////////////////////////////////////////////////////////////////
//
// UCSD ISPG Group 2022
//
// Created on 12-Nov-22
// @author: Colin Weltin-Wu
//
// Description
// -----------
// Test a simple UVM DPI call.
//
// Version History
// ---------------
// 12-Nov-22: Initial version
//
///////////////////////////////////////////////////////////////////////////////

`ifndef TOP
`define TOP

import uvm_pkg::*;

module top;

  import "DPI-C" context function int uvm_hdl_check_path(string path); 

  import "DPI-C" context function int uvm_hdl_read(string path,
                                                   output uvm_hdl_data_t value);


  //////////////////////////
  // Testbench configuration
  //////////////////////////
  localparam real T_RUN = 100;

  //////////
  // Signals
  //////////
  logic clk;
  logic rstb;
  logic [7:0] logic_sig;
  longint     unpacked_ints[4];
  real        unpacked_reals[4];
  
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

  ///////////
  // Stimulus
  ///////////
  int ii;
  initial begin
    logic_sig  = 0;
    for (ii = 0; 4 > ii; ii++) begin
      unpacked_ints[ii]   = 0;
      unpacked_reals[ii]  = 0;
    end
  end
  
  always @(negedge clk) begin
    logic_sig  = logic_sig + 1;
    for (ii = 0; 4 > ii; ii++) begin
      unpacked_ints[ii]   = unpacked_ints[ii] + 1;
      unpacked_reals[ii]  = unpacked_reals[ii] + 0.1;
    end
  end // always @ (negedge clk)

  ////////////
  // Submodule
  ////////////

  submod u_submod
  (
    .clk(clk),
    .rstb(rstb),
    .logic_sig(logic_sig),
    .unpacked_ints(unpacked_ints),
    .unpacked_reals(unpacked_reals)
  );


  // Checks
  initial begin
    $display("Valid path: %d", uvm_hdl_check_path("top.u_submod.local_ints"));
  end

  logic [7:0] rb_sig;
  longint     rb_int;
  real rb_real;
  always @(posedge clk) begin
    void'(uvm_hdl_read("top.u_submod.local_sig", rb_sig));
    void'(uvm_hdl_read("top.u_submod.local_ints[0]", rb_int));
    void'(uvm_hdl_read("top.u_submod.local_reals[0]", rb_real));
    $display("Current time: %g", $realtime);
    $display("  Logic [7:0] value: %d", rb_sig);
    $display("  Integer value: %d", rb_int);
    $display("  Real value: %g", rb_real + 0.1);
  end
  
  
endmodule : top
`endif
