## -----------------------------------------------------------------------------
##
## Copyright (C) 2021 CERN and the University of Geneva for the benefit of the
## BioDynaMo collaboration. All Rights Reserved.
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
##
## See the LICENSE file distributed with this work for details.
##
## -----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.2.0)

project(hiv_malawi)

find_package(BioDynaMo REQUIRED)
include(${BDM_USE_FILE})
include_directories("src")

file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE SOURCES src/*.cc)

bdm_add_executable(hiv_malawi
                   HEADERS ${HEADERS}
                   SOURCES ${SOURCES}
                   LIBRARIES ${BDM_REQUIRED_LIBRARIES})
