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
void svp_dstore_hier_name(hid_t fid, const char *full_name, hid_t *gid,
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
}  // svp_dstore_hier_name


/**
 * @brief Flush the memory cache to the HD5 file.
 *
 * @param dat Data structure to be flushed.
 *
 * This method is used either before closing an HD5 file or each time the cache
 * is full, with CHUNK_SIZE entries. Writing the HD5 file in contiguous chunks
 * rather than element-by-element gains almost 100x speedup.
 */
void svp_dstore_flush(struct svp_dstore_t *dat) {
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
  // Write data (including svp_sim_time_t data)
  H5Dwrite(dat->dset, dat->d_mid, mspc, sspc, dat->xfer_id, dat->dcache);
  // Now update the write and cache pointers
  dat->wptr += dat->cptr;
  dat->cptr = 0;
  // Close views
  H5Sclose(sspc);
  H5Sclose(mspc);
}  // svp_dstore_flush


///////////////////////////////////////////////////////////////////////////////
// API
///////////////////////////////////////////////////////////////////////////////

struct svp_dstore_t *svp_dstore_create(hid_t fid, const char *name,
                                       enum svp_storage_e store_type, int rank,
                                       const int *dims, hid_t raw_type) {
  // Allocate a new data structure and 0-initialize
  struct svp_dstore_t *dat = malloc(sizeof(struct svp_dstore_t));
  memset(dat, 0, sizeof(struct svp_dstore_t));
  // First set any parameters that are simple
  dat->name = name;
  dat->store_type = store_type;
  // Create a resizable dataspace
  hsize_t cpd_dims[1] = {CHUNK_SIZE};
  hsize_t cpd_maxdims[1] = {H5S_UNLIMITED};
  hsize_t cpd_chunk_dims[1] = {CHUNK_SIZE};
  dat->dspc = H5Screate_simple(1, cpd_dims, cpd_maxdims);

  // Create the transfer ID to allow non-contiguous writing of data
  dat->xfer_id = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_preserve(dat->xfer_id, 1);

  // Determine what to do based on the storage enum
  switch (store_type) {
    case (SVP_SIM_TIME) :
      // Handle svp_sim_time_t first, since it is the most different in terms
      // of the dataset structure
      dat->h5type = H5T_NATIVE_LONG;
      dat->dims = malloc(sizeof(hsize_t));
      dat->dims[0] = 1;
      dat->rank = 1;
      // Create the time memoryview, which is the remainder
      dat->t_mid = H5Tcreate(H5T_COMPOUND, H5Tget_size(H5T_NATIVE_DOUBLE));
      H5Tinsert(dat->t_mid, "rem", 0, H5T_NATIVE_DOUBLE);
      // Create the data memoryview, which is the integer number of nanoseconds
      dat->d_mid = H5Tcreate(H5T_COMPOUND, H5Tget_size(H5T_NATIVE_LONG));
      H5Tinsert(dat->d_mid, "ns", 0, H5T_NATIVE_LONG);
      // Create the compound datatype, which has the layout of svp_sim_time_t
      dat->dtyp = H5Tcreate(H5T_COMPOUND, sizeof(struct svp_sim_time_t));
      // Insert elements
      H5Tinsert(dat->dtyp, "ns", HOFFSET(struct svp_sim_time_t, ns),
                H5T_NATIVE_LONG);
      H5Tinsert(dat->dtyp, "rem", HOFFSET(struct svp_sim_time_t, rem),
                H5T_NATIVE_DOUBLE);
      // Allocate both time and data caches
      dat->cstride = H5Tget_size(H5T_NATIVE_LONG);
      dat->tcache = (double *)malloc(CHUNK_SIZE * sizeof(double));
      dat->dcache = malloc(CHUNK_SIZE * H5Tget_size(H5T_NATIVE_LONG));
      break;
    case (SVP_ASYNC_DATA) :
      // Do special setup particular to asynchronous data, then fall through to
      // the regular synchronous data setup. First create the memoryview of
      // the double timestamps
      dat->t_mid = H5Tcreate(H5T_COMPOUND, H5Tget_size(H5T_NATIVE_DOUBLE));
      H5Tinsert(dat->t_mid, "time", 0, H5T_NATIVE_DOUBLE);
      // Allocate cache space
      dat->tcache = (double *)malloc(CHUNK_SIZE * sizeof(double));
    case (SVP_SYNC_DATA) :
      // Main data storage setup, for both synchronous and asynchronous types
      dat->h5type = raw_type;
      // Allocate dimension array, and check the data size
      dat->dims = malloc(rank * sizeof(hsize_t));
      dat->rank = rank;
      hsize_t dim_prod = 1;
      for (int ii = 0; rank > ii; ++ii) {
        dat->dims[ii] = dims[ii];
        dim_prod *= dims[ii];
      }
      if (MAX_FLAT_SIZE < dim_prod) {
        fprintf(stderr, "Data record size %ld exceeds maximum: %ld\n", dim_prod,
                MAX_FLAT_SIZE);
        return NULL;
      }
      // Create the data memoryview
      hid_t d_tid = H5Tarray_create2(raw_type, rank, dat->dims);
      dat->d_mid = H5Tcreate(H5T_COMPOUND, H5Tget_size(d_tid));
      H5Tinsert(dat->d_mid, "data", 0, d_tid);
      // Create the compound datatype, check if this is async
      hsize_t dofst = 0;
      if (dat->t_mid) {
        // We need to insert time in here
        dat->dtyp = H5Tcreate(
            H5T_COMPOUND, H5Tget_size(H5T_NATIVE_DOUBLE) + H5Tget_size(d_tid));
        H5Tinsert(dat->dtyp, "time", 0, H5T_NATIVE_DOUBLE);
        dofst = H5Tget_size(H5T_NATIVE_DOUBLE);
      } else {
        dat->dtyp = H5Tcreate(H5T_COMPOUND, H5Tget_size(d_tid));
      }
      // Finally, insert the data
      H5Tinsert(dat->dtyp, "data", dofst, d_tid);
      // Allocate the cache space
      dat->cstride = H5Tget_size(d_tid);
      dat->dcache = malloc(CHUNK_SIZE * H5Tget_size(d_tid));
  }  // switch (svp_storage_e)

  // Create the hierarchical name
  hid_t prop = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(prop, 1, cpd_chunk_dims);
  hid_t gid;
  char *sig_name;
  svp_dstore_hier_name(fid, name, &gid, &sig_name);
  dat->dset = H5Dcreate2(gid, sig_name, dat->dtyp, dat->dspc, H5P_DEFAULT, prop,
                         H5P_DEFAULT);
  H5Pclose(prop);

  // Return the data structure handle
  return dat;
}  // svp_dstore_create


void svp_dstore_close(struct svp_dstore_t *dat) {
  // Flush any outstanding data
  svp_dstore_flush(dat);
  // Resize the dataspace to only contain the number of elements written
  hid_t sspc = H5Dget_space(dat->dset);
  hsize_t cdims[1];
  H5Sget_simple_extent_dims(sspc, cdims, NULL);
  // Shrink down to the number of data points written
  cdims[0] = dat->wptr;
  H5Dset_extent(dat->dset, cdims);
  H5Sclose(sspc);
  // Close everything that was open
  if (dat->d_mid) {
    H5Tclose(dat->d_mid);
  }
  if (dat->t_mid) {
    H5Tclose(dat->t_mid);
  }
  H5Pclose(dat->xfer_id);
  H5Dclose(dat->dset);
  H5Tclose(dat->dtyp);
  H5Sclose(dat->dspc);
  // Free the cache data
  if (dat->dims) {
    free(dat->dims);
  }
  if (dat->tcache) {
    free(dat->tcache);
  }
  if (dat->dcache) {
    free(dat->dcache);
  }
  // Free the data
  free(dat);
}  // svp_dstore_close


int svp_dstore_write_data(struct svp_dstore_t *dat, double simtime,
                          const void *buf) {
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
    svp_dstore_flush(dat);
    // Increase size of dataset
    hid_t sspc = H5Dget_space(dat->dset);
    hsize_t cdims[1];
    H5Sget_simple_extent_dims(sspc, cdims, NULL);
    cdims[0] += CHUNK_SIZE;
    H5Dset_extent(dat->dset, cdims);
    H5Sclose(sspc);
  }
  return 0;
}  // svp_dstore_write_data


int svp_dstore_write_time(struct svp_dstore_t *dat,
                          struct svp_sim_time_t simtime) {
  // Check this is the correct storage type
  if (SVP_SIM_TIME != dat->store_type) {
    fprintf(stderr, "This signal: %s has the wrong storage type!\n", dat->name);
    return 1;
  }

  // Compute the memory address for the next svp_sim_time_t
  dat->tcache[dat->cptr] = simtime.rem;
  long *dptr = (long *)((char *)dat->dcache + (dat->cptr * dat->cstride));
  *dptr = simtime.ns;
  // Increment cache pointer
  dat->cptr += 1;

  // Check if the cache is full
  if (CHUNK_SIZE == dat->cptr) {
    // First flush the cache (this will update wptr/cptr)
    svp_dstore_flush(dat);
    // Increase size of dataset
    hid_t sspc = H5Dget_space(dat->dset);
    hsize_t cdims[1];
    H5Sget_simple_extent_dims(sspc, cdims, NULL);
    cdims[0] += CHUNK_SIZE;
    H5Dset_extent(dat->dset, cdims);
    H5Sclose(sspc);
  }
  return 0;
}  // svp_dstore_write_time
