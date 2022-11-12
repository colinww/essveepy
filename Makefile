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

CC_FLAGS = -fPIC


# Debug/optimization options
ifdef OPTIMIZE
#   Add debugging information
	ifeq ($(OPTIMIZE), g)
		DEBUG_FLAGS := -g -O0
		CC_FLAGS += -g -O0
	else
		DEBUG_FLAGS := -DNDEBUG
		CC_FLAGS += -O$(OPTIMIZE)
	endif
else
	DEBUG_FLAGS := -DNDEBUG
	CC_FLAGS += -O3
endif

.PHONY: all
all:
	h5cc $(CC_FLAGS) $(DEBUG_FLAGS) -c csrc/svp_dstore.c -o build/svp_dstore.o
	h5cc $(CC_FLAGS) $(DEBUG_FLAGS) -c csrc/svp_file.c -o build/svp_file.o

.PHONY: clean
clean:
	rm -f build/svp_dstore.o
	rm -f build/svp_file.o
