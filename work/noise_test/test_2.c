///////////////////////////////////////////////////////////////////////////////
//
// UCSD ISPG Group 2022
//
// Created on 13-Nov-22
// @author: Colin Weltin-Wu
//
// Description
// -----------
// Test flicker noise generation.
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
#include "../../csrc/svp_noise.h"


#define NUM_RAND 10000000
#define FS 1e9
#define SPOT_AMP 1e-18

int main(void) {
  // Open the data
  struct svp_hdf5_data *dat = svp_hdf5_fopen("test_2_data.h5");

  // Add some data to the file
  int dims[1] = {1};
  struct svp_dstore_t *ds1 =
      svp_dstore_create(dat->fptr, "u_top.flicker", SVP_SYNC_DATA,
                        1, dims, H5T_NATIVE_DOUBLE);

  // Register the data
  svp_hdf5_addsig(dat, ds1);

  // Create a flicker noise generator
  struct svp_rng_flicker_state_t *fgen =
      svp_rng_flicker_new(FS / 1e4, FS / 1e2, FS / 1e3, sqrt(SPOT_AMP), FS);

  // Generate noise samples
  double samp = 0;
  for (int ii = 0; NUM_RAND > ii; ++ii) {
    // Populate data
    samp = svp_rng_flicker_samp(fgen);
    // Write data
    svp_dstore_write_double(ds1, 0, samp);
  }

  // De-allocate flicker generator
  svp_rng_flicker_free(fgen);
    // Close the data
  svp_hdf5_fclose(dat);
}
