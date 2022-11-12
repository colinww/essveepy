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

struct svp_cls *svp_fopen(const char *fname) {
  // Allocate class data
  struct svp_cls *clsdat = malloc(sizeof(struct svp_cls));
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
}  // svp_fopen


int svp_register(struct svp_cls *clsdat, struct svp_dstore_t *dat) {
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
}  // svp_register


int svp_fclose(struct svp_cls *clsdat) {
  for (int ii = 0; clsdat->num_signals > ii; ++ii) {
    svp_close_dstore(clsdat->dptr[ii]);
  }
  // Free the data store
  free(clsdat->dptr);
  // Close the file
  H5Fclose(clsdat->fptr);
  // Delete the class data
  free(clsdat);
}  // svp_fclose
