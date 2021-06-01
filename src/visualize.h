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

#ifndef VISUALIZE_H_
#define VISUALIZE_H_

#include <vector>
#include "datatypes.h"

namespace bdm {

// This functions retrieves the collected time series from the active 
// simulation, saves the results as a JSON file, and plots the results.
int PlotAndSaveTimeseries();

}  // namespace bdm

#endif  // VISUALIZE_H_
