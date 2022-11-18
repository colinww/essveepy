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

module submod
  (
    input logic clk,
    input logic rstb,
    input logic [7:0] logic_sig,
    input longint     unpacked_ints[4],
    input real        unpacked_reals[4]
  );
  logic [7:0]   local_sig;
  longint       local_ints[4];
  real          local_reals[4];


  int           ii;
  
  
  always @(posedge clk or negedge rstb) begin
    if (!rstb) begin
      local_sig  = 0;
      for (ii = 0; 4 > ii; ii++) begin
        local_ints[ii]   = 0;
        local_reals[ii]  = 0;
      end
    end else begin // if (!rstb)
      local_sig  = logic_sig;
      for (ii = 0; 4 > ii; ii++) begin
        local_ints[ii]   = unpacked_ints[ii];
        local_reals[ii]  = unpacked_reals[ii];
      end
    end // else: !if(!rstb)
  end // always @ (posedge clk or negedge rstb)


endmodule // submod
