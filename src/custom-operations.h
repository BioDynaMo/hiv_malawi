// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN and the University of Geneva for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
//
// -----------------------------------------------------------------------------

#ifndef CUSTOM_OPERATIONS_H_
#define CUSTOM_OPERATIONS_H_

#include "core/operation/operation.h"
#include "core/resource_manager.h"
#include "person.h"

namespace bdm {
namespace hiv_malawi {

/// Operation to reset the number of casual partner for each agent (in parallel)
struct ResetCasualPartners : public StandaloneOperationImpl {
  BDM_OP_HEADER(ResetCasualPartners);
  void operator()() override;
};

}  // namespace hiv_malawi
}  // namespace bdm

#endif  // CUSTOM_OPERATIONS_H_
