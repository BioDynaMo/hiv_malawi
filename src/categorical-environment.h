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

#ifndef CATEGORICAL_ENVIRONMENT_H_
#define CATEGORICAL_ENVIRONMENT_H_

#include "core/agent/agent_pointer.h"
#include "core/environment/environment.h"
#include "core/resource_manager.h"
#include "core/util/log.h"

#include "datatypes.h"
#include "person.h"
#include "sim-param.h"  // AM: Added to get location_mixing_matrix to update mate_location_distribution_

#include <cassert>
#include <iostream>
#include <random>

namespace bdm {

// This is a small helper class that wraps a vector of Agent pointers. It's a
// building block of the CategoricalEnvironment because we store a vector of
// AgentPointer for each of the categorical locations.
class AgentVector {
 private:
  // vector of AgentPointers
  std::vector<AgentPointer<Person>> agents_;

 public:
  // Get the number of agents in the vector
  size_t GetNumAgents() { return agents_.size(); }

  // Get a radom agent from the vector agents_
  AgentPointer<Person> GetRandomAgent();

  // Add an AgentPointer to the vector agents_
  void AddAgent(AgentPointer<Person> agent);

  // Delete vector entries and resize vector to 0
  void Clear();
};

// This is our customn BioDynaMo environment to describe the female population
// at all locations. By knowing the all females at a location, it's easy to
// select suitable mates during the MatingBehavior.
class CategoricalEnvironment : public Environment {
 private:
  // minimal age for sexual interaction
  int min_age_;
  // maximal age for sexual interaction
  int max_age_;
  // Number of age categories in the female_agent_index
  size_t no_age_categories_;
  // Number of locations in the female_agent_index
  size_t no_locations_;
  // Number of socialbehavioural categories in the female_agent_index
  size_t no_sociobehavioural_categories_;
  // Vector to store all female agents of within a certain age interval
  // [min_age_, max_age_].
  std::vector<AgentVector> female_agents_;

  // AM: Matrix to store cumulative probability to select a female mate from one
  // location given male agent location
  std::vector<std::vector<float>> mate_location_distribution_;
  // AM: DEBUG Matrix to store the locations of selected mates
  std::vector<std::vector<float>> mate_location_frequencies_;

 public:
  // Default constructor
  CategoricalEnvironment(int min_age = 15, int max_age = 40,
                         size_t no_age_categories = 1,
                         size_t no_locations = Location::kLocLast,
                         size_t no_sociobehavioural_categories = 1)
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

  // This is the update function, the is called automatically by BioDynaMo for
  // every simulation step. We delete the previous information and store a
  // vector of AgentPointers for each location.
  // AM TO DO: Update probability to select a female mate from each location.
  // Depends on static mixing matrix and update number of female agents per
  // location
  void Update() override {
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
          Log::Fatal("CategoricalEnvironment::Update()",
                     "person_ptr is nullptr");
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

  // Mapping from (location, age, socialbehaviour) to the appropriate position
  // in the female_agents_ index.
  inline size_t ComputeCompoundIndex(size_t location, size_t age, size_t sb) {
    assert(location < no_locations_);
    assert(age < no_age_categories_);
    assert(sb < no_sociobehavioural_categories_);
    return age + no_age_categories_ * location +
           (no_age_categories_ * no_locations_) * sb;
  }

  // Add an agent pointer to a certain location, age group, and sb category
  void AddAgentToIndex(AgentPointer<Person> agent, size_t location, size_t age,
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

  // Returns a random AgentPointer at a specific location, age group, and sb
  // category
  AgentPointer<Person> GetRamdomAgentFromIndex(size_t location, size_t age,
                                               size_t sb) {
    size_t compound_index = ComputeCompoundIndex(location, age, sb);
    if (compound_index >= female_agents_.size()) {
      Log::Fatal("CategoricalEnvironment::AddAgentToIndex()",
                 "Location index is out of bounds. Received compound index: ",
                 compound_index, " (loc ", location, ", age ", age, ", sb ", sb,
                 ") female_agents_.size(): ", female_agents_.size());
    }
    return female_agents_[compound_index].GetRandomAgent();
  };

  // Prints how many females are at a certain location, age group, and sb
  // category. Note that by population we refer to women between min_age_ and
  // max_age_.
  void DescribePopulation();

  // Get number of agents at location, age_category, and sb category
  size_t GetNumAgentsAtIndex(size_t location, size_t age, size_t sb) {
    size_t compound_index = ComputeCompoundIndex(location, age, sb);
    assert(compound_index < female_agents_.size());
    return female_agents_[compound_index].GetNumAgents();
  }

  // AM: Increase count of mates in given locations
  void IncreaseCountMatesInLocations(size_t loc_agent, size_t loc_mate) {
    mate_location_frequencies_[loc_agent][loc_mate] += 1.0;
  }

  // AM: Normalize count/frequencies of mates in given locations
  void NormalizeMateLocationFrequencies() {
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

  // AM: Print mate locations frequency matrix
  void PrintMateLocationFrequencies() {
    std::cout << "DEBUG : PrintMateLocationFrequencies()" << std::endl;
    for (int i = 0; i < Location::kLocLast; i++) {
      for (int j = 0; j < Location::kLocLast; j++) {
        std::cout << mate_location_frequencies_[i][j] << ",";
      }
      std::cout << std::endl;
    }
  }

  // Setter functions to access private member variables
  void SetMinAge(int min_age);
  void SetMaxAge(int max_age);

  // Getter functions to access private member variables
  int GetMinAge() { return min_age_; };
  int GetMaxAge() { return max_age_; };
  // AM: Add Getter of mate_location_distribution_
  const std::vector<float>& GetMateLocationDistribution(size_t loc) {
    return mate_location_distribution_[loc];
  };

  // The remaining public functinos are inherited from Environment but not
  // needed here.
  void Clear() override { ; };
  void ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                       const Agent& query, double squared_radius) override;

  std::array<int32_t, 6> GetDimensions() const override;

  std::array<int32_t, 2> GetDimensionThresholds() const override;

  LoadBalanceInfo* GetLoadBalanceInfo() override;

  Environment::NeighborMutexBuilder* GetNeighborMutexBuilder() override;
};

}  // namespace bdm

#endif  // CATEGORICAL_ENVIRONMENT_H_
