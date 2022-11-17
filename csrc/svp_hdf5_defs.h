///////////////////////////////////////////////////////////////////////////////
//
// UCSD ISPG Group 2022
//
// Created on 12-Nov-22
// @author: Colin Weltin-Wu
//
// Description
// -----------
// Common HDF5-related data structures and definitions.
//
// Version History
// ---------------
// 12-Nov-22: Initial version
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __SVP__HDF5__DEFS__H__
#define __SVP__HDF5__DEFS__H__

#include "hdf5.h"

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

/// Maximum number of signals per file
#define MAX_SIGNALS 1024
/// Maximum flattened size of each data record (product of all dimensions)
#define MAX_FLAT_SIZE 2048
/// Size of each page in the HD5 file and corresponding cache
#define CHUNK_SIZE 8192

/**
 * @brief Enumeration of the different types of data that can be stored.
 *
 */
enum svp_storage_e {
  SVP_STORE_SYNC_DATA,
  SVP_STORE_ASYNC_DATA,
  SVP_STORE_SIM_TIME
};

///////////////////////////////////////////////////////////////////////////////
// Data structures
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief High-resolution timestamp.
 *
 * Time is stored as an integral number of nanoseconds, plus a floating-point
 * remainder in [0, 1).
 *
 */
struct svp_sim_time_t {
  long ns;
  double rem;
};


/**
 * @brief State information for a data destination in the HD5 file.
 *
 */
struct svp_dstore_t {
  const char *name;         ///< Name of simulation variable being stored
  enum svp_storage_e store_type; ///< Storage type of dstore
  hid_t dspc;               ///< Dataspace handle
  hid_t dtyp;               ///< Datatype (compound) handle
  hid_t dset;               ///< Dataset handle
  // Memory views for accessing time and data separately
  hid_t xfer_id;            ///< Transfer ID
  hid_t t_mid;              ///< Time memory view
  hid_t d_mid;              ///< Array data memory view
  // Write tracking
  unsigned long wptr;       ///< Total number of elements written
  unsigned long size;       ///< Current size of file buffer
  // Description of the actual data being stored
  hid_t h5type;             ///< Raw atomic datatype
  int rank;                 ///< Number of dimensions of each data element
  hsize_t *dims;            ///< Rank-size list of individual array dimensions
  // Cache data
  hssize_t cstride;         ///< Cache data stride (bytes)
  unsigned long cptr;       ///< Cache pointer
  double *tcache;           ///< Timestamp cache
  void *dcache;             ///< Data cache
};


struct svp_hdf5_data {
  const char *name;
  hid_t fptr;
  int num_signals;
  struct svp_dstore_t **dptr;
};  // svp_hdf5_data

#endif
