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

#ifndef __SVP__FILE__H__
#define __SVP__FILE__H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hdf5.h"
#include "svp_hdf5_defs.h"


///////////////////////////////////////////////////////////////////////////////
// API
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Open a new HDF5 file for dumping simulation data.
 *
 * @param fname Full path to file to be created.
 * @return struct svp_hdf5_data* File handle for adding signals to the dump.
 *
 * This will overwrite any existing file with the given name.
 *
 */
struct svp_hdf5_data *svp_hdf5_fopen(const char *fname);


/**
 * @brief Add a signal to the file to be dumped.
 *
 * @param clsdat File handle to contain signals.
 * @param dat Data store object that will be added to the file.
 * @return int Returns 0 if successful.
 */
int svp_hdf5_addsig(struct svp_hdf5_data *clsdat, struct svp_dstore_t *dat);

/**
 * @brief Close the file and any registered data stores.
 *
 * @param clsdat File handle to be closed.
 * @return int Returns 0 if successful.
 *
 * After closing, none of the associated data stores can be used.
 */
int svp_hdf5_fclose(struct svp_hdf5_data *clsdat);


/**
 * @brief Add a string attribute to HDF5 data file.
 *
 * @param clsdat File handle previously created.
 * @param name Attribute name.
 * @param value Attribute value.
 */
void svp_hdf5_add_attribute(struct svp_hdf5_data *clsdat, char *name,
                            char *value);

#endif
