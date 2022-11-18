///////////////////////////////////////////////////////////////////////////////
//
// UCSD ISPG Group 2022
//
// Created on 12-Nov-22
// @author: Colin Weltin-Wu
//
// Description
// -----------
// This defines the file interface for the data writer functions.
//
// Version History
// ---------------
// 12-Nov-22: Initial version
//
///////////////////////////////////////////////////////////////////////////////

#include "svp_file.h"
#include "svp_dstore.h"

struct svp_hdf5_data *svp_hdf5_fopen(const char *fname) {
  // Allocate class data
  struct svp_hdf5_data *clsdat = malloc(sizeof(struct svp_hdf5_data));
  // Open the file
  clsdat->fptr = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  // Save file name
  clsdat->name = fname;
  // Clear the data counter
  clsdat->num_signals = 0;
  // Allocate space for the data store
  clsdat->dptr = (struct svp_dstore_t **)malloc(MAX_SIGNALS *
                                                sizeof(struct svp_dstore_t *));
  for (int ii = 0; MAX_SIGNALS > ii; ++ii) {
    clsdat->dptr[ii] = NULL;
  }
  // Return new data store
  return clsdat;
}  // svp_hdf5_fopen


int svp_hdf5_addsig(struct svp_hdf5_data *clsdat, struct svp_dstore_t *dat) {
  // Check if maximum number of files has been reached
  if (MAX_SIGNALS == clsdat->num_signals) {
    fprintf(stderr, "Maximum number of signals has been reached: %d\n",
            MAX_SIGNALS);
    return 1;
  }
  // Otherwise add the signal to the list, increment count
  clsdat->dptr[clsdat->num_signals] = dat;
  clsdat->num_signals += 1;
  return 0;
}  // svp_hdf5_addsig


int svp_hdf5_fclose(struct svp_hdf5_data *clsdat) {
  for (int ii = 0; clsdat->num_signals > ii; ++ii) {
    svp_dstore_close(clsdat->dptr[ii]);
  }
  // Free the data store
  free(clsdat->dptr);
  // Close the file
  H5Fclose(clsdat->fptr);
  // Delete the class data
  free(clsdat);
  return 0;
}  // svp_hdf5_fclose


void svp_hdf5_add_attribute(struct svp_hdf5_data *clsdat, char *name,
                            char *value) {
  svp_add_attr(clsdat->fptr, name, value);
}  // svp_hdf5_add_attribute
