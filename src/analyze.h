// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN and the University of Geneva for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
//
// -----------------------------------------------------------------------------

#ifndef VISUALIZE_H_
#define VISUALIZE_H_

#include <vector>
#include "datatypes.h"

namespace bdm {
namespace hiv_malawi {

// This functions defines which data should be extracted from the simulation
// and collected for each time step using the `TimeSeries` object.
void DefineAndRegisterCollectors();

// This functions retrieves the collected time series from the active
// simulation, saves the results as a JSON file, and plots the results.
int PlotAndSaveTimeseries();

}  // namespace hiv_malawi
}  // namespace bdm

#endif  // VISUALIZE_H_
