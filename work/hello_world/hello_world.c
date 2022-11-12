///////////////////////////////////////////////////////////////////////////////
//
// UCSD ISPG Group 2022
//
// Created on 05-Nov-22
// @author: Colin Weltin-Wu
//
// Description
// -----------
// This test is based on hdf5/examples/h5_extend.c
//
// The code has been grouped into sections to emulate C function calls to an
// API that is exposed via DPI to SystemVerilog.
//
//
// Version History
// ---------------
// 05-Nov-22: Initial version
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hdf5.h"


struct clsvars {
  hid_t file;
};

struct hdf5_defs {
  hid_t dspace;
  hid_t dtype;
  hid_t dset;
  hid_t time_mid;
  hid_t data_mid;
};


herr_t add_file_attr(hid_t fileID, char *name, char *value) {
  // Access the root group
  hid_t root_group = H5Gopen(fileID, "/", H5P_DEFAULT);

  // Create string type of correct size for the attribute
  hid_t attr_type = H5Tcopy(H5T_C_S1);
  herr_t status = H5Tset_size(attr_type, strlen(value) + 2);
  if (status < 0) return status;

  // Create the data space for the attribute.
  hid_t attr_dspace = H5Screate(H5S_SCALAR);
  if(attr_dspace < 0) return attr_dspace;
  // Create a dataset attribute.
  hid_t attr_id = H5Acreate(root_group, name, attr_type, attr_dspace,
                            H5P_DEFAULT, H5P_DEFAULT);
  if(attr_id < 0) return attr_id;
  // Write the attribute data
  status = H5Awrite(attr_id, attr_type, value);
  if(status < 0) return status;

  // Close this file property
  status = H5Aclose(attr_id);
  if(status < 0) return status;
  status = H5Sclose(attr_dspace);
  if(status < 0) return status;
  return 0;
}  // add_file_attr


int main(void) {
  // Code body of initialization function
  // Input variables passed in
#define FILENAME    "extend.h5"

  // Create an instance of the data structure
  struct clsvars *dptr = malloc(sizeof(struct clsvars));

  // Create a new file. If file exists its contents will be overwritten
  dptr->file = H5Fcreate(FILENAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  // Add some metadata to the file
  add_file_attr(dptr->file, "property0", "pval");
  add_file_attr(dptr->file, "property1", "another_value");
  // Here we pretend that file initialization has completed
  fprintf(stderr, "File has been opened\n");


  // Now let's add a 2x3 array of int64 values
  struct hdf5_defs arr_data;
  hsize_t dims[3] = {10, 2, 3};
  hsize_t maxdims[3] = {H5S_UNLIMITED, 2, 3};
  hsize_t chunk_dims[3] = {1000, 2, 3};
  // Create the data space with unlimited first dimension
  arr_data.dspace = H5Screate_simple(3, dims, maxdims);
  arr_data.dtype = H5T_NATIVE_LLONG;
  // Create a new chunked dataset
  hid_t prop;
  herr_t status;
  prop = H5Pcreate(H5P_DATASET_CREATE);
  status = H5Pset_chunk(prop, 3, chunk_dims);
  arr_data.dset = H5Dcreate2(dptr->file, "int_array", arr_data.dtype,
                             arr_data.dspace, H5P_DEFAULT, prop, H5P_DEFAULT);
  status = H5Pclose(prop);
  // This output has been added
  fprintf(stderr, "Added 2x3 array of integers\n");


  // Now add a struct of (double, int64[2])
  // First construct the array datatype
  struct hdf5_defs cpd_data;
  hsize_t cpd_adims[1] = {2};
  hid_t cpd_dat_tid = H5Tarray_create2(H5T_NATIVE_LLONG, 1, cpd_adims);
  // Now construct the compound
  hsize_t cpd_size = sizeof(H5T_NATIVE_DOUBLE) + H5Tget_size(cpd_dat_tid);
  // Create the compound datatype
  cpd_data.dtype = H5Tcreate(H5T_COMPOUND, cpd_size);
  // Insert members
  status = H5Tinsert(cpd_data.dtype, "time", 0, H5T_NATIVE_DOUBLE);
  status =
      H5Tinsert(cpd_data.dtype, "data", sizeof(H5T_NATIVE_DOUBLE), cpd_dat_tid);
  // Now create the two memory IDs for the time and data to access separately
  cpd_data.time_mid = H5Tcreate(H5T_COMPOUND, sizeof(H5T_NATIVE_DOUBLE));
  status = H5Tinsert(cpd_data.time_mid, "time", 0, H5T_NATIVE_DOUBLE);
  cpd_data.data_mid = H5Tcreate(H5T_COMPOUND, H5Tget_size(cpd_dat_tid));
  H5Tinsert(cpd_data.data_mid, "data", 0, cpd_dat_tid);
  // Now create a dataspace and dataset with this data
  hsize_t cpd_dims[1] = {10};
  hsize_t cpd_maxdims[1] = {H5S_UNLIMITED};
  hsize_t cpd_chunk_dims[1] = {1000};
  cpd_data.dspace = H5Screate_simple(1, cpd_dims, cpd_maxdims);
  prop = H5Pcreate(H5P_DATASET_CREATE);
  status = H5Pset_chunk(prop, 1, cpd_chunk_dims);
  cpd_data.dset = H5Dcreate2(dptr->file, "async", cpd_data.dtype,
                             cpd_data.dspace, H5P_DEFAULT, prop, H5P_DEFAULT);
  status = H5Pclose(prop);
  fprintf(stderr, "Added compound datatype\n");


  int ii, jj, kk;
  hid_t subspace;
  hid_t memspace;

  // Now add data to the integer array
  // Extend this dataset
  subspace = H5Dget_space(arr_data.dset);
  hsize_t cdims[3];
  H5Sget_simple_extent_dims(subspace, cdims, NULL);
  cdims[0] += 100;
  H5Dset_extent(arr_data.dset, cdims);
  H5Sclose(subspace);
  subspace = H5Dget_space(arr_data.dset);
  // Create a data set
  long long dwrite[100][2][3] = {{}};
  // Fill in the data
  for (ii = 0; 100 > ii; ++ii)
    for (jj = 0; 2 > jj; ++jj)
      for (kk = 0; 3 > kk; ++kk)
        dwrite[ii][jj][kk] = 300 + 6 * ii + 3 * jj + kk;
  // Select the hyperslab
  hsize_t offset[3] = {10, 0, 0};
  hsize_t wrdims[3] = {100, 2, 3};
  status = H5Sselect_hyperslab(subspace, H5S_SELECT_SET, offset, NULL,
                               wrdims, NULL);
  memspace = H5Screate_simple(3, wrdims, NULL);
  // Write the data
  status = H5Dwrite(arr_data.dset, H5T_NATIVE_LLONG, memspace, subspace,
                    H5P_DEFAULT, dwrite);
  status = H5Sclose(memspace);
  status = H5Sclose(subspace);
  fprintf(stderr, "Wrote array datatype\n");

  // Add data to the compound datatype
  // Create a dataset
  double tdata[4] = {10.0, 10.1, 10.2, 10.3};
  long long adata[4][2] = {{100, 101}, {102, 103}, {104, 105}, {106, 107}};
  // Fill in the data
  subspace = H5Dget_space(cpd_data.dset);
  hsize_t cpofst[1] = {0};
  hsize_t cpdims[1] = {1};
  memspace = H5Screate_simple(1, cpdims, NULL);
  hid_t xfer_id = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_preserve(xfer_id, 1);
  for (ii = 0; 4 > ii; ++ii) {
    cpofst[0] = ii;
    status = H5Sselect_hyperslab(subspace, H5S_SELECT_SET, cpofst, NULL,
                                 cpdims, NULL);
    status = H5Dwrite(cpd_data.dset, cpd_data.time_mid, memspace, subspace,
                      xfer_id, &tdata[ii]);
    status = H5Dwrite(cpd_data.dset, cpd_data.data_mid, memspace, subspace,
                      xfer_id, &adata[ii]);
  }
  status = H5Sclose(memspace);
  status = H5Sclose(subspace);
  status = H5Pclose(xfer_id);
  fprintf(stderr, "Wrote compound datatype\n");


  /* Close resources */
  status = H5Dclose(arr_data.dset);
  status = H5Sclose(arr_data.dspace);
  // status = H5Tclose(arr_data.dtype);

  status = H5Dclose(cpd_data.dset);
  status = H5Sclose(cpd_data.dspace);
  status = H5Tclose(cpd_data.dtype);
  status = H5Tclose(cpd_data.time_mid);
  status = H5Tclose(cpd_data.data_mid);

  status = H5Fclose(dptr->file);
  fprintf(stderr, "Data closed\n");
}
