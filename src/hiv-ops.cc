// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN, UniGe. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "hiv-ops.h"
#include "person.h"

namespace bdm {
namespace hiv_malawi {

// Apply operation
void ResetInfectionStatus::operator()() {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  auto fill = L2F([&](Agent* a, AgentHandle) {
    static_cast<Person*>(a)->infected_this_time_step_ = false;
  });
  // ToDo: Parallelize
  rm->ForEachAgentParallel(fill);
};

}  // namespace hiv_malawi
}  // namespace bdm
