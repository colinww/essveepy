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

#include "svp_hdf5_defs.h"

void svp_group_hierarchy_split(hid_t fid, const char *full_name, hid_t *gid,
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
}  // svp_group_hierarchy_split


herr_t svp_add_attr(hid_t obj_id, char *name, char *value) {
  // Create string type of correct size for the attribute
  hid_t attr_type = H5Tcopy(H5T_C_S1);
  herr_t status = H5Tset_size(attr_type, strlen(value) + 2);
  if (status < 0) return status;

  // Create the data space for the attribute.
  hid_t attr_dspace = H5Screate(H5S_SCALAR);
  if(attr_dspace < 0) return attr_dspace;
  // Create a dataset attribute.
  hid_t attr_id = H5Acreate(obj_id, name, attr_type, attr_dspace,
                            H5P_DEFAULT, H5P_DEFAULT);
  if(attr_id < 0) return attr_id;
  // Write the attribute data
  status = H5Awrite(attr_id, attr_type, value);
  if(status < 0) return status;

  // Close allocated objects
  H5Aclose(attr_id);
  H5Sclose(attr_dspace);
  H5Tclose(attr_type);
  return 0;
}  // svp_add_attr
