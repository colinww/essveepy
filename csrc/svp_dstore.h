///////////////////////////////////////////////////////////////////////////////
//
// UCSD ISPG Group 2022
//
// Created on 12-Nov-22
// @author: Colin Weltin-Wu
//
// Description
// -----------
// The data structure and API for interacting with a dataset.
//
// Version History
// ---------------
// 12-Nov-22: Initial version
// 13-Nov-22: Added time datatype, removed max dimensions limit.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hdf5.h"

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

/// Maximum flattened size of each data record (product of all dimensions)
#define MAX_FLAT_SIZE 2048
/// Size of each page in the HD5 file and corresponding cache
#define CHUNK_SIZE 8192

/**
 * @brief Enumeration of the different types of data that can be stored.
 *
 */
enum svp_storage_e {
  SVP_SYNC_DATA,
  SVP_ASYNC_DATA,
  SVP_SIM_TIME
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

///////////////////////////////////////////////////////////////////////////////
// API
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Create a new data storage in an existing file of specified type.
 *
 * @param fid File pointer already opened.
 * @param name Fully-qualified signal name.
 * @param store_type Specify the type of signal to be stored.
 * @param rank Number of dimensions.
 * @param dims Size of each dimension.
 * @param raw_type Underlying HD5 atomic datatype.
 * @return struct svp_dstore_t* Data store object for future writing.
 */
struct svp_dstore_t *svp_dstore_create(hid_t fid, const char *name,
                                       enum svp_storage_e store_type, int rank,
                                       const int *dims, hid_t raw_type);


/**
 * @brief Close data storage once writing is done.
 *
 * @param dat Data store to be closed.
 *
 * Once this function exits, the \p dat object is freed and can no longer be
 * used.
 */
void svp_dstore_close(struct svp_dstore_t *dat);


/**
 * @brief Write a data point to the data storage.
 *
 * @param dat Data store object for the matching data type to be written.
 * @param simtime (optional) For asynchronous data storage, the simtime must
 * also be provided that is written alongside the data.
 * @param buf Data to be written. Must be cast to void pointer first. No
 * type or dimension checking is done on this data. Array data is expected to
 * be laid out in C-contiguous order.
 * @return int 0 if successful.
 */
int svp_dstore_write_data(struct svp_dstore_t *dat, double simtime,
                          const void *buf);


/**
 * @brief Write specifically the high resolution time data.
 *
 * @param dat Data store object initialized with SVP_SIM_TIME.
 * @param simtime Timestamp to be written.
 * @return int Returns 0 if successful.
 */
int svp_dstore_write_time(struct svp_dstore_t *dat,
                          struct svp_sim_time_t simtime);


/**
 * @brief Wrapper for writing scalar long elements (easier to call from DPI).
 *
 * @param dat Data store object initialized with SVP_SIM_TIME.
 * @param simtime Timestamp to be written.
 * @param dwrite Scalar data to be written.
 * @return int Returns 0 if successful.
 */
inline int svp_dstore_write_long(struct svp_dstore_t *dat, double simtime,
                                 long dwrite) {
  return svp_dstore_write_data(dat, simtime, &dwrite);
};


/**
 * @brief Wrapper for writing scalar long elements (easier to call from DPI).
 *
 * @param dat Data store object initialized with SVP_SIM_TIME.
 * @param simtime Timestamp to be written.
 * @param dwrite Scalar data to be written.
 * @return int Returns 0 if successful.
 */
inline int svp_dstore_write_double(struct svp_dstore_t *dat, double simtime,
                                   double dwrite) {
  return svp_dstore_write_data(dat, simtime, &dwrite);
};
