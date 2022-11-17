#!/usr/bin/tcsh -f
###############################################################################
#
# UCSD ISPG Group 2022
#
# Created on 05-Nov-22
# @author: colinww
#
# Description
# -----------
# Configure environment for HD5-DPI development.
#
# Version History
# ---------------
# 05-Nov-22: Initial version
#
###############################################################################


# Python
setenv CONDAHOME /tools/conda/2022.05
source ${CONDAHOME}/etc/profile.d/conda.csh
setenv PYTHONPATH ${CONDAHOME}/bin:${PYTHONPATH}
conda activate

# Xcelium setup
setenv AMSHOME /tools/cadence/XCELIUM2109
setenv PATH ${AMSHOME}/tools/bin:${PATH}
setenv PATH ${AMSHOME}/tools/cdsgcc/gcc/bin:${PATH}

# Cadence license file servers
setenv CDS_LIC_FILE "1704@acms-flexlm-lnx4.ucsd.edu,1704@acms-flexlm-lnx5.ucsd.edu,1704@acms-flexlm-lnx6.ucsd.edu"

# HDF5 compiler setup
setenv HDF5_CC ${AMSHOME}/tools/cdsgcc/gcc/bin/gcc
setenv HDF5_CLINKER ${AMSHOME}/tools/cdsgcc/gcc/bin/gcc

# Add current dir to python path
setenv PYTHONPATH `pwd`:${PYTHONPATH}
