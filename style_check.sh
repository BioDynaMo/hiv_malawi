#!/bin/bash
## -----------------------------------------------------------------------------
##
## Copyright (C) 2022 CERN & University of Geneva for the benefit of the
## BioDynaMo collaboration. All Rights Reserved.
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
##
## See the LICENSE file distributed with this work for details.
## See the NOTICE file distributed with this work for additional information
## regarding copyright ownership.
##
## Authored by: Tobias Duswald, CERN, TUM
##
## -----------------------------------------------------------------------------

# Define directories
SRCDIR="$(pwd)/src"

# Return on failure
set -e

# Run clang format on files
for f in $(find $SRCDIR -name "*.cc" -or -name "*.h"); do 
  echo "Working on $f"
  clang-format --dry-run --Werror $f
done
