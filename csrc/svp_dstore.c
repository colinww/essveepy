///////////////////////////////////////////////////////////////////////////////
//
// UCSD ISPG Group 2022
//
// Created on 12-Nov-22
// @author: Colin Weltin-Wu
//
// Description
// -----------
// Implementation of data storage API functions.
//
// Version History
// ---------------
// 12-Nov-22: Initial version
//
///////////////////////////////////////////////////////////////////////////////

#include "svp_dstore.h"

struct svp_dstore_t *svp_create_dstore(hid_t fid, const char *name,
                                       int is_async, int rank, const int *dims,
                                       hid_t raw_type) {
  // Allocate a new data structure
  struct svp_dstore_t *dat = malloc(sizeof(struct svp_dstore_t));
  // First copy the parameters that are simple
  dat->name = name;
  dat->wptr = 0;
  dat->size = CHUNK_SIZE;
  dat->h5type = raw_type;
  // Check that the rank is smaller than maximum
  if (MAX_RANK < rank) {
    fprintf(stderr, "ERROR: array rank %d is larger than maximum: %d\n",
            rank, MAX_RANK);
  }
  dat->rank = rank;
  for (int ii = 0; rank > ii; ++ii) {
    dat->dims[ii] = dims[ii];
  }

  // Create the memory dataspace
  hsize_t mdims[1] = {1};
  dat->mspc = H5Screate_simple(1, mdims, NULL);
  dat->xfer_id = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_preserve(dat->xfer_id, 1);
  // Create the data memoryview
  hsize_t arr_dims[MAX_RANK];
  for (int ii = 0; rank > ii; ++ii) {
    arr_dims[ii] = dims[ii];
  }
  hid_t d_tid = H5Tarray_create2(raw_type, rank, arr_dims);
  dat->d_mid = H5Tcreate(H5T_COMPOUND, H5Tget_size(d_tid));
  H5Tinsert(dat->d_mid, "data", 0, d_tid);
  // Determine whether or not time will be stored
  if (is_async) {
    // Async signal, so store time as well as data. Create the memoryview
    dat->t_mid = H5Tcreate(H5T_COMPOUND, sizeof(H5T_NATIVE_DOUBLE));
    H5Tinsert(dat->t_mid, "time", 0, H5T_NATIVE_DOUBLE);
    // Now construct the main compound datatype
    hsize_t cpd_size = sizeof(H5T_NATIVE_DOUBLE) + H5Tget_size(d_tid);
    dat->dtyp = H5Tcreate(H5T_COMPOUND, cpd_size);
    // Insert the time and data
    H5Tinsert(dat->dtyp, "time", 0, H5T_NATIVE_DOUBLE);
    H5Tinsert(dat->dtyp, "data", sizeof(H5T_NATIVE_DOUBLE), d_tid);
  } else {
    dat->t_mid = 0;
    // Main datatype is just the bare data array
    hsize_t cpd_size = H5Tget_size(d_tid);
    dat->dtyp = H5Tcreate(H5T_COMPOUND, cpd_size);
    // Insert the data
    H5Tinsert(dat->dtyp, "data", 0, d_tid);
  }

  // Create a resizable dataspace and dataset with this compound type
  hsize_t cpd_dims[1] = {CHUNK_SIZE};
  hsize_t cpd_maxdims[1] = {H5S_UNLIMITED};
  hsize_t cpd_chunk_dims[1] = {CHUNK_SIZE};
  dat->dspc = H5Screate_simple(1, cpd_dims, cpd_maxdims);
  hid_t prop = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(prop, 1, cpd_chunk_dims);
  dat->dset = H5Dcreate2(fid, name, dat->dtyp, dat->dspc, H5P_DEFAULT, prop,
                         H5P_DEFAULT);
  H5Pclose(prop);
  // Return the data structure handle
  return dat;
}  // svp_create_dstore


void svp_close_dstore(struct svp_dstore_t *dat) {
  // Resize the dataspace to only contain the number of elements written
  hid_t sspc = H5Dget_space(dat->dset);
  hsize_t cdims[1];
  H5Sget_simple_extent_dims(sspc, cdims, NULL);
  // Shrink down to the number of data points written
  cdims[0] -= (dat->size - dat->wptr);
  H5Dset_extent(dat->dset, cdims);
  H5Sclose(sspc);
  // Close everything that was open
  H5Tclose(dat->d_mid);
  if (dat->t_mid) {
    H5Tclose(dat->t_mid);
  }
  H5Pclose(dat->xfer_id);
  H5Sclose(dat->mspc);
  H5Tclose(dat->dtyp);
  H5Dclose(dat->dset);
  H5Sclose(dat->dspc);
  // Free the data
  free(dat);
}  // svp_close_dstore


void svp_write_data(struct svp_dstore_t *dat, double simtime, const void *buf) {
  hid_t sspc;
  // First check if the current chunk if filled up
  if (dat->size == dat->wptr) {
    sspc = H5Dget_space(dat->dset);
    hsize_t cdims[1];
    H5Sget_simple_extent_dims(sspc, cdims, NULL);
    cdims[0] += CHUNK_SIZE;
    H5Dset_extent(dat->dset, cdims);
    H5Sclose(sspc);
    // Increase size
    dat->size += CHUNK_SIZE;
  }

  // Initialize the write
  sspc = H5Dget_space(dat->dset);
  hsize_t cnt[1] = {1};
  hsize_t ofst[1] = {0};
  ofst[0] = dat->wptr;
  H5Sselect_hyperslab(sspc, H5S_SELECT_SET, ofst, NULL, cnt, NULL);
  // If this is an async signal, write the time datapoint
  if (dat->t_mid) {
    H5Dwrite(dat->dset, dat->t_mid, dat->mspc, sspc, dat->xfer_id, &simtime);
  }
  // Write data
  H5Dwrite(dat->dset, dat->d_mid, dat->mspc, sspc, dat->xfer_id, buf);

  // Increment the write pointer
  dat->wptr += 1;
}  // svp_write_data
