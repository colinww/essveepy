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
	make prereq
	make test_1
	make test_2

.PHONY: prereq
prereq:
	cd ../../ && make

.PHONY: test_1
test_1: prereq
	h5cc $(CC_FLAGS) -c test_1.c -o test_1.o
	h5cc ../../build/svp_dstore.o ../../build/svp_file.o test_1.o -o test_1.out

.PHONY: test_2
test_2: prereq
	h5cc $(CC_FLAGS) -c test_2.c -o test_2.o
	h5cc ../../build/svp_dstore.o ../../build/svp_file.o test_2.o -o test_2.out

.PHONY: clean
clean:
	cd ../../ && make clean
	rm -f test_1.o
	rm -f test_1.out
	rm -f test_2.o
	rm -f test_2.out
