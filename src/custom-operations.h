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

/// Operation to reset the number of casual partner for each agent (in parallel)
struct ResetCasualPartners : public StandaloneOperationImpl {
  BDM_OP_HEADER(ResetCasualPartners);
  void operator()() override;
};

/// The struct population data describes the information that is extracted from
/// the population of agents at each timestep.
struct PopulationData {
  // Constructor to obtain correct size of zero-initialized vectors.
  PopulationData()
      : healthy_female(0),
        healthy_male(0),
        infected_female(GemsState::kGemsLast, 0),
        infected_male(GemsState::kGemsLast, 0),
        age_female(120, 0),
        age_male(120, 0) {}

  // member variables
  int healthy_female;
  int healthy_male;
  std::vector<int> infected_female;
  std::vector<int> infected_male;
  std::vector<int> age_female;
  std::vector<int> age_male;

  // Inplace-Add for Population data. The idea is to fill one struct
  // PopulationData per thread, the inplace add is used to combine the
  // information. See CombinePopulationData::operator()
  PopulationData& operator+=(const PopulationData& other_population);

  // Print the population data to std::out or a file. Similar functions could be
  // used to export to CSV or similar fileformats.
  void Print(std::ostream& out) const;
};

/// Functor to extract the PopulationData from agents. In the main simulation,
/// we combine it with a ReductionOp. This ReductionOp iterates over all agents
/// with all available threads. Each thread executes the operator().
struct GetPopulationDataThreadLocal
    : public Functor<void, Agent*, PopulationData*> {
  void operator()(Agent* agent, PopulationData* tl_pop);
};

/// Functor to combine multiple thread local PopulationData results into one.
struct CombinePopulationData
    : public Functor<PopulationData, const SharedData<PopulationData>&> {
  PopulationData operator()(
      const SharedData<PopulationData>& tl_populations) override;
};

}  // namespace bdm

#endif  // CUSTOM_OPERATIONS_H_
