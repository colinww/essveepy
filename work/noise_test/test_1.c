///////////////////////////////////////////////////////////////////////////////
//
// UCSD ISPG Group 2022
//
// Created on 13-Nov-22
// @author: Colin Weltin-Wu
//
// Description
// -----------
// Test uniform random doubles and white noise generation function.
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


#define NUM_RAND 1000000
#define SIGMA_MIN -1.5
#define SIGMA_MAX 1

int main(void) {
  // Open the data
  struct svp_hdf5_data *dat = svp_hdf5_fopen("test_1_data.h5");

  // Add some data to the file
  int dims[1] = {1};
  struct svp_dstore_t *ds1 =
      svp_dstore_create(dat->fptr, "u_top.rand", SVP_SYNC_DATA,
                        1, dims, H5T_NATIVE_DOUBLE);
  struct svp_dstore_t *ds2 = svp_dstore_create(
      dat->fptr, "u_top.randn", SVP_SYNC_DATA, 1, dims, H5T_NATIVE_DOUBLE);
  struct svp_dstore_t *ds3 = svp_dstore_create(
      dat->fptr, "u_top.randn_bnd", SVP_SYNC_DATA, 1, dims, H5T_NATIVE_DOUBLE);

  // Register the data
  svp_hdf5_addsig(dat, ds1);
  svp_hdf5_addsig(dat, ds2);
  svp_hdf5_addsig(dat, ds3);

  // Create a noise generator
  struct svp_rng_state_t gen = {};
  double samp = 0;
  // Write uniform random numbers
  for (int ii = 0; NUM_RAND > ii; ++ii) {
    // Populate data
    samp = svp_rng_rand();
    // Write data
    svp_dstore_write_double(ds1, 0, samp);
  }
  // Write normally-distributed random numbers
  for (int ii = 0; NUM_RAND > ii; ++ii) {
    // Populate data
    samp = svp_rng_randn(&gen);
    // Write data
    svp_dstore_write_double(ds2, 0, samp);
    // Populate data
    samp = svp_rng_randn_bnd(&gen, SIGMA_MIN, SIGMA_MAX);
    // Write data
    svp_dstore_write_double(ds3, 0, samp);
  }

  // Close the data
  svp_hdf5_fclose(dat);
}