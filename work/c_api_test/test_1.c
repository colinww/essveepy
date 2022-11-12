///////////////////////////////////////////////////////////////////////////////
//
// UCSD ISPG Group 2022
//
// Created on 12-Nov-22
// @author: Colin Weltin-Wu
//
// Description
// -----------
// Test the C-API.
//
//
// Version History
// ---------------
// 12-Nov-22: Initial version
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../csrc/svp_file.h"


#define NUM_WRITE_1 100000
#define NUM_WRITE_2 100000
#define NUM_WRITE_3 100000

int main(void) {
  // Open the data
  struct svp_cls *dat = svp_fopen("test_1_data.h5");

  // Add some data to the file
  int d1_dims[2] = {2, 3};
  struct svp_dstore_t *ds1 = svp_create_dstore(dat->fptr, "sync_long_2x3", 0, 2,
                                               d1_dims, H5T_NATIVE_LONG);
  int d2_dims[1] = {1};
  struct svp_dstore_t *ds2 = svp_create_dstore(dat->fptr, "sync_long_1", 0, 1,
                                               d2_dims, H5T_NATIVE_LONG);
  int d3_dims[1] = {4};
  struct svp_dstore_t *ds3 = svp_create_dstore(dat->fptr, "async_double_4", 1,
                                               1, d3_dims, H5T_NATIVE_DOUBLE);
  // Register the data
  svp_register(dat, ds1);
  svp_register(dat, ds2);
  svp_register(dat, ds3);

  // Write to long 2x3 array
  long dwrite1[2][3];
  for (int ii = 0; NUM_WRITE_1 > ii; ++ii) {
    // Populate data
    for (int jj = 0; 2 > jj; ++jj) {
      for (int kk = 0; 3 > kk; ++kk) {
        dwrite1[jj][kk] = 6 * ii + 3 * jj + kk;
      }
    }
    // Write data
    svp_write_data(ds1, 0, dwrite1);
  }

  // Write to singleton long
  long dwrite2[1];
  for (int ii = 0; NUM_WRITE_2 > ii; ++ii) {
    // Populate data
    dwrite2[0] = -ii;
    // Write data
    svp_write_data(ds2, 0, dwrite2);
  }

  // Write to asynchronous array of 4 doubles
  double dwrite3[4];
  double dtime;
  for (int ii = 0; NUM_WRITE_3 > ii; ++ii) {
    // Populate data
    for (int jj = 0; 4 > jj; ++jj) {
      dwrite3[jj] = 4 * ii + jj;
    }
    dtime = 1e-9 * ii;
    // Write data
    svp_write_data(ds3, dtime, dwrite3);
  }

  // Close the data
  svp_fclose(dat);
}