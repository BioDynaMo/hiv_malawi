#!/bin/bash

BDM_SCRIPT_DIR=$(readlink -e $(dirname "${BASH_SOURCE[0]}"))
source $BDM_SCRIPT_DIR/../../benchmark/vtune.sh
run ./hiv_malawi

