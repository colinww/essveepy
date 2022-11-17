###############################################################################
#
# UCSD ISPG Group 2022
#
# Created on 19-Nov-22
# @author: colinww
#
# Description
# -----------
# Test loading an HDF5 simulation dump.
#
# Version History
# ---------------
# 19-Nov-22: Initial version
#
###############################################################################

import python.simdump as simdump

FILE = '../dump_api_test/sv_data_dump.h5'


# Load the file from simulation
obj = simdump.SimDump(FILE)
obj.summary()
