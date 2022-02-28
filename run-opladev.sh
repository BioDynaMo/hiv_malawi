#!/bin/bash

BDM_SCRIPT_DIR=$(readlink -e $(dirname "${BASH_SOURCE[0]}"))
source $BDM_SCRIPT_DIR/../../util/default-compile-script.sh ""
cd $BDM_SCRIPT_DIR/build

export OMP_NUM_THREADS=16
taskset -c 0-15 ./hiv_malawi

# SIMULATION="hiv_malawi"
# CMD="./hiv_malawi"
# benchmarkIC $(CPUCount) \
#   "$RESULT_DIR/$SIMULATION/run" \
#   $CMD

