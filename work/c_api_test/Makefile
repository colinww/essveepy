###############################################################################
#
# UCSD ISPG Group 2022
#
# Created on 12-Nov-22
# @author: Colin Weltin-Wu
#
# Description
# -----------
# Makefile for building the HDF5 file interface wrappers.
#
# Version History
# ---------------
# 12-Nov-22: Initial version
#
###############################################################################

CC_FLAGS = -g -O3


.PHONY: all
all:
	cd ../../ && make
	h5cc $(CC_FLAGS) -c test_1.c -o test_1.o
	h5cc ../../build/svp_dstore.o ../../build/svp_file.o test_1.o -o test_1.out

.PHONY: clean
clean:
	cd ../../ && make clean
	rm -f test_1.o
	rm -f test_1.out
