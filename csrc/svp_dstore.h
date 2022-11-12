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
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hdf5.h"

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

#define MAX_RANK 8
#define CHUNK_SIZE 8192

///////////////////////////////////////////////////////////////////////////////
// Data structures
///////////////////////////////////////////////////////////////////////////////

struct svp_dstore_t {
  const char *name;         /// Name of simulation variable being stored
  hid_t dspc;
  hid_t dtyp;
  hid_t dset;
  // Memory views for accessing time and data separately
  hid_t xfer_id;            /// Transfer ID
  hid_t mspc;               /// Memory space
  hid_t t_mid;              /// Time memory view
  hid_t d_mid;              /// Array data memory view
  // Write tracking
  unsigned long wptr;       /// Total number of elements written
  unsigned long size;       /// Current size of file buffer
  // Description of the actual data being stored
  hid_t h5type;             /// Raw atomic datatype
  int rank;                 /// Number of dimensions of each data element
  hsize_t dims[MAX_RANK];   /// Rank-size list of individual array dimensions
};

///////////////////////////////////////////////////////////////////////////////
// API
///////////////////////////////////////////////////////////////////////////////

struct svp_dstore_t *svp_create_dstore(hid_t fid, const char *name,
                                       int is_async, int rank, const int *dims,
                                       hid_t raw_type);

void svp_close_dstore(struct svp_dstore_t *dat);

void svp_write_data(struct svp_dstore_t *dat, double simtime, const void *buf);

void svp_write_char(struct svp_dstore_t *dat, double simtime, long *buf);

void svp_write_long(struct svp_dstore_t *dat, double simtime, long *buf);

void svp_write_double(struct svp_dstore_t *dat, double simtime, double *buf);

void svp_write_time(struct svp_dstore_t *dat, long int_ns, double frac);
