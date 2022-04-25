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
#include "core/algorithm.h"

namespace bdm {
namespace hiv_malawi {

////////////////////////////////////////////////////////////////////////////////
// AgentVector
////////////////////////////////////////////////////////////////////////////////
AgentVector::AgentVector() {
  auto* tinfo = ThreadInfo::GetInstance();
  agents_.resize(tinfo->GetMaxThreads());
  offsets_.resize(tinfo->GetMaxThreads() + 1);
  size_ = 0;
}

AgentVector::AgentVector(const AgentVector& other)
    : agents_(other.agents_),
      offsets_(other.offsets_),
      tinfo_(other.tinfo_),
      size_(other.size_.load()),
      dirty_(other.dirty_) {}

AgentPointer<Person> AgentVector::GetRandomAgent() {
  if (size_ == 0) {
    Log::Fatal("AgentVector::GetRandomAgent()",
               "There are no agents available in one of your "
               "locations or compound categories. Consider increasing the "
               "number of Agents.");
  }
  auto* r = Simulation::GetActive()->GetRandom();
  return GetAgentAtIndex(r->Integer(size_ - 1));
}

AgentPointer<Person> AgentVector::GetAgentAtIndex(size_t i) {
  if (i >= size_) {
    Log::Fatal("AgentVector::GetAgentAtIndex()", "Given index ", i,
               "; agents_.size() ", agents_.size(), ".");
  }
  if (dirty_) {
    UpdateOffsets();
  }
  auto idx = BinarySearch(i, offsets_, 0u, offsets_.size() - 1);
  auto offset = i - offsets_[idx];

  assert(idx < agents_.size());
  assert(offset < agents_[idx].size());
  return agents_[idx][offset];
}

void AgentVector::AddAgent(AgentPointer<Person> agent) {
  auto tid = tinfo_->GetMyThreadId();
  if (agents_[tid].capacity() == agents_[tid].size()) {
    auto new_cap = std::max(static_cast<uint64_t>(1000u),
                            static_cast<uint64_t>(agents_[tid].size() * 1.2));
    agents_[tid].reserve(new_cap);
  }
  agents_[tid].push_back(agent);
  size_++;
  dirty_ = true;
}

void AgentVector::Clear() {
  for (auto& el : agents_) {
    el.clear();
  }
  size_ = 0;
  for (auto& el : offsets_) {
    el = 0;
  }
  dirty_ = false;
}

void AgentVector::UpdateOffsets() {
  std::lock_guard<Spinlock> guard(lock_);
  if (dirty_) {
    for (uint64_t i = 0; i < agents_.size(); ++i) {
      offsets_[i] = agents_[i].size();
    }
    ExclusivePrefixSum(&offsets_, offsets_.size() - 1);
    dirty_ = false;
  }
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
      casual_female_agents_(no_age_categories * no_locations *
                            no_sociobehavioural_categories),
      regular_female_agents_(no_age_categories * no_locations *
                             no_sociobehavioural_categories),
      casual_male_agents_(no_age_categories * no_locations *
                          no_sociobehavioural_categories),
      regular_male_agents_(no_age_categories * no_locations *
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
  for (auto& el : casual_female_agents_) {
    el.Clear();
  }
  casual_female_agents_.resize(no_age_categories_ * no_locations_ *
                               no_sociobehavioural_categories_);

  for (auto& el : regular_female_agents_) {
    el.Clear();
  }
  regular_female_agents_.resize(no_age_categories_ * no_locations_ *
                                no_sociobehavioural_categories_);

  for (auto& el : casual_male_agents_) {
    el.Clear();
  }
  casual_male_agents_.resize(no_age_categories_ * no_locations_ *
                             no_sociobehavioural_categories_);

  for (auto& el : regular_male_agents_) {
    el.Clear();
  }
  regular_male_agents_.resize(no_age_categories_ * no_locations_ *
                              no_sociobehavioural_categories_);
  for (auto& el : adults_) {
    el.Clear();
  }
  adults_.resize(no_locations_);
  // DEBUG
  /*if (iter < 4) {
     std::cout << "After clearing section" << std::endl;
     DescribePopulation();
  }*/

  // Index females (by location x age x sociobehaviour for casual and regular
  // partnerships), and adults (by location for location attractivity)
  auto* rm = Simulation::GetActive()->GetResourceManager();
  auto assign_to_indices = L2F([](Agent* agent) {
    auto* env = bdm_static_cast<CategoricalEnvironment*>(
        Simulation::GetActive()->GetEnvironment());
    auto* person = bdm_static_cast<Person*>(agent);
    if (person == nullptr) {
      Log::Fatal("CategoricalEnvironment::UpdateImplementation()",
                 "person is nullptr");
    }

    // Reset number of casual partners at the beginning of every year
    // person->no_casual_partners_ = 0;

    // Adults
    if (person->age_ >= env->GetMinAge()) {
      AgentPointer<Person> person_ptr = person->GetAgentPtr<Person>();
      if (person_ptr == nullptr) {
        Log::Fatal("CategoricalEnvironment::UpdateImplementation()",
                   "person_ptr is nullptr");
      }
      // Under max_age_
      if (person->age_ <= env->GetMaxAge()) {
        // Compute age category of agent
        size_t age_category =
            person->GetAgeCategory(env->GetMinAge(), env->GetNoAgeCategories());
        // Adult women under max_age_ are potential casual partners
        if (person->sex_ == Sex::kFemale) {
          // Add female agent to the right index, based on her location, age
          // category and socio-behavioural category
          env->AddCasualFemaleToIndex(person_ptr, person->location_,
                                      age_category,
                                      person->social_behaviour_factor_);
        } else {
          // Adult male under max_age_ are potential casual partners
          // Add male agent to the right index, based on his location, age
          // category and socio-behavioural category
          env->AddCasualMaleToIndex(person_ptr, person->location_, age_category,
                                    person->social_behaviour_factor_);
        }
      }
      // Adult single women are potential regular partners
      if (person->sex_ == Sex::kFemale && person->hasPartner() == false) {
        // Compute age category of female agent
        size_t age_category =
            person->GetAgeCategory(env->GetMinAge(), env->GetNoAgeCategories());
        // Add female agent to the right index, based on her location, age
        // category and socio-behavioural category
        env->AddRegularFemaleToIndex(person_ptr, person->location_,
                                     age_category,
                                     person->social_behaviour_factor_);
      }
      // Index adults by location (for location attractivity)
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
  rm->ForEachAgentParallel(assign_to_indices);

  // During first iteration, assign mothers to children
  // Note: Ignore for parallelization because it is only executed once at the
  // beginning of the simulation -> setup cost.
  if (!mothers_are_assiged_) {
    mothers_are_assiged_ = true;
    uint64_t iter =
        Simulation::GetActive()->GetScheduler()->GetSimulatedSteps();
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

      // TO DO AM: Change to MaxAgeBirth
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
        // TO DO AM: ideally, mother is at least 15 and at most 40 years older
        // than child
        person->mother_ = env->GetRandomMotherFromLocation(person->location_);
        if (!person->mother_) {
          return;
        }
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

  auto* sim = Simulation::GetActive();  // AM: Needed to get current iteration
  const auto* sparam =
      sim->GetParam()->Get<SimParam>();  // AM : Needed to get mixing matrices
  auto* random = sim->GetRandom();       // : Needed for sampling

  // Regular Partnership Updates
  // AM : Update probability matrix to select regular female partner
  // given location, age and socio-behaviour of male agent
  UpdateRegularPartnerCategoryDistribution(
      sparam->reg_partner_age_mixing_matrix,
      sparam->reg_partner_sociobehav_mixing_matrix);
  // AM: Select potential regular partner's category for each adult single man
  auto choose_regular_partner_category = L2F([&](Agent* agent) {
    auto* env = bdm_static_cast<CategoricalEnvironment*>(
        Simulation::GetActive()->GetEnvironment());
    auto* person = bdm_static_cast<Person*>(agent);
    if (person == nullptr) {
      Log::Fatal("CategoricalEnvironment::UpdateImplementation()",
                 "person is nullptr");
    }
    if (person->sex_ == Sex::kMale && person->IsAdult() &&
        !person->hasPartner() && person->seek_regular_partnership_ == true) {
      AgentPointer<Person> person_ptr = person->GetAgentPtr<Person>();
      // Compute man's compound category
      size_t age_category =
          person->GetAgeCategory(env->GetMinAge(), env->GetNoAgeCategories());
      size_t man_compound_index = ComputeCompoundIndex(
          person->location_, age_category, person->social_behaviour_factor_);
      // Get man's partner category distribution
      const auto& partner_category_distribution =
          reg_partner_compound_category_distribution_[man_compound_index];
      // Sample regular partner's category
      float rand_num = static_cast<float>(random->Uniform());
      bool indexed = false;
      for (size_t j = 0; j < partner_category_distribution.size(); j++) {
        if (rand_num <= partner_category_distribution[j]) {
          env->AddRegularMaleToIndex(person_ptr, j);
          indexed = true;
          break;
        }
      }
      if (!indexed) {
        // This line of code should never be reached
        Log::Warning("UpdateImplementation()",
                     "Could not sample the category of regular partner. "
                     "Recieved inputs: ",
                     rand_num);
      }
    }
  });

  rm->ForEachAgentParallel(choose_regular_partner_category);

  // AM: Map regular partners for each compound category
#pragma omp parallel for
  for (size_t cat = 0; cat < regular_male_agents_.size(); cat++) {
    size_t no_males = regular_male_agents_[cat].GetNumAgents();
    size_t no_females = regular_female_agents_[cat].GetNumAgents();
    /*std::cout << "Coumpound Category " << i << " - Number of male seeking
    partner = " << no_males <<
    ", Number of single females = " << no_females << std::endl;*/
    if (no_males > 0 && no_females > 0) {
      if (no_males < no_females) {
        // Vector of ordered female indexes
        std::vector<int> v(no_females);
        std::iota(std::begin(v), std::end(v), 0);
        // Shuffle female indexes
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(v.begin(), v.end(), g);
        // Male select Females
        for (size_t i = 0; i < no_males; i++) {
          regular_male_agents_[cat].GetAgentAtIndex(i)->SetPartner(
              regular_female_agents_[cat].GetAgentAtIndex(v[i]));
          if (regular_male_agents_[cat]
                  .GetAgentAtIndex(i)
                  ->partner_->partner_ !=
              regular_male_agents_[cat].GetAgentAtIndex(i)) {
            Log::Warning(
                "CategoricalEnvironment::UpdateImplementation()",
                "Regular Partnership (male selects female) is ASYMMETRICAL");
          }
        }
      } else {
        // Vector of ordered male indexes
        std::vector<int> v(no_males);
        std::iota(std::begin(v), std::end(v), 0);
        // Shuffle male indexes
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(v.begin(), v.end(), g);
        // Females select Males
        for (size_t i = 0; i < no_females; i++) {
          regular_female_agents_[cat].GetAgentAtIndex(i)->SetPartner(
              regular_male_agents_[cat].GetAgentAtIndex(v[i]));
          // Check Symmetry
          if (regular_female_agents_[cat]
                  .GetAgentAtIndex(i)
                  ->partner_->partner_ !=
              regular_female_agents_[cat].GetAgentAtIndex(i)) {
            Log::Warning(
                "CategoricalEnvironment::UpdateImplementation()",
                "Regular Partnership (female selects male) is ASYMMETRICAL");
          }
        }
      }
    }
  }

  // AM: Probability of migration location depends on the current year
  int year = static_cast<int>(
      sparam->start_year +
      sim->GetScheduler()->GetSimulatedSteps());  // Current year
  // If no transition year is higher than current year, then use last
  // transition year
  int year_index = sparam->migration_year_transition.size() - 1;
  for (size_t y = 0; y < sparam->migration_year_transition.size() - 1; y++) {
    if (year < sparam->migration_year_transition[y + 1]) {
      year_index = y;
      break;
    }
  }
  // AM : Update probability matrix to select migration/relocation destination
  // given current year index and origin location
  UpdateMigrationLocationProbability(year_index, sparam->migration_matrix);

  // AM : Update probability matrix to select female mate
  // given location, age and socio-behaviour of male agent
  UpdateCasualPartnerCategoryDistribution(sparam->location_mixing_matrix,
                                          sparam->age_mixing_matrix,
                                          sparam->sociobehav_mixing_matrix);
};

void CategoricalEnvironment::UpdateCasualPartnerCategoryDistribution(
    const std::vector<std::vector<float>>& location_mixing_matrix,
    const std::vector<std::vector<float>>& age_mixing_matrix,
    const std::vector<std::vector<float>>& sociobehav_mixing_matrix) {
  //#pragma omp parallel
  for (auto& el : mate_compound_category_distribution_) {
    el.clear();
  }
  mate_compound_category_distribution_.resize(
      no_locations_ * no_age_categories_ * no_sociobehavioural_categories_);

  //#pragma omp for
  for (size_t i = 0;
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
      proba_locations[l_j] =
          location_mixing_matrix[l_i][l_j] * GetNumCasualFemalesAtLocation(l_j);
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
            GetNumCasualFemalesAtLocationAge(l_j, a_j);
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
              GetNumCasualFemalesAtIndex(l_j, a_j, s_j);
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

void CategoricalEnvironment::UpdateRegularPartnerCategoryDistribution(
    std::vector<std::vector<float>> reg_partner_age_mixing_matrix,
    std::vector<std::vector<float>> reg_partner_sociobehav_mixing_matrix) {
  //#pragma omp parallel
  for (auto& el : reg_partner_compound_category_distribution_) {
    el.clear();
  }
  reg_partner_compound_category_distribution_.resize(
      no_locations_ * no_age_categories_ * no_sociobehavioural_categories_);

  //#pragma omp for
  for (size_t i = 0;
       i < no_locations_ * no_age_categories_ * no_sociobehavioural_categories_;
       i++) {  // Loop over male agent compound categories (location x age x
               // socio-behaviour)

    // AM : Probability distribution matrix to select a female regulat partner
    // given male agent and female partner compound categories
    reg_partner_compound_category_distribution_[i].resize(
        no_locations_ * no_age_categories_ * no_sociobehavioural_categories_);

    // Get Location, Age and Socio-behaviour of male agent from Index
    size_t l_i = ComputeLocationFromCompoundIndex(i);
    size_t a_i = ComputeAgeFromCompoundIndex(i);
    size_t s_i = ComputeSociobehaviourFromCompoundIndex(i);

    // Step 1 - Location: Compute probability to select a female partner from
    // each location. Regular Partners are selected from the same location.
    std::vector<float> proba_locations(no_locations_, 0.0);
    for (size_t l_j = 0; l_j < no_locations_; l_j++) {
      if (l_j == l_i) {
        proba_locations[l_j] = 1.0;
      } else {
        proba_locations[l_j] = 0.0;
      }
    }

    // Step 2 -  Age: Compute probability to select a female mate from each
    // age category given the selected location
    std::vector<std::vector<float>> proba_ages_given_location;
    proba_ages_given_location.resize(no_locations_);
    for (size_t l_j = 0; l_j < no_locations_;
         l_j++) {  // Loop over potential locations of female mate
      proba_ages_given_location[l_j].resize(no_age_categories_);

      if (l_j == l_i) {
        float sum_ages = 0.0;
        for (size_t a_j = 0; a_j < no_age_categories_;
             a_j++) {  // For each location l_j, compute probability to select a
                       // female mate from each age category a_j
          proba_ages_given_location[l_j][a_j] =
              reg_partner_age_mixing_matrix[a_i][a_j] *
              GetNumRegularFemalesAtLocationAge(l_j, a_j);
          sum_ages += proba_ages_given_location[l_j][a_j];
        }
        // Normalise to compute probability between 0 and 1 to select from each
        // age category given a location
        if (sum_ages > 0) {
          for (size_t a_j = 0; a_j < no_age_categories_; a_j++) {
            proba_ages_given_location[l_j][a_j] /= sum_ages;
          }
        }
      } else {
        for (size_t a_j = 0; a_j < no_age_categories_;
             a_j++) {  // For each location l_j, compute probability to select a
                       // female mate from each age category a_j
          proba_ages_given_location[l_j][a_j] = 0.0;
        }
      }
    }

    // Step 3 - Socio-behaviour : Compute probability to select from each
    // socio-behavioural category given the selected location and age
    std::vector<std::vector<std::vector<float>>> proba_socio_given_location_age;
    proba_socio_given_location_age.resize(no_locations_);
    for (size_t l_j = 0; l_j < no_locations_; l_j++) {
      proba_socio_given_location_age[l_j].resize(no_age_categories_);
      if (l_j == l_i) {
        for (size_t a_j = 0; a_j < no_age_categories_; a_j++) {
          proba_socio_given_location_age[l_j][a_j].resize(
              no_sociobehavioural_categories_);
          float sum_socio = 0.0;
          for (size_t s_j = 0; s_j < no_sociobehavioural_categories_; s_j++) {
            proba_socio_given_location_age[l_j][a_j][s_j] =
                reg_partner_sociobehav_mixing_matrix[s_i][s_j] *
                GetNumRegularFemalesAtIndex(l_j, a_j, s_j);
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
      } else {
        for (size_t a_j = 0; a_j < no_age_categories_; a_j++) {
          proba_socio_given_location_age[l_j][a_j].resize(
              no_sociobehavioural_categories_);
          for (size_t s_j = 0; s_j < no_sociobehavioural_categories_; s_j++) {
            proba_socio_given_location_age[l_j][a_j][s_j] = 0.0;
          }
        }
      }
    }

    // Compute the final probability that a male agent of compound category i,
    // selects a female regular partner of compound category j.
    for (size_t j = 0; j < no_locations_ * no_age_categories_ *
                               no_sociobehavioural_categories_;
         j++) {
      size_t l_j = ComputeLocationFromCompoundIndex(j);
      size_t a_j = ComputeAgeFromCompoundIndex(j);
      size_t s_j = ComputeSociobehaviourFromCompoundIndex(j);

      reg_partner_compound_category_distribution_[i][j] =
          proba_locations[l_j] * proba_ages_given_location[l_j][a_j] *
          proba_socio_given_location_age[l_j][a_j][s_j];

      // Compute Cumulative distribution
      if (j > 0) {
        reg_partner_compound_category_distribution_[i][j] +=
            reg_partner_compound_category_distribution_[i][j - 1];
      }
    }

    // Make sure that the commulative probability distribution actually ends
    // with 1.0 and not 0.9999x or something similar. Do not fix only the last
    // element but all the previous ones, which had the same cumulative
    // probability ~1 (<=> probability = 0)
    size_t no_compound_categories =
        no_locations_ * no_age_categories_ * no_sociobehavioural_categories_;
    auto last_cumul_proba =
        reg_partner_compound_category_distribution_[i]
                                                   [no_compound_categories - 1];
    // Go looking backward
    for (size_t j = no_compound_categories - 1; j >= 0; j--) {
      if (reg_partner_compound_category_distribution_[i][j] ==
          last_cumul_proba) {
        reg_partner_compound_category_distribution_[i][j] = 1.0;
      } else {
        break;
      }
    }
  }
}

void CategoricalEnvironment::UpdateMigrationLocationProbability(
    size_t year_index,
    const std::vector<std::vector<std::vector<float>>>& migration_matrix) {
  for (auto& el : migration_location_distribution_) {
    el.clear();
  }
  migration_location_distribution_.resize(no_locations_);
  for (size_t i = 0; i < no_locations_; i++) {
    migration_location_distribution_[i].resize(no_locations_);
    // Compute Denominator for Normalization
    float sum = 0.0;
    for (size_t j = 0; j < no_locations_; j++) {
      // Weight migration_matrix with population size per destination
      migration_location_distribution_[i][j] =
          migration_matrix[year_index][i][j] * GetNumAdultsAtLocation(j);
      sum += migration_location_distribution_[i][j];
    }
    // Normalize and Cumulate
    for (size_t j = 0; j < no_locations_; j++) {
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
    auto last_cumul_proba =
        migration_location_distribution_[i][no_locations_ - 1];
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

void CategoricalEnvironment::AddAdultToLocation(AgentPointer<Person> agent,
                                                size_t location) {
  assert(location >= 0 and location < no_locations_);

  if (location >= adults_.size()) {
    Log::Fatal("CategoricalEnvironment::AddAdultToLocation()",
               "Location index is out of bounds. Received (loc ", location,
               ") and adults_.size(): ", adults_.size());
  }
  adults_[location].AddAgent(agent);
}

void CategoricalEnvironment::AddCasualFemaleToIndex(AgentPointer<Person> agent,
                                                    size_t location, size_t age,
                                                    size_t sb) {
  assert(location >= 0 and location < no_locations_);
  assert(age >= 0 and age < no_age_categories_);
  assert(sb >= 0 and sb < no_sociobehavioural_categories_);

  size_t compound_index = ComputeCompoundIndex(location, age, sb);
  if (compound_index >= casual_female_agents_.size()) {
    Log::Fatal(
        "CategoricalEnvironment::AddCasualFemaleToIndex()",
        "Location index is out of bounds. Received compound index: ",
        compound_index, " (loc ", location, ", age ", age, ", sb ", sb,
        ") casual_female_agents_.size(): ", casual_female_agents_.size());
  }
  casual_female_agents_[compound_index].AddAgent(agent);
};

void CategoricalEnvironment::AddRegularFemaleToIndex(AgentPointer<Person> agent,
                                                     size_t location,
                                                     size_t age, size_t sb) {
  assert(location >= 0 and location < no_locations_);
  assert(age >= 0 and age < no_age_categories_);
  assert(sb >= 0 and sb < no_sociobehavioural_categories_);

  size_t compound_index = ComputeCompoundIndex(location, age, sb);
  if (compound_index >= regular_female_agents_.size()) {
    Log::Fatal(
        "CategoricalEnvironment::AddRegularFemaleToIndex()",
        "Location index is out of bounds. Received compound index: ",
        compound_index, " (loc ", location, ", age ", age, ", sb ", sb,
        ") regular_female_agents_.size(): ", regular_female_agents_.size());
  }
  regular_female_agents_[compound_index].AddAgent(agent);
};

void CategoricalEnvironment::AddRegularMaleToIndex(AgentPointer<Person> agent,
                                                   size_t index) {
  if (index >= regular_male_agents_.size()) {
    Log::Fatal(
        "CategoricalEnvironment::AddRegularMaleToIndex()",
        "Compound index is out of bounds. Received compound index: ", index,
        " and regular_male_agents_.size(): ", regular_male_agents_.size());
  }
  regular_male_agents_[index].AddAgent(agent);
};

void CategoricalEnvironment::AddCasualMaleToIndex(AgentPointer<Person> agent,
                                                  size_t location, size_t age,
                                                  size_t sb) {
  assert(location >= 0 and location < no_locations_);
  assert(age >= 0 and age < no_age_categories_);
  assert(sb >= 0 and sb < no_sociobehavioural_categories_);

  size_t compound_index = ComputeCompoundIndex(location, age, sb);
  if (compound_index >= casual_male_agents_.size()) {
    Log::Fatal("CategoricalEnvironment::AddCasualMaleToIndex()",
               "Location index is out of bounds. Received compound index: ",
               compound_index, " (loc ", location, ", age ", age, ", sb ", sb,
               ") casual_male_agents_.size(): ", casual_male_agents_.size());
  }
  casual_male_agents_[compound_index].AddAgent(agent);
};

void CategoricalEnvironment::AddMotherToLocation(AgentPointer<Person> agent,
                                                 size_t location) {
  assert(location >= 0 and location < no_locations_);
  mothers_[location].AddAgent(agent);
}

AgentPointer<Person> CategoricalEnvironment::GetRandomCasualFemaleFromIndex(
    size_t location, size_t age, size_t sb) {
  size_t compound_index = ComputeCompoundIndex(location, age, sb);
  if (compound_index >= casual_female_agents_.size()) {
    Log::Fatal(
        "CategoricalEnvironment::GetRandomCasualFemaleFromIndex()",
        "Location index is out of bounds. Received compound index: ",
        compound_index, " (loc ", location, ", age ", age, ", sb ", sb,
        ") casual_female_agents_.size(): ", casual_female_agents_.size());
  }
  return casual_female_agents_[compound_index].GetRandomAgent();
};

AgentPointer<Person> CategoricalEnvironment::GetRandomCasualFemaleFromIndex(
    size_t compound_index) {
  size_t location = ComputeLocationFromCompoundIndex(compound_index);
  size_t age = ComputeAgeFromCompoundIndex(compound_index);
  size_t sb = ComputeSociobehaviourFromCompoundIndex(compound_index);

  if (compound_index >= casual_female_agents_.size()) {
    Log::Fatal(
        "CategoricalEnvironment::GetRandomCasualFemaleFromIndex()",
        "Location index is out of bounds. Received compound index: ",
        compound_index, " (loc ", location, ", age ", age, ", sb ", sb,
        ") casual_female_agents_.size(): ", casual_female_agents_.size());
  }
  if (casual_female_agents_[compound_index].GetNumAgents() == 0) {
    Log::Fatal("CategoricalEnvironment::GetRandomCasualFemaleFromIndex()",
               "Female agents empty. Received compound index: ", compound_index,
               " (loc ", location, ", age ", age, ", sb ", sb, ")");
  }
  return casual_female_agents_[compound_index].GetRandomAgent();
};

// Function for Debug - prints number of females per location.
void CategoricalEnvironment::DescribePopulation() {
  size_t total_population{0};
  std::cout << "\n ### population (Casual female partners) description ### \n";
  std::cout << "SB | location | age |  number of humans in index\n";
  for (size_t s = 0; s < no_sociobehavioural_categories_; s++) {
    for (size_t l = 0; l < no_locations_; l++) {
      for (size_t a = 0; a < no_age_categories_; a++) {
        auto num_agents = GetNumCasualFemalesAtIndex(l, a, s);
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

size_t CategoricalEnvironment::GetNumCasualFemalesAtIndex(size_t location,
                                                          size_t age,
                                                          size_t sb) {
  size_t compound_index = ComputeCompoundIndex(location, age, sb);
  assert(compound_index < casual_female_agents_.size());
  return casual_female_agents_[compound_index].GetNumAgents();
}

size_t CategoricalEnvironment::GetNumRegularFemalesAtIndex(size_t location,
                                                           size_t age,
                                                           size_t sb) {
  size_t compound_index = ComputeCompoundIndex(location, age, sb);
  assert(compound_index < regular_female_agents_.size());
  return regular_female_agents_[compound_index].GetNumAgents();
}

size_t CategoricalEnvironment::GetNumAdultsAtLocation(size_t location) {
  assert(location < adults_.size());
  return adults_[location].GetNumAgents();
}

size_t CategoricalEnvironment::GetNumCasualFemalesAtLocationAge(size_t location,
                                                                size_t age) {
  size_t sum = 0;
  for (size_t sb = 0; sb < no_sociobehavioural_categories_; sb++) {
    size_t compound_index = ComputeCompoundIndex(location, age, sb);
    assert(compound_index < casual_female_agents_.size());
    sum += casual_female_agents_[compound_index].GetNumAgents();
  }
  return sum;
}

size_t CategoricalEnvironment::GetNumRegularFemalesAtLocationAge(
    size_t location, size_t age) {
  size_t sum = 0;
  for (size_t sb = 0; sb < no_sociobehavioural_categories_; sb++) {
    size_t compound_index = ComputeCompoundIndex(location, age, sb);
    assert(compound_index < regular_female_agents_.size());
    sum += regular_female_agents_[compound_index].GetNumAgents();
  }
  return sum;
}

size_t CategoricalEnvironment::GetNumCasualFemalesAtLocation(size_t location) {
  size_t sum = 0;
  for (size_t sb = 0; sb < no_sociobehavioural_categories_; sb++) {
    for (size_t age = 0; age < no_age_categories_; age++) {
      size_t compound_index = ComputeCompoundIndex(location, age, sb);
      assert(compound_index < casual_female_agents_.size());
      sum += casual_female_agents_[compound_index].GetNumAgents();
    }
  }
  return sum;
}

size_t CategoricalEnvironment::GetNumRegularFemalesAtLocation(size_t location) {
  size_t sum = 0;
  for (size_t sb = 0; sb < no_sociobehavioural_categories_; sb++) {
    for (size_t age = 0; age < no_age_categories_; age++) {
      size_t compound_index = ComputeCompoundIndex(location, age, sb);
      assert(compound_index < regular_female_agents_.size());
      sum += regular_female_agents_[compound_index].GetNumAgents();
    }
  }
  return sum;
}

// AM: GET Random mother from location
AgentPointer<Person> CategoricalEnvironment::GetRandomMotherFromLocation(
    size_t location) {
  if (mothers_[location].GetNumAgents() == 0) {
    Log::Warning("CategoricalEnvironment::GetRandomMotherFromLocation()",
                 "Mothers empty. Received location: ", location);
    return nullptr;
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

const std::vector<float>& CategoricalEnvironment::GetMigrationLocDistribution(
    size_t loc) {
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

}  // namespace hiv_malawi
}  // namespace bdm
