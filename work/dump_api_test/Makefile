###############################################################################
#
# UCSD ISPG Group 2022
#
# Created on 05-Nov-22
# @author: Colin Weltin-Wu
#
# Description
# -----------
# Testing all the permutations of the dumping tools.
#
# Version History
# ---------------
# 05-Nov-22: Initial version
#
###############################################################################

# Compiler arguments
COMPILE_ARGS := -64bit -elaborate -sv -access rwc -ALLOWREDEFINITION
COMPILE_ARGS += -disable_sem2009 -64bit -l xrun_compile.log -uvmhome CDNS-1.2
COMPILE_ARGS += -TIMESCALE 1ns/1fs
# Simulation arguments
XRUN_ARGS := -64bit -R -l xrun_run.log

# Compile and run the test
.PHONY: all
all: compile run

.PHONY: compile
compile:
	cd ../../ && make
	xrun $(COMPILE_ARGS) ../../svinc/svp_pkg.sv top.sv

.PHONY: run
run:
	xrun $(XRUN_ARGS) -sv_lib ../../svlib/libessveepy.so -input shm.tcl

# Clean simulation artifacts
.PHONY: clean
clean:
	cd ../../ && make clean
	rm -rf xcelium.d
	rm -rf waves.shm
	rm -f xrun_compile.history
	rm -f xrun_compile.log
	rm -f xrun.history
	rm -f xrun.log
	rm -f xrun.key
	rm -f xrun_run.log
	rm -f *.h5
