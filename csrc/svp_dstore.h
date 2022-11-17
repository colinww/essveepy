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

#ifndef __SVP__DSTORE__H__
#define __SVP__DSTORE__H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hdf5.h"
#include "svdpi.h"
#include "svp_hdf5_defs.h"

///////////////////////////////////////////////////////////////////////////////
// API
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Create a new data storage in an existing file of specified type.
 *
 * @param clsdat Data structure containing HDF5 file pointer.
 * @param name Fully-qualified signal name.
 * @param store_type Specify the type of signal to be stored.
 * @param rank Number of dimensions.
 * @param dims Size of each dimension.
 * @param raw_type Underlying HD5 atomic datatype.
 * @return struct svp_dstore_t* Data store object for future writing.
 */
struct svp_dstore_t *svp_dstore_create(struct svp_hdf5_data *clsdat,
                                       const char *name,
                                       enum svp_storage_e store_type, int rank,
                                       const int *dims, hid_t raw_type);


/**
 * @brief Wrapper for easier calling via DPI.
 *
 * @param clsdat Data structure containing HDF5 file pointer.
 * @param name Fully-qualified signal name.
 * @param is_async Stores timestamp along with data.
 * @param width Assumes single-dimensional arrays.
 * @param dtype String version of type.
 * @return struct svp_dstore_t* Data store object for future writing.
 *
 * Current datatypes supported: char, uchar, sint, usint, int, uint, long,
 * ulong, double, time.
 */
struct svp_dstore_t *svp_dstore_svcreate(struct svp_hdf5_data *clsdat,
                                         const char *name, int is_async,
                                         int width, const char *dtype);


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
 * @brief Add a string attribute to HDF5 dataset.
 *
 * @param dat Datastore object.
 * @param name Attribute name.
 * @param value Attribute value.
 */
void svp_dstore_svattr(struct svp_dstore_t *dat, char *name, char *value);


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
 * @param dat Data store object initialized with SVP_STORE_SIM_TIME.
 * @param simtime Timestamp to be written.
 * @return int Returns 0 if successful.
 */
int svp_dstore_write_time(struct svp_dstore_t *dat,
                          struct svp_sim_time_t simtime);


/**
 * @brief Explicit typed call for DPI interface.
 *
 * @param dat Data store object for the matching data type to be written.
 * @param simtime (optional) For asynchronous data storage, the simtime must
 * also be provided that is written alongside the data.
 * @param dbuf Alias of void*, dereferenced with svGetArrayPtr.
 * @return int 0 if successful.
 */
int svp_dstore_write_int8(struct svp_dstore_t *dat, double simtime,
                          const svOpenArrayHandle dbuf);


/**
 * @brief Explicit typed call for DPI interface.
 *
 * @param dat Data store object for the matching data type to be written.
 * @param simtime (optional) For asynchronous data storage, the simtime must
 * also be provided that is written alongside the data.
 * @param dbuf Alias of void*, dereferenced with svGetArrayPtr.
 * @return int 0 if successful.
 */
int svp_dstore_write_int16(struct svp_dstore_t *dat, double simtime,
                           const svOpenArrayHandle dbuf);


/**
 * @brief Explicit typed call for DPI interface.
 *
 * @param dat Data store object for the matching data type to be written.
 * @param simtime (optional) For asynchronous data storage, the simtime must
 * also be provided that is written alongside the data.
 * @param dbuf Alias of void*, dereferenced with svGetArrayPtr.
 * @return int 0 if successful.
 */
int svp_dstore_write_int32(struct svp_dstore_t *dat, double simtime,
                           const svOpenArrayHandle dbuf);


/**
 * @brief Explicit typed call for DPI interface.
 *
 * @param dat Data store object for the matching data type to be written.
 * @param simtime (optional) For asynchronous data storage, the simtime must
 * also be provided that is written alongside the data.
 * @param dbuf Alias of void*, dereferenced with svGetArrayPtr.
 * @return int 0 if successful.
 */
int svp_dstore_write_int64(struct svp_dstore_t *dat, double simtime,
                           const svOpenArrayHandle dbuf);


/**
 * @brief Explicit typed call for DPI interface.
 *
 * @param dat Data store object for the matching data type to be written.
 * @param simtime (optional) For asynchronous data storage, the simtime must
 * also be provided that is written alongside the data.
 * @param dbuf Alias of void*, dereferenced with svGetArrayPtr.
 * @return int 0 if successful.
 */
int svp_dstore_write_float64(struct svp_dstore_t *dat, double simtime,
                             const svOpenArrayHandle dbuf);

#endif
