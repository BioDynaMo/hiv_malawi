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

#include "custom-operations.h"

namespace bdm {
namespace hiv_malawi {

void ResetCasualPartners::operator()() {
  // L2F converts a lambda call to a bdm::functor. We introduce this functor
  // because the ResourceManager::ForEachAgentParallel expects a functor.
  auto reset_functor = L2F([](Agent* agent) {
    auto* person = bdm_static_cast<Person*>(agent);
    person->ResetCasualPartners();
  });

  // Execute the functor for each agent in parallel.
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  rm->ForEachAgentParallel(reset_functor);
}

}  // namespace hiv_malawi
}  // namespace bdm
