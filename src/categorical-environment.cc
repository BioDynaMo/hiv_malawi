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
                     no_sociobehavioural_categories),
      mothers_(no_locations),
      adults_(no_locations),
      mothers_are_assiged_(false) {}


// AM : Update probability to select a female mate from each location x age x sb
// compound category. Depends on static mixing matrices and updated number of
// female agents per category
void CategoricalEnvironment::UpdateImplementation() {
  // Debug
  /*uint64_t iter =
       Simulation::GetActive()->GetScheduler()->GetSimulatedSteps();
  if (iter < 4) {
     std::cout << "Iteration: " << iter << std::endl;
     std::cout << "Before clearing section" << std::endl;
     DescribePopulation();
  }*/
  female_agents_.clear();
  female_agents_.resize(no_age_categories_ * no_locations_ *
                        no_sociobehavioural_categories_);

  adults_.clear();
  adults_.resize(no_locations_);
  // DEBUG
  /*if (iter < 4) {
     std::cout << "After clearing section" << std::endl;
     DescribePopulation();
  }*/

  
  auto* rm = Simulation::GetActive()->GetResourceManager();
  rm->ForEachAgent([](Agent* agent) {
    auto* env = bdm_static_cast<CategoricalEnvironment*>(
        Simulation::GetActive()->GetEnvironment());
    auto* person = bdm_static_cast<Person*>(agent);
    if (person == nullptr) {
      Log::Fatal("CategoricalEnvironment::UpdateImplementation()",
                 "person is nullptr");
    }

    // Index women (potential partners; >=15yo; maxAge??) by location x age x sociobhevioural risk
    // TO DO AM: Female needs to be single to be selected as casual (or regular) partner (?)
    if (person->sex_ == Sex::kFemale && person->age_ >= env->GetMinAge() &&
        person->age_ <= env->GetMaxAge()) {
      AgentPointer<Person> person_ptr = person->GetAgentPtr<Person>();
      if (person_ptr == nullptr) {
        Log::Fatal("CategoricalEnvironment::UpdateImplementation()",
                   "person_ptr is nullptr");
      }
      // Compute age category of female agent
      size_t age_category =
          person->GetAgeCategory(env->GetMinAge(), env->GetNoAgeCategories());
      // Add female agent to the right index, based on her location, age
      // category and socio-behavioural category
      env->AddAgentToIndex(person_ptr, person->location_, age_category,
                           person->social_behaviour_factor_);
    };

    // Index adults by location
    if (person->age_ >= env->GetMinAge()) {
      AgentPointer<Person> person_ptr = person->GetAgentPtr<Person>();
      if (person_ptr == nullptr) {
        Log::Fatal("CategoricalEnvironment::UpdateImplementation()",
                   "person_ptr is nullptr");
      }
      env->AddAdultToLocation(person_ptr, person->location_);
    };

    // DEBUG: ARE NEW-BORN RECOGNIZED BY THEIR MOTHERS'
    /*if (person->age_ < 1){
        if (person->mother_ != nullptr){
            if (person->mother_->IsParentOf(person->GetAgentPtr<Person>()) ==
    0){ std::cout << "I am a new-born! My mother points on me? " <<
    person->mother_->IsParentOf(person->GetAgentPtr<Person>()) << std::endl;
            }
        } else {
            std::cout << "I am a new-born! My mother is nullptr " << std::endl;
        }
    }*/
    // DEBUG: ARE MOTHERS RECOGNIZED BY THEIR CHILDREN
    /*if (person->sex_ == Sex::kFemale){
          for (int c = 0; c < person->GetNumberOfChildren(); c++){
              bool ok =
      person->children_[c]->IsChildOf(person->GetAgentPtr<Person>()); if (!ok){
                  std::cout << "I am a woman! My child n° " << c << " (age " <<
      person->children_[c]->age_ << ") does not point on me! Null Mum ? "<<
      (person->children_[c]->mother_ == nullptr) << std::endl;
              }
          }
      }*/
  });

  // During first iteration, assign mothers to children
  if (!mothers_are_assiged_) {
    mothers_are_assiged_ = true;
    uint64_t iter = Simulation::GetActive()->GetScheduler()->GetSimulatedSteps();
    std::cout << "iter = " << iter << " ==> Assign mothers to children "
              << std::endl;

    // AM: Index Potential Mothers by location
    mothers_.clear();
    mothers_.resize(no_locations_);

    rm->ForEachAgent([](Agent* agent) {
      auto* env = bdm_static_cast<CategoricalEnvironment*>(
          Simulation::GetActive()->GetEnvironment());
      auto* person = bdm_static_cast<Person*>(agent);
      if (person == nullptr) {
        Log::Fatal("CategoricalEnvironment::UpdateImplementation()",
                   "person is nullptr");
      }

      if (person->sex_ == Sex::kFemale && person->age_ >= env->GetMinAge() && 
          person->age_ <= env->GetMaxAge()) {
        AgentPointer<Person> person_ptr = person->GetAgentPtr<Person>();
        if (person_ptr == nullptr) {
          Log::Fatal("CategoricalEnvironment::UpdateImplementation()",
                    "person_ptr is nullptr");
        }
        
        // Add potential mother to the location index
        env->AddMotherToLocation(person_ptr, person->location_);           
      };
    });

    // AM: Assign mothers to children
    int cntr = 0;
    rm->ForEachAgent([&](Agent* agent) {
      auto* env = bdm_static_cast<CategoricalEnvironment*>(
          Simulation::GetActive()->GetEnvironment());
      auto* person = bdm_static_cast<Person*>(agent);
      if (person == nullptr) {
        Log::Fatal("CategoricalEnvironment::UpdateImplementation()",
                   "person is nullptr");
      }

      if (person->age_ < env->GetMinAge()) {
        // std::cout << "I am a child (" << person->age_ << ") looking for a
        // mother at location " << person->location_ << std::endl;
        // Select a mother, at same location as child 
        // TO DO AM: ideally, mother is at least 15 and at most 40 years older than child
        person->mother_ = env->GetRamdomMotherFromLocation(person->location_);
        // Check that mother and child have the same location
        if (person->location_ != person->mother_->location_) {
          Log::Warning("CategoricalEnvironment::UpdateImplementation()",
                       "child assigned to mother with different location");
        }
        AgentPointer<Person> person_ptr = person->GetAgentPtr<Person>();
        if (person_ptr == nullptr) {
          Log::Fatal("CategoricalEnvironment::UpdateImplementation()",
                     "person_ptr is nullptr");
        }
        person->mother_->AddChild(person_ptr);
        // std::cout << "Found a mother (age "<< person->mother_->age_ << ") at
        // location " << person->mother_->location_ << std::endl;
        cntr += 1;
      };
    });
    std::cout << "Assigned " << cntr << " children to mothers." << std::endl;

    // DEBUG: All agents' children are at the same location as their mothers
    /*rm->ForEachAgent([](Agent* agent) {
      auto* person = bdm_static_cast<Person*>(agent);
      if (person == nullptr) {
        Log::Fatal("CategoricalEnvironment::UpdateImplementation()",
                   "person is nullptr");
      }

      for (int c = 0; c < person->GetNumberOfChildren(); c++) {
        if (person->children_[c]->location_ != person->location_) {
          Log::Warning("CategoricalEnvironment::UpdateImplementation()",
                       "After child/mother assignment, child has different "
                       "location from mother");
        }
      }

      // Check that mothers recognise their children
      if (person->age_ < 15) {
        if (!person->mother_->IsParentOf(person->GetAgentPtr<Person>())) {
          Log::Warning("CategoricalEnvironment::UpdateImplementation()",
                       "After child/mother assignment, child points on mother, "
                       "who does not recognise him/her.");
        }
      }
    });*/
  }
  // TO DO: Assign regular partners to men (or women)

  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  const auto* sparam =
  param->Get<SimParam>();  // AM : Needed to get location mixing matrix
  
  // AM: Probability of migration location depends on the current year
  int year = static_cast<int>(
      sparam->start_year +
      sim->GetScheduler()->GetSimulatedSteps());  // Current year
  // If no transition year is higher than current year, then use last
  // transition year
  int year_index = sparam->migration_year_transition.size() - 1;
  for (int y = 0; y < sparam->migration_year_transition.size() - 1; y++) {
    if (year < sparam->migration_year_transition[y + 1]) {
      year_index = y;
      break;
    }
  }
  // AM : Update probability matrix to select migration/relocation destination given current year index and origin location 
  UpdateMigrationLocationProbability(year_index, sparam->migration_matrix);

  // AM : Update probability matrix to select female mate
  // given location, age and socio-behaviour of male agent
  UpdateCasualPartnerCategoryDistribution(sparam->location_mixing_matrix,
                                          sparam->age_mixing_matrix,
                                          sparam->sociobehav_mixing_matrix);
};

void CategoricalEnvironment::UpdateCasualPartnerCategoryDistribution(std::vector<std::vector<float>> location_mixing_matrix,
std::vector<std::vector<float>> age_mixing_matrix,
std::vector<std::vector<float>> sociobehav_mixing_matrix
) {
  mate_compound_category_distribution_.clear();
  mate_compound_category_distribution_.resize(
      no_locations_ * no_age_categories_ * no_sociobehavioural_categories_);

  for (int i = 0;
       i < no_locations_ * no_age_categories_ * no_sociobehavioural_categories_;
       i++) {  // Loop over male agent compound categories (location x age x
               // socio-behaviour)

    // AM : Probability distribution matrix to select a female mate given male
    // agent and female mate compound categories
    mate_compound_category_distribution_[i].resize(
        no_locations_ * no_age_categories_ * no_sociobehavioural_categories_);

    // Get Location, Age and Socio-behaviour of male agent from Index
    size_t l_i = ComputeLocationFromCompoundIndex(i);
    size_t a_i = ComputeAgeFromCompoundIndex(i);
    size_t s_i = ComputeSociobehaviourFromCompoundIndex(i);

    // Step 1 - Location: Compute probability to select a female mate from
    // each location
    std::vector<float> proba_locations(no_locations_, 0.0);
    float sum_locations = 0.0;
    for (size_t l_j = 0; l_j < no_locations_; l_j++) {
      proba_locations[l_j] = location_mixing_matrix[l_i][l_j] *
                             GetNumAgentsAtLocation(l_j);
      sum_locations += proba_locations[l_j];
    }
    // Normalise to get probability between 0 and 1
    if (sum_locations > 0) {
      for (size_t l_j = 0; l_j < no_locations_; l_j++) {
        proba_locations[l_j] /= sum_locations;
      }
    }

    // Step 2 -  Age: Compute probability to select a female mate from each
    // age category given the selected location
    std::vector<std::vector<float>> proba_ages_given_location;
    proba_ages_given_location.resize(no_locations_);
    for (size_t l_j = 0; l_j < no_locations_;
         l_j++) {  // Loop over potential locations of female mate
      proba_ages_given_location[l_j].resize(no_age_categories_);
      float sum_ages = 0.0;
      for (size_t a_j = 0; a_j < no_age_categories_;
           a_j++) {  // For each location l_j, compute probability to select a
                     // female mate from each age category a_j
        proba_ages_given_location[l_j][a_j] =
            age_mixing_matrix[a_i][a_j] *
            GetNumAgentsAtLocationAge(l_j, a_j);
        sum_ages += proba_ages_given_location[l_j][a_j];
      }
      // Normalise to compute probability between 0 and 1 to select from each
      // age category given a location
      if (sum_ages > 0) {
        for (size_t a_j = 0; a_j < no_age_categories_; a_j++) {
          proba_ages_given_location[l_j][a_j] /= sum_ages;
        }
      }
    }

    // Step 3 - Socio-behaviour : Compute probability to select from each
    // socio-behavioural category given the selected location and age
    std::vector<std::vector<std::vector<float>>> proba_socio_given_location_age;
    proba_socio_given_location_age.resize(no_locations_);
    for (size_t l_j = 0; l_j < no_locations_; l_j++) {
      proba_socio_given_location_age[l_j].resize(no_age_categories_);
      for (size_t a_j = 0; a_j < no_age_categories_; a_j++) {
        proba_socio_given_location_age[l_j][a_j].resize(
            no_sociobehavioural_categories_);
        float sum_socio = 0.0;
        for (size_t s_j = 0; s_j < no_sociobehavioural_categories_; s_j++) {
          proba_socio_given_location_age[l_j][a_j][s_j] =
              sociobehav_mixing_matrix[s_i][s_j] *
              GetNumAgentsAtIndex(l_j, a_j, s_j);
          sum_socio += proba_socio_given_location_age[l_j][a_j][s_j];
        }
        // Normalise to compute probability between 0 and 1 to select each
        // socio-behaviour given location and age
        if (sum_socio > 0) {
          for (size_t s_j = 0; s_j < no_sociobehavioural_categories_; s_j++) {
            proba_socio_given_location_age[l_j][a_j][s_j] /= sum_socio;
          }
        }
      }
    }

    // Compute the final probability that a male agent of compound category i,
    // selects a female mate of compound category j.
    for (size_t j = 0; j < no_locations_ * no_age_categories_ *
                               no_sociobehavioural_categories_;
         j++) {
      size_t l_j = ComputeLocationFromCompoundIndex(j);
      size_t a_j = ComputeAgeFromCompoundIndex(j);
      size_t s_j = ComputeSociobehaviourFromCompoundIndex(j);

      mate_compound_category_distribution_[i][j] =
          proba_locations[l_j] * proba_ages_given_location[l_j][a_j] *
          proba_socio_given_location_age[l_j][a_j][s_j];

      // Compute Cumulative distribution
      if (j > 0) {
        mate_compound_category_distribution_[i][j] +=
            mate_compound_category_distribution_[i][j - 1];
      }
    }

    // Make sure that the commulative probability distribution actually ends
    // with 1.0 and not 0.9999x or something similar. Do not fix only the last
    // element but all the previous ones, which had the same cumulative
    // probability ~1 (<=> probability = 0)
    size_t no_compound_categories =
        no_locations_ * no_age_categories_ * no_sociobehavioural_categories_;
    auto last_cumul_proba =
        mate_compound_category_distribution_[i][no_compound_categories - 1];
    // Go looking backward
    for (size_t j = no_compound_categories - 1; j >= 0; j--) {
      if (mate_compound_category_distribution_[i][j] == last_cumul_proba) {
        mate_compound_category_distribution_[i][j] = 1.0;
      } else {
        break;
      }
    }
  }
}

void CategoricalEnvironment::UpdateMigrationLocationProbability(size_t year_index, std::vector<std::vector<std::vector<float>>> migration_matrix) {
  migration_location_distribution_.clear(); 
  migration_location_distribution_.resize(no_locations_);
  for (int i = 0; i < no_locations_; i++) {
    migration_location_distribution_[i].resize(no_locations_);
    // Compute Denominator for Normalization
    float sum = 0.0;
    for (int j = 0; j < no_locations_; j++) {
      // Weight migration_matrix with population size per destination
      migration_location_distribution_[i][j] = migration_matrix[year_index][i][j] * GetNumAdultsAtLocation(j);
      sum += migration_location_distribution_[i][j];
    }
    // Normalize and Cumulate
    for (int j = 0; j < no_locations_; j++) {
      if (j == 0) {
        migration_location_distribution_[i][j] =
            migration_location_distribution_[i][j] / sum;
      } else {
        migration_location_distribution_[i][j] =
            migration_location_distribution_[i][j - 1] +
            migration_location_distribution_[i][j] / sum;
      }
    }
    // Check that we do end with 1.0 (not 0.99999, or 1.00001)
    auto last_cumul_proba = migration_location_distribution_[i][no_locations_ - 1];
    // Go looking backward
    for (size_t j = no_locations_ - 1; j >= 0; j--) {
      if (migration_location_distribution_[i][j] == last_cumul_proba) {
        migration_location_distribution_[i][j] = 1.0;
      } else {
        break;
      }
    }
  }

  /*std::cout << "migration_location_distribution_ = " << std::endl;
  for (int i = 0; i < no_locations_; i++) {
    for (int j = 0; j < no_locations_; j++) {
        std::cout << migration_location_distribution_[i][j] << ", ";
    }
    std::cout << std::endl;
  }*/
};

void CategoricalEnvironment::AddAdultToLocation(AgentPointer<Person> agent, size_t location) {
  assert(location >= 0 and location < no_locations_);

  if (location >= adults_.size()) {
    Log::Fatal("CategoricalEnvironment::AddAgentToIndex()",
               "Location index is out of bounds. Received (loc ", location, ") and adults_.size(): ", adults_.size());
  }
  adults_[location].AddAgent(agent);
}

void CategoricalEnvironment::AddAgentToIndex(AgentPointer<Person> agent,
                                             size_t location, size_t age,
                                             size_t sb) {
  assert(location >= 0 and location < no_locations_);
  assert(age >= 0 and age < no_age_categories_);
  assert(sb >= 0 and sb < no_sociobehavioural_categories_);

  size_t compound_index = ComputeCompoundIndex(location, age, sb);
  if (compound_index >= female_agents_.size()) {
    Log::Fatal("CategoricalEnvironment::AddAgentToIndex()",
               "Location index is out of bounds. Received compound index: ",
               compound_index, " (loc ", location, ", age ", age, ", sb ", sb,
               ") female_agents_.size(): ", female_agents_.size());
  }
  female_agents_[compound_index].AddAgent(agent);
};

void CategoricalEnvironment::AddMotherToLocation(AgentPointer<Person> agent,
                                                 size_t location) {
  assert(location >= 0 and location < no_locations_);
  mothers_[location].AddAgent(agent);
}

AgentPointer<Person> CategoricalEnvironment::GetRamdomAgentFromIndex(
    size_t location, size_t age, size_t sb) {
  size_t compound_index = ComputeCompoundIndex(location, age, sb);
  if (compound_index >= female_agents_.size()) {
    Log::Fatal("CategoricalEnvironment::GetRamdomAgentFromIndex()",
               "Location index is out of bounds. Received compound index: ",
               compound_index, " (loc ", location, ", age ", age, ", sb ", sb,
               ") female_agents_.size(): ", female_agents_.size());
  }
  return female_agents_[compound_index].GetRandomAgent();
};

AgentPointer<Person> CategoricalEnvironment::GetRamdomAgentFromIndex(
    size_t compound_index) {
  size_t location = ComputeLocationFromCompoundIndex(compound_index);
  size_t age = ComputeAgeFromCompoundIndex(compound_index);
  size_t sb = ComputeSociobehaviourFromCompoundIndex(compound_index);

  if (compound_index >= female_agents_.size()) {
    Log::Fatal("CategoricalEnvironment::GetRamdomAgentFromIndex()",
               "Location index is out of bounds. Received compound index: ",
               compound_index, " (loc ", location, ", age ", age, ", sb ", sb,
               ") female_agents_.size(): ", female_agents_.size());
  }
  if (female_agents_[compound_index].GetNumAgents() == 0) {
    Log::Fatal("CategoricalEnvironment::GetRamdomAgentFromIndex()",
               "Female agents empty. Received compound index: ", compound_index,
               " (loc ", location, ", age ", age, ", sb ", sb, ")");
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

size_t CategoricalEnvironment::GetNumAdultsAtLocation(size_t location) {
  assert(location < adults_.size());
  return adults_[location].GetNumAgents();
}

size_t CategoricalEnvironment::GetNumAgentsAtLocationAge(size_t location,
                                                         size_t age) {
  size_t sum = 0;
  for (size_t sb = 0; sb < no_sociobehavioural_categories_; sb++) {
    size_t compound_index = ComputeCompoundIndex(location, age, sb);
    assert(compound_index < female_agents_.size());
    sum += female_agents_[compound_index].GetNumAgents();
  }
  return sum;
}

size_t CategoricalEnvironment::GetNumAgentsAtLocation(size_t location) {
  size_t sum = 0;
  for (size_t sb = 0; sb < no_sociobehavioural_categories_; sb++) {
    for (size_t age = 0; age < no_age_categories_; age++) {
      size_t compound_index = ComputeCompoundIndex(location, age, sb);
      assert(compound_index < female_agents_.size());
      sum += female_agents_[compound_index].GetNumAgents();
    }
  }
  return sum;
}

// AM: GET Random mother from location
AgentPointer<Person> CategoricalEnvironment::GetRamdomMotherFromLocation(
    size_t location) {
  if (mothers_[location].GetNumAgents() == 0) {
    Log::Warning("CategoricalEnvironment::GetRamdomMotherFromLocation()",
                 "Mothers empty. Received location: ", location);
    return AgentPointer<Person>();  // nullptr
  }
  return mothers_[location].GetRandomAgent();
}

const std::vector<float>&
CategoricalEnvironment::GetMateCompoundCategoryDistribution(size_t loc,
                                                            size_t age_category,
                                                            size_t sociobehav) {
  size_t compound_index = ComputeCompoundIndex(loc, age_category, sociobehav);
  return mate_compound_category_distribution_[compound_index];
};

const std::vector<float>&
CategoricalEnvironment::GetMigrationLocDistribution(size_t loc){
    return migration_location_distribution_[loc];
}

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

void CategoricalEnvironment::ForEachNeighbor(Functor<void, Agent*>& lambda,
                                             const Agent& query,
                                             void* criteria) {
  Log::Fatal("CategoricalEnvironment::ForEachNeighbor",
             "Function call not supported in this envrionment.");
};

void CategoricalEnvironment::ForEachNeighbor(
    Functor<void, Agent*, double>& lambda, const Double3& query_position,
    double squared_radius, const Agent* query_agent) {
  Log::Fatal("CategoricalEnvironment::ForEachNeighbor",
             "Function call not supported in this envrionment.");
}

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
