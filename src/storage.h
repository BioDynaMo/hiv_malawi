// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN (Tobias Duswald, Lukas Breitwieser, Ahmad Hesam, Fons
// Rademakers) for the benefit of the BioDynaMo collaboration. All Rights 
// Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef STORAGE_H_
#define STORAGE_H_

#include <vector>
#include "datatypes.h"

namespace bdm {

// This function is currently not implemented. If necessary, this function can 
// be filled with code to save the Population-s at different stages of the 
// simulation to disk. Consider using ROOT files.
int save_to_disk(const std::vector<Population>& populations);

}  // namespace bdm

#endif  // STORAGE_H_
