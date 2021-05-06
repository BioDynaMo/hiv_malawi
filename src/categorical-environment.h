// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN (Tobias Duswald, Lukas Breitwieser, Ahmad Hesam, Fons
// Rademakers) for the benefit of the BioDynaMo collaboration. All Rights 
// Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
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
  // Has the vector been shuffled or not
  bool shuffled_;
  // Iter to go through vector and obtain random Agents
  size_t iter;

  // Shuffle the vector agents_
  void shuffle_();

 public:
  // Get the number of agents in the vector
  size_t GetNumAgents() { return agents_.size(); }

  // Return if vector is suffled or not
  bool IsShuffled() { return shuffled_; };

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
 public:
  // Default constructor
  CategoricalEnvironment(int min_age = 15, int max_age = 40,
                         size_t n_loc = Location::kLocLast)
      : min_age_(min_age), max_age_(max_age), female_agents_(n_loc) {}

  // This is the update function, the is called automatically by BioDynaMo for 
  // every simulation step. We delete the previous information and store a 
  // vector of AgentPointers for each location.
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
    female_agents_.resize(Location::kLocLast);
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
        env->AddAgentToLocation(person->location_, person_ptr);
      } else {
        ;
      };
    });
  };

  // Add an agent pointer to a certain location
  void AddAgentToLocation(size_t loc, AgentPointer<Person> agent) {
    if (female_agents_.size() <= loc) {
      Log::Fatal("CategoricalEnvironment::AddAgentToLocation()",
                 "Location index is out of bounds. Received loc: ", loc,
                 "female_agents_.size(): ", female_agents_.size());
    }
    female_agents_[loc].AddAgent(agent);
  };

  // Prints how many females are at a certain location
  void DescribePopulation();

  // Returns a random AgentPointer at a specific location
  AgentPointer<Person> GetRamdomAgentAtLocation(size_t loc) {
    if (female_agents_.size() <= loc) {
      Log::Fatal("CategoricalEnvironment::AddAgentToLocation()",
                 "Location index is out of bounds. Received loc: ", loc,
                 "female_agents_.size(): ", female_agents_.size());
    }
    return female_agents_[loc].GetRandomAgent();
  };


  // Setter functions to access private member variables
  void SetNumLocations(size_t num_locations);
  void SetMinAge(int min_age);
  void SetMaxAge(int max_age);

  // Getter functions to access private member variables
  size_t GetNumLocations() { return female_agents_.size(); };
  int GetMinAge() { return min_age_; };
  int GetMaxAge() { return max_age_; };

  // The remaining public functinos are inherited from Environment but not
  // needed here.
  void Clear() override { ; };
  void ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                       const Agent& query, double squared_radius) override;

  const std::array<int32_t, 6>& GetDimensions() const override;

  const std::array<int32_t, 2>& GetDimensionThresholds() const override;

  LoadBalanceInfo* GetLoadBalanceInfo() override;

  Environment::NeighborMutexBuilder* GetNeighborMutexBuilder() override;

 private:
  // minimal age for sexual interaction
  int min_age_;
  // maximal age for sexual interaction
  int max_age_;
  // Vector to store all female agents of within a certain age interval
  // [min_age_, max_age_].
  std::vector<AgentVector> female_agents_;
};

}  // namespace bdm

#endif  // CATEGORICAL_ENVIRONMENT_H_
