###############################################################################
#
# UCSD ISPG Group 2022
#
# Created on 19-Nov-22
# @author: colinww
#
# Description
# -----------
# Load the HDF5 data file as a Python-friendly structure.
#
# Version History
# ---------------
# 19-Nov-22: Initial version
#
###############################################################################

import os
import psutil
import h5py
from dataclasses import dataclass

###########
# Constants
###########

BIT_COLOR = '\x1b[1;31m'
INTEGER_COLOR = '\x1b[1;32m'
REAL_COLOR = '\x1b[1;33m'
TIME_COLOR = '\x1b[1;34m'

################################
# Internal classes and functions
################################

@dataclass
class _DumpGroup: pass

@dataclass
class _DumpData: pass

@dataclass
class _DumpInfo:
    name : None
    storage : None
    svtype : None
    shape : None
    dtype : None


def _parsedata(name, dobj):
    """Process attributes and information about the dataset contents.
    """
    obj = _DumpData()
    info = _DumpInfo(name, dobj.attrs['storage'].decode('ascii'),
                     dobj.attrs['svtype'].decode('ascii'),
                     None, None)
    # Construct the members
    if ('time' == info.storage):
        # This is time, add ns and rem
        setattr(obj, 'ns', dobj['ns'])
        setattr(obj, 'rem', dobj['rem'])
        info.shape = dobj.shape
        info.dtype = dobj.dtype
        return obj, info
    elif ('async' == info.storage):
        # Split into time and data
        setattr(obj, 'time', dobj['time'])
        setattr(obj, 'data', dobj['data'])
        info.shape = dobj['data'].shape
        info.dtype = dobj['data'].dtype
        return obj, info
    else:
        # This is synchronous data, drop final hierarchy level
        info.shape = dobj['data'].shape
        info.dtype = dobj['data'].dtype
        return dobj['data'], info


def _treewalk(grp):
    """Recursively walk the HDF5 file group structure and collect information.
    """
    node_obj = _DumpGroup()
    node_info = _DumpGroup()
    for k, v in grp.items():
        if (h5py.Group == type(v)):
            # Add this name as an attribute to the current group, and recurse
            obj_data, obj_info = _treewalk(v)
            setattr(node_obj, k, obj_data)
            setattr(node_info, k, obj_info)
        elif (h5py.Dataset == type(v)):
            # Assign the member to the data structure itself
            obj_data, obj_info = _parsedata(k, v)
            setattr(node_obj, k, obj_data)
            setattr(node_info, k, obj_info)
        else:
            # This is an unrecognized group member
            print("Member {} is unknown type: {}".format(k, type(v)))
    return node_obj, node_info


def _fmt_data(obj):
    """Format information about the dataset.
    """
    # First format the SV data tyoe
    if ('bit' == obj.svtype):
        defstr = "{}{}\x1b[0m   ".format(BIT_COLOR, obj.name)
    elif ('integer' == obj.svtype):
        defstr = "{}{}\x1b[0m   ".format(INTEGER_COLOR, obj.name)
    elif ('real' == obj.svtype):
        defstr = "{}{}\x1b[0m   ".format(REAL_COLOR, obj.name)
    elif ('time' == obj.svtype):
        defstr = "{}{}\x1b[0m   ".format(TIME_COLOR, obj.name)
    # Now add the storage type
    if ('time' == obj.storage):
        defstr += "{}:(time)".format(obj.shape)
    elif ('async' == obj.storage):
        defstr += "{}:(async {})".format(obj.shape, obj.dtype)
    else:
        defstr += "{}:(sync {})".format(obj.shape, obj.dtype)
    return defstr


def _treeprint(node, pre=''):
    """Pretty-print the structure and information of the data.
    """
    items = [x for x in dir(node) if not callable(getattr(node, x)) and
             not x.startswith("__")]
    num_items = len(items)
    for key in items:
        val = getattr(node, key)
        num_items -= 1
        if num_items == 0:
            # the last item, change the hierarchy strings
            idt_str = '└── '
            ext_str = '    '
            term_str = '└── '
        else:
            idt_str = '├── '
            ext_str = '│   '
            term_str  = '├── '
        # Check the type of the current node
        if (_DumpGroup == type(val)):
            # This is a group so recurse
            print(pre + idt_str + key)
            _treeprint(val, pre + ext_str)
        else:
            # This is a leaf node, so print information about the data
            print(pre + term_str + _fmt_data(val))


############
# Main class
############

class SimDump:
    """Provide a structured view of the HDF5 data dump from SV simulation.
    """

    def __init__(self, fname, cache_size=32):
        """Create a new file view.

        Parameters
        ----------
        fname : str
            File to be opened.
        cache_size : int, optional
            Per-file HDF5 cache in MB. GENERALLY DO NOT TOUCH.
            The default is 32.

        Returns
        -------
        None.

        """
        # Save file name
        self.fname = os.path.realpath(fname)
        # First limit the cache size by available system memory
        ram_mb = psutil.virtual_memory().total / 1024.**3
        # TODO
        # Set cache size based on input and system limits
        cache_bytes = cache_size * 1024 * 1024
        # Open the file
        self.fp = h5py.File(fname, rdcc_nbytes=cache_bytes)
        # Construct the object members by traversing the tree
        self.data, self.info = _treewalk(self.fp)

    def close(self):
        """Explicitly close the file.
        """
        self.fp.close()

    def summary(self):
        """Print a summary of the file contents.
        """
        print("Legend")
        print("======")
        print("{}Bit\x1b[0m".format(BIT_COLOR))
        print("{}Integer\x1b[0m".format(INTEGER_COLOR))
        print("{}Real\x1b[0m".format(REAL_COLOR))
        print("{}Time\x1b[0m".format(TIME_COLOR))
        print("======")
        print(self.fname)
        _treeprint(self.info)
