###############################################################################
#
# UCSD ISPG Group 2022
#
# Created on 12-Nov-22
# @author: Colin Weltin-Wu
#
# Description
# -----------
# Makefile for building the shared library.
#
# Version History
# ---------------
# 12-Nov-22: Initial version
# 13-Nov-22: Converted to a general Makefile format.
#
###############################################################################



###########################
# HDF5-specific source files
HDF5_CSRC := svp_hdf5_defs svp_dstore svp_file

##############################
# General library source files
SVP_CSRC := svp_noise

#################
# Build directory
BUILD := build

##################
# Source directory
SRC := csrc

##############
# SV libraries
SVLIB := svlib


# Extra compiler options
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

################################################################################
# Common
################################################################################

.PHONY: all
all: $(SVLIB)/libessveepy.so | $(SVLIB)

.PHONY: clean
clean:
	rm -rf $(BUILD)/*
	rm -f $(SVLIB)/libessveepy.so

$(BUILD):
	mkdir -p $@

$(SVLIB):
	mkdir -p $@

################################################################################
# Generic build rule
################################################################################

$(BUILD)/%.o: $(SRC)/%.c | $(BUILD)
	h5cc $(CC_FLAGS) $(DEBUG_FLAGS) -I$(AMSHOME)/tools/include $< -c -o $@

################################################################################
# HDF5-specific compiles
################################################################################

HDF5_OBJ := $(addsuffix .o,$(addprefix $(BUILD)/,$(HDF5_CSRC)))

################################################################################
# Library compiles
################################################################################

SVP_OBJ := $(addsuffix .o,$(addprefix $(BUILD)/,$(SVP_CSRC)))

################################################################################
# Shared library
################################################################################

$(SVLIB)/libessveepy.so: $(HDF5_OBJ) $(SVP_OBJ) | $(SVLIB)
	h5cc -shared $(HDF5_OBJ) $(SVP_OBJ) -o $@ -I$(AMSHOME)/tools/include
