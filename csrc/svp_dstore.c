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
// 12-Nov-22: Initial version.
// 13-Nov-22: Added caching and hierarchical group paths.
//
///////////////////////////////////////////////////////////////////////////////

#include "svp_dstore.h"

///////////////////////////////////////////////////////////////////////////////
// Internal functions
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Flush the memory cache to the HD5 file.
 *
 * @param dat Data structure to be flushed.
 *
 * This method is used either before closing an HD5 file or each time the cache
 * is full, with CHUNK_SIZE entries. Writing the HD5 file in contiguous chunks
 * rather than element-by-element gains almost 100x speedup.
 */
void svp_flush_cache(struct svp_dstore_t *dat) {
  // Define the memory view of the cache source data
  hsize_t mdims[1] = {};
  mdims[0] = dat->cptr;
  hid_t mspc = H5Screate_simple(1, mdims, NULL);
  // Get the subspace from dataset
  hid_t sspc = H5Dget_space(dat->dset);
  // Define the hyperslab where data will be written
  hsize_t cnt[1] = {0};
  hsize_t ofst[1] = {0};
  cnt[0] = dat->cptr;
  ofst[0] = dat->wptr;
  H5Sselect_hyperslab(sspc, H5S_SELECT_SET, ofst, NULL, cnt, NULL);
  // Write data
  if (dat->t_mid) {
    H5Dwrite(dat->dset, dat->t_mid, mspc, sspc, dat->xfer_id, dat->tcache);
  }
  // Write data
  H5Dwrite(dat->dset, dat->d_mid, mspc, sspc, dat->xfer_id, dat->dcache);
  // Now update the write and cache pointers
  dat->wptr += dat->cptr;
  dat->cptr = 0;
  // Close views
  H5Sclose(sspc);
  H5Sclose(mspc);
}  // svp_flush_cache

/**
 * @brief Reconstruct a signal absolute path hierarchy in HD5 groups.
 *
 * @param fid Starting object, usually file ID.
 * @param full_name The full path to the signal.
 * @param gid Final group ID containing the actual signal.
 * @param sig_name Actual signal name.
 *
 * This performs a similar function to the python os.mkdirs() function, in that
 * any intermediate hierarchy (containing submodules) are constructed as HD5
 * groups if they do not already exist.
 */
void svp_hier_name(hid_t fid, const char *full_name, hid_t *gid,
                   char **sig_name) {
  // Copy the string to a new location
  char *name_cpy = (char *)malloc(strlen(full_name) + 1);
  strcpy(name_cpy, full_name);
  // Token-ize the name by the '.' hierarchy separators
  const char *delim = ".";
  // This contains the offset within the full_name of the first character of
  // the signal name
  int name_start = 0;
  // Starting point group is the root '/' within the file
  *gid = H5Gopen(fid, "/", H5P_DEFAULT);
  hid_t next_gid;
  // Apply first tokenization, to load string into strtok
  char *token = strtok(name_cpy, delim);
  // Split token by delimiter
  char *next_token = strtok(NULL, delim);
  while (next_token) {
    // If next_token is found, it means we just removed one hierarchy separator
    // Increment the name starting point offset (plus delimiter)
    name_start += next_token - token;
    // Descend into the hierarchy by trying to open the next group
    if (0 == H5Lexists(*gid, token, H5P_DEFAULT)) {
      // This group does not exist yet, so create it
      next_gid = H5Gcreate(*gid, token, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    } else {
      next_gid = H5Gopen(*gid, token, H5P_DEFAULT);
    }
    H5Gclose(*gid);
    // Assign gid to the next and continue
    *gid = next_gid;
    // Continue tokenization
    token = next_token;
    next_token = strtok(NULL, delim);
  }
  // When next_token is null, it means that token contains the final name, and
  // name_start is the correct offset from the original full_name
  *sig_name = (char *)((unsigned long)full_name + name_start);
}  // svp_hier_name

///////////////////////////////////////////////////////////////////////////////
// API
///////////////////////////////////////////////////////////////////////////////

struct svp_dstore_t *svp_create_dstore(hid_t fid, const char *name,
                                       int is_async, int rank, const int *dims,
                                       hid_t raw_type) {
  // Allocate a new data structure
  struct svp_dstore_t *dat = malloc(sizeof(struct svp_dstore_t));
  // First copy the parameters that are simple
  dat->name = name;
  dat->wptr = 0;
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

  // Create the transfer ID to allow non-contiguous writing of async data
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
    dat->t_mid = H5Tcreate(H5T_COMPOUND, H5Tget_size(H5T_NATIVE_DOUBLE));
    H5Tinsert(dat->t_mid, "time", 0, H5T_NATIVE_DOUBLE);
    // Now construct the main compound datatype
    hsize_t cpd_size = H5Tget_size(H5T_NATIVE_DOUBLE) + H5Tget_size(d_tid);
    dat->dtyp = H5Tcreate(H5T_COMPOUND, cpd_size);
    // Insert the time and data
    H5Tinsert(dat->dtyp, "time", 0, H5T_NATIVE_DOUBLE);
    H5Tinsert(dat->dtyp, "data", H5Tget_size(H5T_NATIVE_DOUBLE), d_tid);
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
  // Create the hierarchical name
  hid_t gid;
  char *sig_name;
  svp_hier_name(fid, name, &gid, &sig_name);
  dat->dset = H5Dcreate2(gid, sig_name, dat->dtyp, dat->dspc, H5P_DEFAULT, prop,
                         H5P_DEFAULT);
  H5Pclose(prop);

  // Allocate space for the cache data
  dat->cptr = 0;
  dat->cstride = H5Tget_size(d_tid);
  if (is_async) {
    // Allocate timestamp cache
    dat->tcache = (double *)malloc(CHUNK_SIZE * sizeof(double));
  } else {
    dat->tcache = NULL;
  }
  // Data cache
  dat->dcache = malloc(CHUNK_SIZE * H5Tget_size(d_tid));

  // Return the data structure handle
  return dat;
}  // svp_create_dstore


void svp_close_dstore(struct svp_dstore_t *dat) {
  // Flush any outstanding data
  svp_flush_cache(dat);
  // Resize the dataspace to only contain the number of elements written
  hid_t sspc = H5Dget_space(dat->dset);
  hsize_t cdims[1];
  H5Sget_simple_extent_dims(sspc, cdims, NULL);
  // Shrink down to the number of data points written
  cdims[0] = dat->wptr;
  H5Dset_extent(dat->dset, cdims);
  H5Sclose(sspc);
  // Close everything that was open
  H5Tclose(dat->d_mid);
  if (dat->t_mid) {
    H5Tclose(dat->t_mid);
  }
  H5Pclose(dat->xfer_id);
  H5Tclose(dat->dtyp);
  H5Dclose(dat->dset);
  H5Sclose(dat->dspc);

  // Free the cache data
  if (dat->tcache) {
    free(dat->tcache);
  }
  free(dat->dcache);

  // Free the data
  free(dat);
}  // svp_close_dstore


void svp_write_data(struct svp_dstore_t *dat, double simtime, const void *buf) {
  // Write timestamp to the time cache if this is async signal
  if (dat->t_mid) {
    dat->tcache[dat->cptr] = simtime;
  }
  // Compute the memory address for next data sample
  void *dptr = (void *)((char *)dat->dcache + (dat->cptr * dat->cstride));
  // Copy chunk of bytes
  memcpy(dptr, buf, dat->cstride);
  // Increment cache pointer
  dat->cptr += 1;

  // Check if the cache is full
  if (CHUNK_SIZE == dat->cptr) {
    // First flush the cache (this will update wptr/cptr)
    svp_flush_cache(dat);
    // Increase size of dataset
    hid_t sspc = H5Dget_space(dat->dset);
    hsize_t cdims[1];
    H5Sget_simple_extent_dims(sspc, cdims, NULL);
    cdims[0] += CHUNK_SIZE;
    H5Dset_extent(dat->dset, cdims);
    H5Sclose(sspc);
  }
}  // svp_write_data
