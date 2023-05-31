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

#ifndef HIV_OPS_H_
#define HIV_OPS_H_

#include "biodynamo.h"
#include "person.h"

namespace bdm {
namespace hiv_malawi {

class GetOlderOperation : public StandaloneOperationImpl {
  BDM_OP_HEADER(GetOlderOperation);

  // Wraps the ProcessAgent function in a ForEachAgentParallel call.
  void operator()() final;

 private:
  // -- Helper functions -- //
  // Implements the former GetOlder behavior.
  void ProcessAgent(Agent* agent);

  // AM : Get mortality rate by age
  float GetMortalityRateAge(
      float age, const std::vector<int>& mortality_rate_age_transition,
      const std::vector<float>& mortality_rate_by_age);

  // AM: Get HIV-related mortality rate
  float GetMortalityRateHiv(int state,
                            const std::vector<float>& hiv_mortality_rate);
};

}  // namespace hiv_malawi
}  // namespace bdm

#endif  // HIV_OPS_H_
