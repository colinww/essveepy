# Makefile for iir_lpf
# Compiler arguments
COMPILE_ARGS := -64bit -elaborate -sv -access rwc -ALLOWREDEFINITION
COMPILE_ARGS += -disable_sem2009 -64bit -l xrun_compile.log -uvmhome CDNS-1.2
COMPILE_ARGS += -TIMESCALE 1ns/1fs
# Simulation arguments
XRUN_ARGS := -R -l xrun_run.log

# Compile and run the test
.PHONY: test
test:
	xrun $(COMPILE_ARGS) submod.sv top.sv
	xrun $(XRUN_ARGS) -input shm.tcl

# Clean simulation artifacts
.PHONY: clean
clean:
	rm -rf xcelium.d
	rm -rf waves.shm
	rm -f xrun_compile.history
	rm -f xrun_compile.log
	rm -f xrun.history
	rm -f xrun.log
	rm -f xrun.key
	rm -f xrun_run.log
