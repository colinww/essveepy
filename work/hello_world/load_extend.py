###############################################################################
#
# UCSD ISPG Group 2022
#
# Created on 12-Nov-22
# @author: colinww
#
# Description
# -----------
# Read a trivial HDF5 file.
#
# Version History
# ---------------
# 12-Nov-22: Initial version
#
###############################################################################

import h5py

fp = h5py.File('extend.h5', libver='latest')
print(fp['int_array'][10])
print(fp['int_array'][11])

print(fp['async']['time'])
print(fp['async']['data'])
