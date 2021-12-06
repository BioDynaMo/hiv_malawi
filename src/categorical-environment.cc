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
}

// AM : Update probability to select a female mate from each location x age x sb compound category.
// Depends on static mixing matrices and updated number of female agents per
// category
void CategoricalEnvironment::Update() {
  
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
      Log::Fatal("CategoricalEnvironment::Update()", "person is nullptr");
    }

    if (person->sex_ == Sex::kFemale && person->age_ >= env->GetMinAge() &&
        person->age_ <= env->GetMaxAge()) {
      AgentPointer<Person> person_ptr = person->GetAgentPtr<Person>();
      if (person_ptr == nullptr) {
        Log::Fatal("CategoricalEnvironment::Update()", "person_ptr is nullptr");
      }
      // Compute age category of female agent
      size_t age_category = person->GetAgeCategory(env->GetMinAge(),env->GetNoAgeCategories());
      // Add female agent to the right index, based on her location, age category and socio-behavioural category
      env->AddAgentToIndex(person_ptr, person->location_, age_category, person->social_behaviour_factor_);
    };
  });

  // AM : Update probability matrix to select female mate
  // given location, age and socio-behaviour of male agent
  mate_compound_category_distribution_.clear();
  mate_compound_category_distribution_.resize(no_locations_ * no_age_categories_ *
                                              no_sociobehavioural_categories_);

  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  const auto* sparam =
      param->Get<SimParam>();  // AM : Needed to get location mixing matrix
    
  for (int i = 0; i < no_locations_ * no_age_categories_ *
       no_sociobehavioural_categories_; i++) {  // Loop over male agent compound categories (location x age x socio-behaviour)
    
    // AM : Probability distribution matrix to select a female mate given male agent and female mate compound categories
    mate_compound_category_distribution_[i].resize(no_locations_ * no_age_categories_ *
                                          no_sociobehavioural_categories_);
    
    // Get Location, Age and Socio-behaviour of male agent from Index
    size_t l_i = ComputeLocationFromCompoundIndex(i);
    size_t a_i = ComputeAgeFromCompoundIndex(i);
    size_t s_i = ComputeSociobehaviourFromCompoundIndex(i);
    
    // Step 1 - Location: Compute probability to select a female mate from each location
    std::vector<float> proba_locations(no_locations_,0.0);
    float sum_locations = 0.0;
    for (size_t l_j = 0; l_j < no_locations_; l_j++){
        proba_locations[l_j] = sparam->location_mixing_matrix[l_i][l_j]*GetNumAgentsAtLocation(l_j);
        sum_locations += proba_locations[l_j];
    }
    // Normalise to get probability between 0 and 1
    if (sum_locations > 0){
        for (size_t l_j = 0; l_j < no_locations_; l_j++){
            proba_locations[l_j] /= sum_locations;
        }
    }
    // DEBUG
    /*std::cout << "Probability to select from locations, given male location " << l_i << std::endl;
    for (int l = 0; l < proba_locations.size(); l++){
          std::cout << proba_locations[l] << ", ";
    }
    std::cout << std::endl;*/
    // END DEBUG
    
    // Step 2 -  Age: Compute probability to select a female mate from each age category given the selected location
    std::vector<std::vector<float>> proba_ages_given_location;
    proba_ages_given_location.resize(no_locations_);
    for (size_t l_j = 0; l_j < no_locations_; l_j++){ // Loop over potential locations of female mate
        proba_ages_given_location[l_j].resize(no_age_categories_);
        float sum_ages = 0.0;
        for (size_t a_j = 0; a_j < no_age_categories_; a_j++){ // For each location l_j, compute probability to select a female mate from each age category a_j
            proba_ages_given_location[l_j][a_j] = sparam->age_mixing_matrix[a_i][a_j]*GetNumAgentsAtLocationAge(l_j,a_j);
            sum_ages += proba_ages_given_location[l_j][a_j];
        }
        // Normalise to compute probability between 0 and 1 to select from each age category given a location
        if (sum_ages > 0){
            for (size_t a_j = 0; a_j < no_age_categories_; a_j++){
                proba_ages_given_location[l_j][a_j] /=sum_ages;
            }
        }
    }
    // DEBUG
    /*std::cout << "Probability to select from age categories, given male age " << a_i << " and female locations "  << std::endl;
    for (int l = 0; l < proba_ages_given_location.size(); l++){
        for (int a = 0; a < proba_ages_given_location[l].size(); a++){
            std::cout << proba_ages_given_location[l][a] << ", ";
        }
        std::cout << std::endl;
    }*/
    // END DEBUG
      
    // Step 3 - Socio-behaviour : Compute probability to select from each socio-behavioural category given the selected location and age
    std::vector<std::vector<std::vector<float>>> proba_socio_given_location_age;
    proba_socio_given_location_age.resize(no_locations_);
    for (size_t l_j = 0; l_j < no_locations_; l_j++){
        proba_socio_given_location_age[l_j].resize(no_age_categories_);
        for (size_t a_j = 0; a_j < no_age_categories_; a_j++){
            proba_socio_given_location_age[l_j][a_j].resize(no_sociobehavioural_categories_);
            float sum_socio = 0.0;
            for (size_t s_j = 0; s_j < no_sociobehavioural_categories_; s_j++){
                proba_socio_given_location_age[l_j][a_j][s_j] = sparam->sociobehav_mixing_matrix[s_i][s_j]*GetNumAgentsAtIndex(l_j,a_j,s_j);
                sum_socio += proba_socio_given_location_age[l_j][a_j][s_j];
                
            }
            // Normalise to compute probability between 0 and 1 to select each socio-behaviour given location and age
            if (sum_socio > 0){
                for (size_t s_j = 0; s_j < no_sociobehavioural_categories_; s_j++){
                    proba_socio_given_location_age[l_j][a_j][s_j] /=sum_socio;
                }
            }
        }
    }
    // DEBUG
    /*std::cout << "Probability to select from sb, given male sb " << s_i << ", and female locations and ages "  << std::endl;
    for (int l = 0; l < proba_socio_given_location_age.size(); l++){
        for (int a = 0; a < proba_socio_given_location_age[l].size(); a++){
            for (int s = 0; s < proba_socio_given_location_age[l][a].size(); s++){
                std::cout << proba_socio_given_location_age[l][a][s] << ", ";
            }
            std::cout << std::endl;
        }
    }*/
    // END DEBUG
      
    // Compute the final probability that a male agent of compound category i, selects a female mate of compound category j.
    for (size_t j = 0; j < no_locations_* no_age_categories_ * no_sociobehavioural_categories_; j++){
        size_t l_j = ComputeLocationFromCompoundIndex(j);
        size_t a_j = ComputeAgeFromCompoundIndex(j);
        size_t s_j = ComputeSociobehaviourFromCompoundIndex(j);

        mate_compound_category_distribution_[i][j] = proba_locations[l_j] * proba_ages_given_location[l_j][a_j] * proba_socio_given_location_age[l_j][a_j][s_j];
        
        // Compute Cumulative distribution
        if (j > 0) {
            mate_compound_category_distribution_[i][j] +=
            mate_compound_category_distribution_[i][j - 1];
        }
    }
    // DEBUG
    /*std::cout << "CUMULATIVE Probability to select from female compound categories given male category " << i << std::endl;
    for (int j = 0; j <  no_locations_* no_age_categories_ * no_sociobehavioural_categories_; j++){
        std::cout << mate_compound_category_distribution_[i][j] << ", ";
    }
    std::cout << std::endl;*/
    // END DEBUG
    
    // Make sure that the commulative probability distribution actually ends
    // with 1.0 and not 0.9999x or something similar. Fix not only the last element but also all the previous ones that had the same cumulative probability ~1 (<=> probability = 0)
    size_t no_compound_categories = no_locations_* no_age_categories_ * no_sociobehavioural_categories_;
    auto last_cumul_proba = mate_compound_category_distribution_[i][no_compound_categories - 1];
    // Go looking backward
    for (size_t j = no_compound_categories - 1; j>=0; j--){
        if (mate_compound_category_distribution_[i][j] == last_cumul_proba){
            mate_compound_category_distribution_[i][j] = 1.0;
        } else {
            break;
        }
      }
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
  assert(location >=0 and location < no_locations_);
  assert(age >=0 and age < no_age_categories_);
  assert(sb >=0 and sb < no_sociobehavioural_categories_);

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
  if (female_agents_[compound_index].GetNumAgents() == 0){
    Log::Fatal("CategoricalEnvironment::GetRamdomAgentFromIndex()",
               "Female agents empty. Received compound index: ",
               compound_index, " (loc ", location, ", age ", age, ", sb ", sb,
               ")");
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

size_t CategoricalEnvironment::GetNumAgentsAtLocationAge(size_t location, size_t age) {
    size_t sum = 0;
    for (size_t sb = 0; sb < no_sociobehavioural_categories_; sb++){
        size_t compound_index = ComputeCompoundIndex(location, age, sb);
        assert(compound_index < female_agents_.size());
        sum += female_agents_[compound_index].GetNumAgents();
    }
    return sum;
}

size_t CategoricalEnvironment::GetNumAgentsAtLocation(size_t location) {
    size_t sum = 0;
    for (size_t sb = 0; sb < no_sociobehavioural_categories_; sb++){
        for (size_t age = 0; age < no_age_categories_; age++){
            size_t compound_index = ComputeCompoundIndex(location, age, sb);
            assert(compound_index < female_agents_.size());
            sum += female_agents_[compound_index].GetNumAgents();
        }
    }
    return sum;
}

const std::vector<float>& CategoricalEnvironment::GetMateCompoundCategoryDistribution(size_t loc, size_t age_category, size_t sociobehav) {
  size_t compound_index = ComputeCompoundIndex(loc,age_category,sociobehav);
  return mate_compound_category_distribution_[compound_index];
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
