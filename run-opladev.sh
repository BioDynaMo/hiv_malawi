#!/bin/bash

BDM_SCRIPT_DIR=$(readlink -e $(dirname "${BASH_SOURCE[0]}"))
source $BDM_SCRIPT_DIR/../../util/default-compile-script.sh ""
# source $BDM_SCRIPT_DIR/../../util/default-compile-script.sh "-DCMAKE_BUILD_TYPE=RelWithDebInfo" "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
# source $BDM_SCRIPT_DIR/../../util/default-compile-script.sh "-DCMAKE_BUILD_TYPE=Debug" "-DCMAKE_BUILD_TYPE=Debug"
cd $BDM_SCRIPT_DIR/build

./hiv_malawi

# gdb -x $BDM_SCRIPT_DIR/../../util/gdbscript ./hiv_malawi
# export OMP_NUM_THREADS=2
# taskset -c 0-1 gdb -x $BDM_SCRIPT_DIR/../../util/gdbscript ./hiv_malawi

# export OMP_NUM_THREADS=2
# # taskset -c 0,18 gdb ./hiv_malawi
# taskset -c 0,18 gdb -ex r ./hiv_malawi
# # taskset -c 0,1 ../../../../biodynamo/util/valgrind.sh ./hiv_malawi

# export OMP_NUM_THREADS=36
# /usr/bin/time -v taskset -c 0-17,72-89 ./hiv_malawi

# SIMULATION="hiv_malawi"
# CMD="./hiv_malawi"
# benchmarkIC $(CPUCount) \
#   "$RESULT_DIR/$SIMULATION/run" \
#   $CMD

