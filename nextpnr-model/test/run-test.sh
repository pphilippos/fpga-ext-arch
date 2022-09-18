#!/usr/bin/env bash
set -e
TEST_NAME=$1
if [ -z "$TEST_NAME" ]; then
	echo "usage: run-test.sh <testname>"
	exit 2
fi
# Synthesise test
yosys -p "hierarchy -top top; tcl ../synth/synth.tcl; write_json ${TEST_NAME}.json" ../instr/${TEST_NAME}.v
# Run nextpnr to build arch and bitstream
LD_PRELOAD=../arch_gen.so nextpnr-generic --uarch futurefpga --json ${TEST_NAME}.json --router router2 --placer-heap-beta 0.5
# Run iverilog simulation
iverilog -o ${TEST_NAME}.vvp futurefpga_arch.v ../arch_blocks/rmux.v ../arch_blocks/slice.v  tb_${TEST_NAME}.v
vvp ${TEST_NAME}.vvp