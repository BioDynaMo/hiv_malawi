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

#include "categorical-environment.h"
#include "biodynamo.h"

namespace bdm {

////////////////////////////////////////////////////////////////////////////////
// AgentVector
////////////////////////////////////////////////////////////////////////////////

AgentPointer<Person> AgentVector::GetRandomAgent() {
  if (agents_.size() == 0) {
    Log::Fatal("AgentVector::GetRandomAgent()",
               "There are no females available for mating in one of your "
               "regions. Consider increasing the number of Agents.");
  }
  auto* r = Simulation::GetActive()->GetRandom();
  return agents_[r->Integer(agents_.size() - 1)];
}

void AgentVector::AddAgent(AgentPointer<Person> agent) {
  agents_.push_back(agent);
}

void AgentVector::Clear() {
  agents_.clear();
  agents_.reserve(10000);
}

////////////////////////////////////////////////////////////////////////////////
// CategoricalEnvironment
////////////////////////////////////////////////////////////////////////////////

CategoricalEnvironment::CategoricalEnvironment(
    int min_age, int max_age, size_t no_age_categories, size_t no_locations,
    size_t no_sociobehavioural_categories)
    : min_age_(min_age),
      max_age_(max_age),
      no_age_categories_(no_age_categories),
      no_locations_(no_locations),
      no_sociobehavioural_categories_(no_sociobehavioural_categories),
      female_agents_(no_age_categories * no_locations *
                     no_sociobehavioural_categories) {
  // Initialise all elements of mate_location_frequencies_ matrix with 0.0.
  mate_location_frequencies_.clear();
  mate_location_frequencies_.resize(Location::kLocLast);
  for (int i = 0; i < Location::kLocLast; i++) {
    mate_location_frequencies_[i].resize(Location::kLocLast);
    fill(mate_location_frequencies_[i].begin(),
         mate_location_frequencies_[i].end(), 0.0);
  }
  PrintMateLocationFrequencies();
}

// AM TO DO: Update probability to select a female mate from each location.
// Depends on static mixing matrix and update number of female agents per
// location
void CategoricalEnvironment::Update() {
  // // Debug
  // uint64_t iter =
  //     Simulation::GetActive()->GetScheduler()->GetSimulatedSteps();
  // if (iter < 4) {
  //   std::cout << "Iteration: " << iter << std::endl;
  //   std::cout << "Before clearing section" << std::endl;
  //   DescribePopulation();
  // }
  female_agents_.clear();
  female_agents_.resize(no_age_categories_ * no_locations_ *
                        no_sociobehavioural_categories_);
  // // DEBUG
  // if (iter < 4) {
  //   std::cout << "After clearing section" << std::endl;
  //   DescribePopulation();
  // }

  auto* rm = Simulation::GetActive()->GetResourceManager();
  rm->ForEachAgent([](Agent* agent) {
    auto* env = bdm_static_cast<CategoricalEnvironment*>(
        Simulation::GetActive()->GetEnvironment());
    auto* person = bdm_static_cast<Person*>(agent);
    if (person == nullptr) {
      Log::Fatal("CategoricalEnvironment::Update()", "person is nullptr");
    }

    if (person->sex_ == Sex::kFemale && person->age_ >= env->GetMinAge() &&
        person->age_ <= env->GetMaxAge()) {
      AgentPointer<Person> person_ptr = person->GetAgentPtr<Person>();
      if (person_ptr == nullptr) {
        Log::Fatal("CategoricalEnvironment::Update()", "person_ptr is nullptr");
      }
      // Todo(Aziza): bring in age_category and sociobehavioral category
      env->AddAgentToIndex(person_ptr, person->location_, 0, 0);
      // AM TO DO:
      // env->AddAgentToIndexAge(person->location_, person->age_,
      // person_ptr);

    } else {
      ;
    };
  });

  // AM : Update probability matrix to select female mate from one location
  // given location of male agent
  mate_location_distribution_.clear();
  mate_location_distribution_.resize(no_locations_ * no_age_categories_ *
                                     no_sociobehavioural_categories_);

  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  const auto* sparam =
      param->Get<SimParam>();  // AM : Needed to get location mixing matrix

  for (int i = 0; i < Location::kLocLast;
       i++) {  // Loop over male agent locations
    mate_location_distribution_[i].resize(Location::kLocLast);
    float sum = 0.0;
    // Todo(Aziza): bring in age_category and sociobehavioral category
    // Loop over female mate locations
    for (int j = 0; j < Location::kLocLast; j++) {
      mate_location_distribution_[i][j] =
          sparam->location_mixing_matrix[i][j] * GetNumAgentsAtIndex(j, 0, 0);
      sum += mate_location_distribution_[i][j];
    }
    for (int j = 0; j < Location::kLocLast;
         j++) {  // Loop again over female mate locations to NORMALIZE
                 // (probabilities should sum to 1 for each male location over
                 // all female mate locations) and compute CUMULATIVE
                 // probabilities.
      mate_location_distribution_[i][j] /= sum;
      if (j > 0) {
        mate_location_distribution_[i][j] +=
            mate_location_distribution_[i][j - 1];
      }
    }
    // Make sure that the commulative probability distribution actually ends
    // with 1.0 and not 0.9999x or something similar. (Fix for
    // SampleLocation warning).
    mate_location_distribution_[i][Location::kLocLast - 1] = 1.0;
  }

  // DEBUG: Check mate location distribution
  /*for (int i=0; i<Location::kLocLast; i++){
      for (int j=0; j<Location::kLocLast; j++){
          std::cout << mate_location_distribution_[i][j] << ", ";
      }
      std::cout << "\n";
  }*/
};

void CategoricalEnvironment::AddAgentToIndex(AgentPointer<Person> agent,
                                             size_t location, size_t age,
                                             size_t sb) {
  size_t compound_index = ComputeCompoundIndex(location, age, sb);
  if (compound_index >= female_agents_.size()) {
    Log::Fatal("CategoricalEnvironment::AddAgentToIndex()",
               "Location index is out of bounds. Received compound index: ",
               compound_index, " (loc ", location, ", age ", age, ", sb ", sb,
               ") female_agents_.size(): ", female_agents_.size());
  }
  female_agents_[compound_index].AddAgent(agent);
};

AgentPointer<Person> CategoricalEnvironment::GetRamdomAgentFromIndex(
    size_t location, size_t age, size_t sb) {
  size_t compound_index = ComputeCompoundIndex(location, age, sb);
  if (compound_index >= female_agents_.size()) {
    Log::Fatal("CategoricalEnvironment::AddAgentToIndex()",
               "Location index is out of bounds. Received compound index: ",
               compound_index, " (loc ", location, ", age ", age, ", sb ", sb,
               ") female_agents_.size(): ", female_agents_.size());
  }
  return female_agents_[compound_index].GetRandomAgent();
};

// Function for Debug - prints number of females per location.
void CategoricalEnvironment::DescribePopulation() {
  size_t total_population{0};
  std::cout << "\n ### population description ### \n";
  std::cout << "SB | location | age |  number of humans in index\n";
  for (size_t s = 0; s < no_sociobehavioural_categories_; s++) {
    for (size_t l = 0; l < no_locations_; l++) {
      for (size_t a = 0; a < no_age_categories_; a++) {
        auto num_agents = GetNumAgentsAtIndex(l, a, s);
        std::cout << std::setw(2) << s << "   " << std::setw(8) << l << "   "
                  << std::setw(3) << a << "   " << std::setw(25) << num_agents
                  << "\n";
        total_population += num_agents;
      }
    }
  }
  std::cout << "PopulationIndex Total: " << total_population << "\n"
            << std::endl;
};

size_t CategoricalEnvironment::GetNumAgentsAtIndex(size_t location, size_t age,
                                                   size_t sb) {
  size_t compound_index = ComputeCompoundIndex(location, age, sb);
  assert(compound_index < female_agents_.size());
  return female_agents_[compound_index].GetNumAgents();
}

void CategoricalEnvironment::IncreaseCountMatesInLocations(size_t loc_agent,
                                                           size_t loc_mate) {
  mate_location_frequencies_[loc_agent][loc_mate] += 1.0;
}

void CategoricalEnvironment::NormalizeMateLocationFrequencies() {
  for (int i = 0; i < Location::kLocLast; i++) {
    float sum = 0.0;
    for (int j = 0; j < Location::kLocLast; j++) {
      sum += mate_location_frequencies_[i][j];
    }
    for (int j = 0; j < Location::kLocLast; j++) {
      mate_location_frequencies_[i][j] /= sum;
    }
  }
}

void CategoricalEnvironment::PrintMateLocationFrequencies() {
  std::cout << "DEBUG : PrintMateLocationFrequencies()" << std::endl;
  for (int i = 0; i < Location::kLocLast; i++) {
    for (int j = 0; j < Location::kLocLast; j++) {
      std::cout << mate_location_frequencies_[i][j] << ",";
    }
    std::cout << std::endl;
  }
}

const std::vector<float>& CategoricalEnvironment::GetMateLocationDistribution(
    size_t loc) {
  return mate_location_distribution_[loc];
};

void CategoricalEnvironment::SetMinAge(int min_age) {
  if (min_age >= 0 && min_age <= 120) {
    min_age_ = min_age;
  } else {
    Log::Fatal("CategoricalEnvironment::SetMinAge()",
               "SetMinAge ignored, min_age must be in [0,120].");
  }
}

void CategoricalEnvironment::SetMaxAge(int max_age) {
  if (max_age >= 0 && max_age <= 120) {
    max_age_ = max_age;
  } else {
    Log::Fatal("CategoricalEnvironment::SetMaxAge()",
               "SetMinAge ignored, max_age must be in [0,120].");
  }
}

// Code for virtual functions (ignore)
void CategoricalEnvironment::ForEachNeighbor(
    Functor<void, Agent*, double>& lambda, const Agent& query,
    double squared_radius) {
  ;
};

// Code for virtual functions (ignore)
std::array<int32_t, 6> CategoricalEnvironment::GetDimensions() const {
  static std::array<int32_t, 6> arr;
  return arr;
};

// Code for virtual functions (ignore)
std::array<int32_t, 2> CategoricalEnvironment::GetDimensionThresholds() const {
  static std::array<int32_t, 2> arr;
  return arr;
};

// Code for virtual functions (ignore)
LoadBalanceInfo* CategoricalEnvironment::GetLoadBalanceInfo() {
  Log::Fatal("CategoricalEnvironment::GetLoadBalanceInfo()",
             "LoadBalancing not supported for this environment.");
  return nullptr;
};

// Code for virtual functions (ignore)
Environment::NeighborMutexBuilder*
CategoricalEnvironment::GetNeighborMutexBuilder() {
  return nullptr;
};

}  // namespace bdm
