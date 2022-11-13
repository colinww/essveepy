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

def h5_tree(val, pre=''):
    items = len(val)
    for key, val in val.items():
        items -= 1
        if items == 0:
            # the last item
            if type(val) == h5py._hl.group.Group:
                print(pre + '└── ' + key)
                h5_tree(val, pre+'    ')
            else:
                print(pre + '└── ' + key + ' (%d)' % len(val))
        else:
            if type(val) == h5py._hl.group.Group:
                print(pre + '├── ' + key)
                h5_tree(val, pre+'│   ')
            else:
                print(pre + '├── ' + key + ' (%d)' % len(val))

with h5py.File('test_1_data.h5', 'r') as hf:
    print(hf)
    h5_tree(hf)
