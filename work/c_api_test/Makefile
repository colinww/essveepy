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
# Get directory of svlib
SVLIB := $(shell readlink -f ../../svlib)

.PHONY: all
all: prereq test_1 test_2

.PHONY: prereq
prereq:
	cd ../../ && make

.PHONY: test_1
test_1: prereq
	h5cc $(CC_FLAGS) -I$(AMSHOME)/tools/include -c test_1.c -o test_1.o
	h5cc test_1.o -o test_1.out -Wl,-rpath=$(SVLIB) -L$(SVLIB) -lessveepy

.PHONY: test_2
test_2: prereq
	h5cc $(CC_FLAGS) -I$(AMSHOME)/tools/include -c test_2.c -o test_2.o
	h5cc test_2.o -o test_2.out -Wl,-rpath=$(SVLIB) -L$(SVLIB) -lessveepy

.PHONY: clean
clean:
	cd ../../ && make clean
	rm -f test_1.o
	rm -f test_1.out
	rm -f test_2.o
	rm -f test_2.out
	rm -f *.h5
