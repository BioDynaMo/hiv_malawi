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

#include <iostream>
#include <vector>
#include "datatypes.h"
#include "core/util/log.h"

namespace bdm {

int save_to_disk(const std::vector<Population>& populations) { 
  Log::Warning("save_to_disk()","This function is not yet implemented.");
  return 1; 
}

}  // namespace bdm