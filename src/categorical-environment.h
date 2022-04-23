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
namespace hiv_malawi {

// This is a small helper class that wraps a vector of Agent pointers. It's a
// building block of the CategoricalEnvironment because we store a vector of
// AgentPointer for each of the categorical locations.
class AgentVector {
 private:
  // thread-local vector of AgentPointers
  SharedData<std::vector<AgentPointer<Person>>> agents_;
  std::vector<uint64_t> offsets_;
  ThreadInfo* tinfo_ = nullptr;
  std::atomic<uint64_t> size_;
  Spinlock lock_;
  bool dirty_ = false;

  void UpdateOffsets();

 public:
  AgentVector();
  AgentVector(const AgentVector& other);

  // Get the number of agents in the vector
  size_t GetNumAgents() const { return size_; }

  // Get a radom agent from the vector agents_
  AgentPointer<Person> GetRandomAgent();

  // Get a agent at given index in the vector agents_
  AgentPointer<Person> GetAgentAtIndex(size_t i);

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
  // Number of age categories in the female_agent_ index
  size_t no_age_categories_;
  // Number of locations in the female_agent_, mothers_, and adults_ indexes
  size_t no_locations_;
  // Number of socialbehavioural categories in the female_agent_index
  size_t no_sociobehavioural_categories_;
  // Vector to store all female agents within a certain age interval
  // [min_age_, max_age_], indexed by location x age x sociobehaviours.
  std::vector<AgentVector> casual_female_agents_;
  // Vector to store all adult female agents, indexed by location x age x
  // sociobehaviours.
  std::vector<AgentVector> regular_female_agents_;
  // Vector to store all male agents within a certain age interval
  // [min_age_, max_age_], indexed by location x age x sociobehaviours.
  std::vector<AgentVector> casual_male_agents_;
  // Vector to store all adult single men looking for a regular female partner,
  // indexed by the location x age x sociobehaviours of their potential partner.
  std::vector<AgentVector> regular_male_agents_;
  // AM: Vector to store all potential mothers (female between min_age_ and
  // max_age_), indexed by location only.
  std::vector<AgentVector> mothers_;
  // Vector to store all adult agents (male and female older than min_age_),
  // indexed by location. Used to estimate population size per location, and
  // attractiveness.
  std::vector<AgentVector> adults_;
  // We only assign mother in the first update.
  bool mothers_are_assiged_;

  // AM: Matrix to store cumulative probability to select a female mate (casual
  // partner) from one compound category (location x age category x
  // sociobehaviour category) given male agent compound category
  std::vector<std::vector<float>> mate_compound_category_distribution_;

  // AM: Matrix to store cumulative probability to select a female regular
  // partner from one compound category (location x age category x
  // sociobehaviour category) given male agent compound category
  std::vector<std::vector<float>> reg_partner_compound_category_distribution_;

  // AM: DEBUG Matrix to store the locations of selected mates
  std::vector<std::vector<float>> mate_location_frequencies_;
  // AM: Matrix to store the current (year) cumulative probability to
  // relocate/migrate from one origin to one destination location Location x
  // Location
  std::vector<std::vector<float>> migration_location_distribution_;

 protected:
  // This is the update function, the is called automatically by BioDynaMo for
  // every simulation step. We delete the previous information and store a
  // vector of AgentPointers for each location.
  // AM: Update probability to select a female mate from each Location x Age
  // Category x Sociobehvioural category. Depends on static mixing matrices and
  // updated number of female agents per compound category
  void UpdateImplementation() override;

  // Update (at every iteration) matrix storing the probability to relocate from
  // origin location to destination location. Probability depends on a (location
  // x location) mixing matrix and the adult population size (attractivity) per
  // district
  void UpdateMigrationLocationProbability(
      size_t year_index,
      const std::vector<std::vector<std::vector<float>>>& migration_matrix);

  // Update (at every iteration) matrix storing the porbability that a male
  // agent selects a casual partner based on their compound categories (location
  // x age category x sociobehaviour category)
  void UpdateCasualPartnerCategoryDistribution(
      const std::vector<std::vector<float>>& location_mixing_matrix,
      const std::vector<std::vector<float>>& age_mixing_matrix,
      const std::vector<std::vector<float>>& sociobehav_mixing_matrix);

  void UpdateRegularPartnerCategoryDistribution(
      std::vector<std::vector<float>> reg_partner_age_mixing_matrix,
      std::vector<std::vector<float>> reg_partner_sociobehav_mixing_matrix);

 public:
  // Constructor
  CategoricalEnvironment(int min_age = 15, int max_age = 40,
                         size_t no_age_categories = 1,
                         size_t no_locations = Location::kLocLast,
                         size_t no_sociobehavioural_categories = 1);

  // Mapping from (location, age_category, socialbehaviour) to the appropriate
  // position in the female_agents_ and male_agents_ index.
  inline size_t ComputeCompoundIndex(size_t location, size_t age_category,
                                     size_t sb) {
    assert(location < no_locations_);
    assert(age_category < no_age_categories_);
    assert(sb < no_sociobehavioural_categories_);
    return age_category + no_age_categories_ * location +
           (no_age_categories_ * no_locations_) * sb;
  }

  // Mapping from position in the female_agents_ and male_agents_ index to the
  // appropriate location.
  inline size_t ComputeLocationFromCompoundIndex(size_t i) {
    assert(i < no_locations_ * no_age_categories_ *
                   no_sociobehavioural_categories_);

    return (int)(i % (no_age_categories_ * no_locations_)) / no_age_categories_;
  }

  // Mapping from position in the female_agents_ and male_agents_ index to the
  // appropriate age category.
  inline size_t ComputeAgeFromCompoundIndex(size_t i) {
    assert(i < no_locations_ * no_age_categories_ *
                   no_sociobehavioural_categories_);

    return (int)(i % (no_age_categories_ * no_locations_)) % no_age_categories_;
  }

  // Mapping from position in the female_agents_ and male_agents_ index to the
  // appropriate Socio-behavioural category.
  inline size_t ComputeSociobehaviourFromCompoundIndex(size_t i) {
    assert(i < no_locations_ * no_age_categories_ *
                   no_sociobehavioural_categories_);

    return (int)i / (no_age_categories_ * no_locations_);
  }

  // Add an agent pointer to a certain location, age group, and sb category in
  // casual_female_agents_ index.
  void AddCasualFemaleToIndex(AgentPointer<Person> agent, size_t location,
                              size_t age, size_t sb);
  // Add an agent pointer to a certain location, age group, and sb category in
  // regular_female_agents_ index.
  void AddRegularFemaleToIndex(AgentPointer<Person> agent, size_t location,
                               size_t age, size_t sb);

  // Add an agent pointer to a certain location, age group, and sb category in
  // casual_male_agents_ index.
  void AddCasualMaleToIndex(AgentPointer<Person> agent, size_t location,
                            size_t age, size_t sb);
  // Add a male agent pointer to a certain compound index (location x age group
  // x sb) category in regular_male_agents_ index.
  void AddRegularMaleToIndex(AgentPointer<Person> agent, size_t index);

  // Add an adult agent pointer to a certain location in adults_ index
  void AddAdultToLocation(AgentPointer<Person> agent, size_t location);

  // Add an agent pointer to a certain location in mothers_ index
  void AddMotherToLocation(AgentPointer<Person> agent, size_t location);

  // Returns a random AgentPointer at a specific location, age group, and sb
  // category in casual_female_agents_
  AgentPointer<Person> GetRandomCasualFemaleFromIndex(size_t location,
                                                      size_t age, size_t sb);
  // Returns a random AgentPointer at a specific location, age group, and sb
  // category in regular_female_agents_
  AgentPointer<Person> GetRandomRegularFemaleFromIndex(size_t location,
                                                       size_t age, size_t sb);

  // Returns a random AgentPointer at a specific compound category (location,
  // age group, and sb category) in casual_female_agents_
  AgentPointer<Person> GetRandomCasualFemaleFromIndex(size_t compound_index);

  // Returns a random AgentPointer at a specific compound category (location,
  // age group, and sb category) in regular_female_agents_
  AgentPointer<Person> GetRandomRegularFemaleFromIndex(size_t compound_index);

  // Returns a random Potential Mother (AgentPointer) at a specific location
  AgentPointer<Person> GetRandomMotherFromLocation(size_t location);

  // Prints how many potential casual female partners are at a certain location,
  // age group, and sb category. Note that by population we refer to adult women
  // between min_age_ and max_age_.
  void DescribePopulation();

  // Get number of potential casual female partners at location, age_category,
  // and sb category
  size_t GetNumCasualFemalesAtIndex(size_t location, size_t age, size_t sb);

  // Get number of potential regular female partners at location, age_category,
  // and sb category
  size_t GetNumRegularFemalesAtIndex(size_t location, size_t age, size_t sb);

  // Get number of potential casual female partners at location from
  // casual_female_agents_ index
  size_t GetNumCasualFemalesAtLocation(size_t location);

  // Get number of potential regular female partners at location from
  // regular_female_agents_ index
  size_t GetNumRegularFemalesAtLocation(size_t location);

  // Get number of female agents at location and age_category from
  // casual_female_agents_ index
  size_t GetNumCasualFemalesAtLocationAge(size_t location, size_t age);

  // Get number of female agents at location and age_category from
  // regular_female_agents_ index
  size_t GetNumRegularFemalesAtLocationAge(size_t location, size_t age);

  // Get number of adults at location from adults_ index
  size_t GetNumAdultsAtLocation(size_t location);

  // Setter functions to access private member variables
  void SetMinAge(int min_age);
  void SetMaxAge(int max_age);

  // Getter functions to access private member variables
  int GetMinAge() { return min_age_; };
  int GetMaxAge() { return max_age_; };
  // AM: Getter of no_age_categories_
  int GetNoAgeCategories() { return no_age_categories_; };
  // AM: Getter of no_sociobehavioural_categories_
  int GetNoSociobehaviouralCategories() {
    return no_sociobehavioural_categories_;
  };
  // AM: Getter of mate_compound_category_distribution_
  const std::vector<float>& GetMateCompoundCategoryDistribution(
      size_t loc, size_t age_category, size_t sociobehav);

  // AM: Getter of migration_location_distribution_
  const std::vector<float>& GetMigrationLocDistribution(size_t loc);

  // The remaining public functions are inherited from Environment but not
  // needed here.
  void Clear() override { ; };
  void ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                       const Agent& query, double squared_radius) override;

  // Not supported Neighbor search for this particular environment. Throws fatal
  // error.
  virtual void ForEachNeighbor(Functor<void, Agent*>& lambda,
                               const Agent& query, void* criteria) override;

  // Not supported Neighbor search for this particular environment. Throws fatal
  // error.
  virtual void ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                               const Double3& query_position,
                               double squared_radius,
                               const Agent* query_agent = nullptr) override;

  std::array<int32_t, 6> GetDimensions() const override;

  std::array<int32_t, 2> GetDimensionThresholds() const override;

  LoadBalanceInfo* GetLoadBalanceInfo() override;

  Environment::NeighborMutexBuilder* GetNeighborMutexBuilder() override;
};

}  // namespace hiv_malawi
}  // namespace bdm

#endif  // CATEGORICAL_ENVIRONMENT_H_
