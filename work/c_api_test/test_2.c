///////////////////////////////////////////////////////////////////////////////
//
// UCSD ISPG Group 2022
//
// Created on 13-Nov-22
// @author: Colin Weltin-Wu
//
// Description
// -----------
// Test the C-API, high-precision time storage.
// Test both methods of writing the time object.
//
//
// Version History
// ---------------
// 13-Nov-22: Initial version
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../csrc/svp_file.h"

#define NUM_WRITE 1000000

int main(void) {
  // Open the data
  struct svp_hdf5_data *dat = svp_hdf5_fopen("test_2_data.h5");

  // Create a timestamp entry
  struct svp_dstore_t *ds1 = svp_dstore_create(dat->fptr, "u_top.ts_write_time",
                                               SVP_SIM_TIME, 0, NULL, 0);
  struct svp_dstore_t *ds2 = svp_dstore_create(dat->fptr, "u_top.ts_write_data",
                                               SVP_SIM_TIME, 0, NULL, 0);
  // Register
  svp_hdf5_addsig(dat, ds1);
  svp_hdf5_addsig(dat, ds2);

  // Populate some data
  struct svp_sim_time_t ts;
  for (int ii = 0; NUM_WRITE > ii; ++ii) {
    ts.ns = ii;
    ts.rem = 1e-9 * ii;
    svp_dstore_write_time(ds1, ts);
    svp_dstore_write_long(ds2, ts.rem, ts.ns);
  }

  // Close the data
  svp_hdf5_fclose(dat);
}
