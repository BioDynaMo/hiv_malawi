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

class ResetInfectionStatus : public StandaloneOperationImpl {
  BDM_OP_HEADER(ResetInfectionStatus);

  // Sets all agents to infected_this_time_step_ = false.
  void operator()() final;
};
}  // namespace hiv_malawi
}  // namespace bdm

#endif  // HIV_OPS_H_
