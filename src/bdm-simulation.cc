// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
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

#include "bdm-simulation.h"
#include "population-initialization.h"

namespace bdm {

int Simulate(int argc, const char** argv) {
  Simulation simulation(argc, argv);

  // Define initial model - in this example: single cell at origin
  //auto* rm = simulation.GetResourceManager();
  //auto* cell = new Cell(30);
  //rm->AddAgent(cell);
  auto random = simulation.GetRandom();
  random->SetSeed(1234);
  initialize_population(random, 50);

  // Run simulation for one timestep
  simulation.GetScheduler()->Simulate(1);
  auto* rm = simulation.GetResourceManager();
  std::cout << "Simulation considers " << rm->GetNumAgents() << " persons.\n";

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace bdm
