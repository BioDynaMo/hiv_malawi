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
    auto* person = dynamic_cast<Person*>(agent);
    person->ResetCasualPartners();
  });

  // Execute the functor for each agent in parallel.
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  rm->ForEachAgentParallel(reset_functor);
}

PopulationData& PopulationData::operator+=(
    const PopulationData& other_population) {
  // add vectors for age_male
  std::transform(this->age_male.begin(), this->age_male.end(),
                 other_population.age_male.begin(), this->age_male.begin(),
                 std::plus<int>());
  // add vectors infected male
  std::transform(this->infected_male.begin(), this->infected_male.end(),
                 other_population.infected_male.begin(),
                 this->infected_male.begin(), std::plus<int>());
  // add vectors age_female
  std::transform(this->age_female.begin(), this->age_female.end(),
                 other_population.age_female.begin(), this->age_female.begin(),
                 std::plus<int>());
  // add vectors infected_female
  std::transform(this->infected_female.begin(), this->infected_female.end(),
                 other_population.infected_female.begin(),
                 this->infected_female.begin(), std::plus<int>());
  // add up healthy_male
  this->healthy_male += other_population.healthy_male;
  // add up healthy_female
  this->healthy_female += other_population.healthy_female;

  return *this;
}

void PopulationData::Print(std::ostream& out) const {
  out << "Population Information: \n";
  out << "healthy_male    : " << healthy_male << " \n";
  out << "healthy_female  : " << healthy_female << " \n";
  out << "infected_male   : "
      << std::accumulate(infected_male.begin(), infected_male.end(), 0);
  out << "\ninfected_female : "
      << std::accumulate(infected_female.begin(), infected_female.end(), 0);
  out << "\n\nage        male      female\n";
  for (size_t age = 0; age < std::max(age_female.size(), age_male.size());
       age++) {
    out << std::setw(3) << age << "    " << std::setw(8) << age_male[age]
        << "    " << std::setw(8) << age_female[age] << " \n";
  }
  out << std::endl;
}

void GetPopulationDataThreadLocal::operator()(Agent* agent,
                                              PopulationData* tl_pop) {
  auto* person = bdm_static_cast<Person*>(agent);
  if (person->sex_ == Sex::kMale) {
    tl_pop->age_male[static_cast<int>(person->age_)] += 1;
    if (person->state_ == GemsState::kHealthy) {
      tl_pop->healthy_male += 1;
    } else {
      tl_pop->infected_male[person->state_ - 1] += 1;
    }
  } else {
    tl_pop->age_female[static_cast<int>(person->age_)] += 1;
    if (person->state_ == GemsState::kHealthy) {
      tl_pop->healthy_female += 1;
    } else {
      tl_pop->infected_female[person->state_ - 1] += 1;
    }
  }
}

PopulationData CombinePopulationData::operator()(
    const SharedData<PopulationData>& tl_populations) {
  // Get object for total population
  PopulationData total_pop;
  for (PopulationData tl_population : tl_populations) {
    total_pop += tl_population;
  }
  return total_pop;
}

}  // namespace hiv_malawi
}  // namespace bdm
