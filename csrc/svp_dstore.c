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

struct svp_dstore_t *svp_dstore_create(struct svp_hdf5_data *clsdat,
                                       const char *name,
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
    case (SVP_STORE_SIM_TIME) :
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
    case (SVP_STORE_ASYNC_DATA) :
      // Do special setup particular to asynchronous data, then fall through to
      // the regular synchronous data setup. First create the memoryview of
      // the double timestamps
      dat->t_mid = H5Tcreate(H5T_COMPOUND, H5Tget_size(H5T_NATIVE_DOUBLE));
      H5Tinsert(dat->t_mid, "time", 0, H5T_NATIVE_DOUBLE);
      // Allocate cache space
      dat->tcache = (double *)malloc(CHUNK_SIZE * sizeof(double));
    case (SVP_STORE_SYNC_DATA) :
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
  svp_group_hierarchy_split(clsdat->fptr, name, &gid, &sig_name);
  dat->dset = H5Dcreate2(gid, sig_name, dat->dtyp, dat->dspc, H5P_DEFAULT, prop,
                         H5P_DEFAULT);
  H5Pclose(prop);
  // Add attributes to the dataset
  switch (store_type) {
    case (SVP_STORE_SIM_TIME) :
      svp_add_attr(dat->dset, "storage", "time");
      break;
    case (SVP_STORE_ASYNC_DATA) :
      svp_add_attr(dat->dset, "storage", "async");
      break;
    case (SVP_STORE_SYNC_DATA) :
      svp_add_attr(dat->dset, "storage", "sync");
      break;
  }
  // Return the data structure handle
  return dat;
}  // svp_dstore_create


struct svp_dstore_t *svp_dstore_svcreate(struct svp_hdf5_data *clsdat,
                                         const char *name, int is_async,
                                         int width, const char *dtype) {
  // Single dimensional array
  int dims[1] = {0};
  dims[0] = width;
  // Assign switch
  enum svp_storage_e store_type = 0;
  if (strcmp(dtype, "time") == 0) {
    // If we store time, ignore other parameters
    store_type = SVP_STORE_SIM_TIME;
  } else {
    // Check if we store sync or async data
    store_type = (is_async) ? SVP_STORE_ASYNC_DATA : SVP_STORE_SYNC_DATA;
  }
  // Switch based on data type
  if (strcmp(dtype, "char") == 0) {
    return svp_dstore_create(clsdat, name, store_type, 1, dims,
                             H5T_NATIVE_CHAR);
  } else if (strcmp(dtype, "uchar") == 0) {
    return svp_dstore_create(clsdat, name, store_type, 1, dims,
                             H5T_NATIVE_UCHAR);
  } else if (strcmp(dtype, "sint") == 0) {
    return svp_dstore_create(clsdat, name, store_type, 1, dims,
                             H5T_NATIVE_SHORT);
  } else if (strcmp(dtype, "usint") == 0) {
    return svp_dstore_create(clsdat, name, store_type, 1, dims,
                             H5T_NATIVE_USHORT);
  } else if (strcmp(dtype, "int") == 0) {
    return svp_dstore_create(clsdat, name, store_type, 1, dims,
                             H5T_NATIVE_INT);
  } else if (strcmp(dtype, "uint") == 0) {
    return svp_dstore_create(clsdat, name, store_type, 1, dims,
                             H5T_NATIVE_UINT);
  } else if (strcmp(dtype, "long") == 0) {
    return svp_dstore_create(clsdat, name, store_type, 1, dims,
                             H5T_NATIVE_LONG);
  } else if (strcmp(dtype, "ulong") == 0) {
    return svp_dstore_create(clsdat, name, store_type, 1, dims,
                             H5T_NATIVE_ULONG);
  } else if (strcmp(dtype, "double") == 0) {
    return svp_dstore_create(clsdat, name, store_type, 1, dims,
                             H5T_NATIVE_DOUBLE);
  } else if (strcmp(dtype, "time") == 0) {
    return svp_dstore_create(clsdat, name, store_type, 1, dims,
                             H5T_NATIVE_DOUBLE);
  } else {
    fprintf(stderr, "ERROR %s: Unknown dtype: %s\n", __func__, dtype);
  }
  // IF we reach here, something is wrong
  return NULL;
}  // svp_dstore_svcreate


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


void svp_dstore_svattr(struct svp_dstore_t *dat, char *name, char *value) {
  svp_add_attr(dat->dset, name, value);
}  // svp_dstore_svattr


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
  if (SVP_STORE_SIM_TIME != dat->store_type) {
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


inline int svp_dstore_write_int8(struct svp_dstore_t *dat, double simtime,
                                  const svOpenArrayHandle dbuf) {
  void *dptr = svGetArrayPtr(dbuf);
  return svp_dstore_write_data(dat, simtime, dptr);
}  // svp_dstore_write_int8


inline int svp_dstore_write_int16(struct svp_dstore_t *dat, double simtime,
                                  const svOpenArrayHandle dbuf) {
  void *dptr = svGetArrayPtr(dbuf);
  return svp_dstore_write_data(dat, simtime, dptr);
}  // svp_dstore_write_int16


inline int svp_dstore_write_int32(struct svp_dstore_t *dat, double simtime,
                                  const svOpenArrayHandle dbuf) {
  void *dptr = svGetArrayPtr(dbuf);
  return svp_dstore_write_data(dat, simtime, dptr);
}  // svp_dstore_write_int32


inline int svp_dstore_write_int64(struct svp_dstore_t *dat, double simtime,
                                  const svOpenArrayHandle dbuf) {
  void *dptr = svGetArrayPtr(dbuf);
  return svp_dstore_write_data(dat, simtime, dptr);
}  // svp_dstore_write_int64


inline int svp_dstore_write_float64(struct svp_dstore_t *dat, double simtime,
                                    const svOpenArrayHandle dbuf) {
  void *dptr = svGetArrayPtr(dbuf);
  return svp_dstore_write_data(dat, simtime, dptr);
}  // svp_dstore_write_float64
