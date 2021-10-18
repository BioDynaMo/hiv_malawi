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

#ifndef PERSON_BEHAVIOR_H_
#define PERSON_BEHAVIOR_H_

#include "categorical-environment.h"
#include "datatypes.h"
#include "person.h"
#include "population-initialization.h"

namespace bdm {

////////////////////////////////////////////////////////////////////////////////
// BioDynaMo's Agent / Individual Behaviors
////////////////////////////////////////////////////////////////////////////////

// A behavior that allows agents to randomly migrate between the categorical
// loactions. It uses a gausion random process to determine the next location.
struct RandomMigration : public Behavior {
  BDM_BEHAVIOR_HEADER(RandomMigration, Behavior, 1);

  RandomMigration() {}

  void Run(Agent* agent) override {
    auto* sim = Simulation::GetActive();
    auto* random = sim->GetRandom();
    auto* person = bdm_static_cast<Person*>(agent);
    auto* param = sim->GetParam();
    const auto* sparam = param->Get<SimParam>();

    int migration_direction = static_cast<int>(
        random->Gaus(sparam->migration_mean, sparam->migration_sigma));
    if (person->location_ + migration_direction < 0) {
      person->location_ += Location::kLocLast + migration_direction;
    } else if (person->location_ + migration_direction >= Location::kLocLast) {
      person->location_ += migration_direction - Location::kLocLast;
    } else {
      person->location_ += migration_direction;
    }
  }
};

// This is the mating and therefore also the infection behaviour. The Behavior
// is only executed by male agents. For each male agent, we determine the
// number of sex partners with a gaussian random process and randomly select
// from all available females at his location. For each contact, HIV (if
// present) can be transmitted from the infected to the healthy individual with
// a certain probability.
struct MatingBehaviour : public Behavior {
  BDM_BEHAVIOR_HEADER(MatingBehaviour, Behavior, 1);

  MatingBehaviour() {}

  int SampleCompoundCategory(float rand_num,
                       const std::vector<float>& category_distribution) {
      for (size_t i = 0; i < category_distribution.size(); i++) {
        if (rand_num <= category_distribution[i]) {
          return i;
        }
      }

      // This line of code should never be reached
      std::cout << std::endl;
      Log::Warning("SampleCompoundCategory()",
                   "Could not sample the category. Recieved inputs: ", rand_num,
                   ". Use location 0.");
      return 0;
  }
    
  void Run(Agent* agent) override {
    auto* sim = Simulation::GetActive();
    auto* env = bdm_static_cast<CategoricalEnvironment*>(sim->GetEnvironment());
    auto* random = sim->GetRandom();
    auto* param = sim->GetParam();
    const auto* sparam = param->Get<SimParam>();
    auto* person = bdm_static_cast<Person*>(agent);

    // Randomly determine the number of mates
    int no_mates = static_cast<int>(
        random->Gaus(sparam->no_mates_mean, sparam->no_mates_sigma));

    // This part is only executed for male persons in a certain age group, since
    // the infection goes into both directions.
    if (no_mates > 0 && person->sex_ == Sex::kMale &&
        person->age_ > env->GetMinAge() && person->age_ <= env->GetMaxAge()) {
      
      // Compute male agent's age category
      size_t age_category = person->GetAgeCategory(env->GetMinAge(),env->GetNoAgeCategories());
      // Get (cumulative) probability distribution that the male agent selects a female mate from each compound category
      const std::vector<float> mate_compound_category_distribution =
            env->GetMateCompoundCategoryDistribution(person->location_,age_category,person->social_behaviour_factor_);

      for (int i = 0; i < no_mates; i++) {
        // AM: select compound category of mate
        float rand_num = static_cast<float>(random->Uniform());
        
        size_t mate_compound_category =
            SampleCompoundCategory(rand_num, mate_compound_category_distribution);

        // AM: Choose a random female mate at the selected mate compound category (location, age group
        // and sociobehavioral category
        AgentPointer<Person> mate =
            env->GetRamdomAgentFromIndex(mate_compound_category);
        
        // DEBUG: Increase count of partners from given locations
        //env->IncreaseCountMatesInLocations(person->location_,mate_location);
            
        if (mate == nullptr) {
          Log::Fatal("MatingBehaviour()",
                     "Received nullptr as AgentPointer mate.");
        }
        // Scenario healthy male has intercourse with infected acute female
        if (mate->state_ == GemsState::kAcute &&
            person->state_ == GemsState::kHealthy &&
            random->Uniform() < sparam->infection_probability_acute_fm) {
          person->state_ = GemsState::kAcute;
        }
        // Scenario healthy male has intercourse with infected chronic female
        else if (mate->state_ == GemsState::kChronic &&
                 person->state_ == GemsState::kHealthy &&
                 random->Uniform() < sparam->infection_probability_chronic_fm) {
          person->state_ = GemsState::kAcute;
        }
        // Scenario healthy male has intercourse with infected treated female
        else if (mate->state_ == GemsState::kTreated &&
                 person->state_ == GemsState::kHealthy &&
                 random->Uniform() < sparam->infection_probability_treated_fm) {
          person->state_ = GemsState::kAcute;
        }
        // Scenario healthy male has intercourse with infected failing treatment
        // female
        else if (mate->state_ == GemsState::kFailing &&
                 person->state_ == GemsState::kHealthy &&
                 random->Uniform() < sparam->infection_probability_failing_fm) {
          person->state_ = GemsState::kAcute;
        }
        // Scenario infected acute male has intercourse with healthy female
        else if (mate->state_ == GemsState::kHealthy &&
                 person->state_ == GemsState::kAcute &&
                 random->Uniform() < sparam->infection_probability_acute_mf) {
          mate->state_ = GemsState::kAcute;
        }  // Scenario infected chronic male has intercourse with healthy female
        else if (mate->state_ == GemsState::kHealthy &&
                 person->state_ == GemsState::kChronic &&
                 random->Uniform() < sparam->infection_probability_chronic_mf) {
          mate->state_ = GemsState::kAcute;
        }  // Scenario infected treated male has intercourse with healthy female
        else if (mate->state_ == GemsState::kHealthy &&
                 person->state_ == GemsState::kTreated &&
                 random->Uniform() < sparam->infection_probability_treated_mf) {
          mate->state_ = GemsState::kAcute;
        }  // Scenario infected failing treatment male has intercourse with
           // healthy female
        else if (mate->state_ == GemsState::kHealthy &&
                 person->state_ == GemsState::kFailing &&
                 random->Uniform() < sparam->infection_probability_failing_mf) {
          mate->state_ = GemsState::kAcute;
        } else {
          ;  // if both are infected or both are healthy, do nothing
        }
      }
    }
  }
};

// The GetOlder behavior describes all things that happen to an agent while
// getting older such as for instance having a greater chance to die.
struct GetOlder : public Behavior {
  BDM_BEHAVIOR_HEADER(GetOlder, Behavior, 1);

  GetOlder() {}

  // AM TO DO: DO NOT DEFINE PARAMETERS HERE BUT IN sim-param.h
  float get_mortality_rate_age(float age) {
    if (age < 15) {
      return 0.01;
    } else if (age < 50) {
      return 0.005;
    } else if (age < 90) {
      return 0.05;
    } else {
      return 1.0;
    }
  }

  float get_mortality_rate_hiv(int state) {
    if (state == GemsState::kChronic) {
      return 0.05;
    } else if (state == GemsState::kTreated) {
      return 0.01;
    } else if (state == GemsState::kFailing) {
      return 0.1;
    } else {
      return 0.0;
    }
  }

  void Run(Agent* agent) override {
    auto* sim = Simulation::GetActive();
    auto* random = sim->GetRandom();
    auto* param = sim->GetParam();
    const auto* sparam = param->Get<SimParam>();
    auto* person = bdm_static_cast<Person*>(agent);

    // If between min_age and max_age, reassign risk factors
    if (person->age_ >= sparam->min_age &&
        person->age_ <= sparam->max_age) {  // AM corrected typo sparam->min_age
                                            // <= sparam->max_age

      // Update risk factors stochastically like in initialization
      if (random->Uniform() < sparam->sociobehavioural_risk_probability) {
        person->social_behaviour_factor_ = 0;
      } else {
        person->social_behaviour_factor_ = 1;
      }
      if (random->Uniform() < sparam->biomedical_risk_probability) {
        person->biomedical_factor_ = 0;
      } else {
        person->biomedical_factor_ = 1;
      }
    } else {
      person->social_behaviour_factor_ = 0;
      person->biomedical_factor_ = 0;
    }

    // AM: HIV state transition, depending on current year and population
    // category (important for transition to treatment)
    int year_population_category = -1;
    double year =
        static_cast<double>(1960 + sim->GetScheduler()->GetSimulatedSteps());

    if (year < 2003) {  // Prior to 2003
      year_population_category =
          0;  // All (No difference in ART between people. ART not available.)
    } else if (year < 2011) {  // Between 2003 and 2010
      if (person->sex_ == Sex::kFemale and person->age_ >= 18 and
          person->age_ <= 40) {
        year_population_category = 1;  // Female between 18 and 40
      } else if (person->age_ < 15) {
        year_population_category = 2;  // Child
      } else {
        year_population_category =
            3;  // Others (Male, Female under 18, and Female over 40)
      }
    } else {  // After 2011
      if (person->sex_ == Sex::kFemale and person->age_ >= 18 and
          person->age_ <= 40) {
        year_population_category = 4;  // Female between 18 and 40
      } else if (person->age_ < 15) {
        year_population_category = 5;  // Child
      } else {
        year_population_category =
            6;  // Others (Male, Female under 18, and Female over 40)
      }
    }
    std::vector<float> transition_proba =
        sparam->hiv_transition_matrix[person->state_][year_population_category];
    for (size_t i = 0; i < transition_proba.size(); i++) {
      if (random->Uniform() < transition_proba[i]) {
        person->state_ = i;
        break;
      }
    }

    // Possibly die - if not, just get older
    bool stay_alive{true};
    // Let's assume a linear increase of the death probability per year for
    // healty agents.
    /*if (person->state_ == GemsState::kHealthy) {
      if (random->Uniform() <
          (person->age_ - sparam->min_healthy) /
              (sparam->delta_healthy * sparam->alpha_healthy)) {
        stay_alive = false;
      }
    }
    // Let's assume a linear increase of the death probability per year for
    // non-healty agents.
    if (person->state_ != GemsState::kHealthy) {
      if (random->Uniform() < (person->age_ - sparam->min_hiv) /
                                  (sparam->delta_hiv * sparam->alpha_hiv)) {
        stay_alive = false;
      }
    }
    // hard cut at a certain age
    if (person->age_ >= sparam->age_of_death) {
      stay_alive = false;
    }*/

    // AM: Mortality
    // HIV-related mortality
    float rand_num_hiv = static_cast<float>(random->Uniform());
    if (rand_num_hiv < get_mortality_rate_hiv(person->state_)) {
      stay_alive = false;
    }
    // Age-related mortality
    float rand_num_age = static_cast<float>(random->Uniform());
    if (rand_num_age < get_mortality_rate_age(person->age_)) {
      stay_alive = false;
    }

    if (!stay_alive) {
      // Person dies, i.e. is removed from simulation.
      person->RemoveFromSimulation();
    } else {
      // increase age
      person->age_ += 1;
    }
  }
};

// The GiveBirth behavior is assigned to all female agents. If a female is in a
// certain age range, she can give birth to a child that is located at the same
// place. If she is HIV positive, there is a certain chance to infect the child
// while giving birth.
struct GiveBirth : public Behavior {
  BDM_BEHAVIOR_HEADER(GiveBirth, Behavior, 1);

  GiveBirth() {}

  // Helper function to create a single child
  Person* CreateChild(Random* random_generator, Person* mother,
                      const SimParam* sparam) {
    // Create new child
    Person* child = new Person();
    // Assign sex
    child->sex_ =
        SampleSex(random_generator->Uniform(), sparam->probability_male);
    // Assign age - possibly -1 ?
    child->age_ = random_generator->Uniform();
    // Assign location
    child->location_ = mother->location_;
    // Compute risk factors
    child->social_behaviour_factor_ = 0;
    child->biomedical_factor_ = 0;
    // Stores the current GemsState of the child.
    if (mother->state_ == GemsState::kHealthy) {
      child->state_ = GemsState::kHealthy;

      ///! The aguments below are currently either not used or repetitive.
      // // Store the year when the agent got infected
      // child->year_of_infection_ = std::numeric_limits<float>::max();
    }
    // let's assume that if a mother is HIV positive, the child will be HIV
    // positive, too. (with a certain probability)
    // else if (random_generator->Uniform() <
    //         sparam->birth_infection_probability) {
    //  // child->state_ = GemsState::kAcute;
    //  child->state_ = GemsState::kChronic;

    ///! The aguments below are currently either not used or repetitive.
    // // year of infection to present year, Question: Ask Lukas how to get
    // iter child->year_of_infection_ = 2000;
    //}
    // AM: birth infection probability depends on whether mother is treated
    else if (mother->state_ == GemsState::kTreated) {
      if (random_generator->Uniform() <
          sparam->birth_infection_probability_treated) {
        child->state_ = GemsState::kAcute;
      } else {
        child->state_ = GemsState::kHealthy;
      }
    } else {  // AM: Mother is not healthy and not treated
      if (random_generator->Uniform() <
          sparam->birth_infection_probability_untreated) {
        child->state_ = GemsState::kAcute;
      } else {
        child->state_ = GemsState::kHealthy;
      }
    }

    ///! The aguments below are currently either not used or repetitive.
    // // NOTE: we do not assign a specific mother or partner at the moment. Use
    // // nullptr instead.
    // child->mother_id_ = AgentPointer<Person>();
    // child->partner_id_ = AgentPointer<Person>();

    // BioDynaMo API: Add the behaviors to the Agent
    child->AddBehavior(new RandomMigration());
    child->AddBehavior(new GetOlder());
    if (child->sex_ == Sex::kFemale) {
      child->AddBehavior(new GiveBirth());
    } else {
      child->AddBehavior(new MatingBehaviour());
    }
    return child;
  }

  void Run(Agent* agent) override {
    auto* sim = Simulation::GetActive();
    auto* ctxt = sim->GetExecutionContext();
    auto* random = sim->GetRandom();
    auto* param = sim->GetParam();
    const auto* sparam = param->Get<SimParam>();
    auto* mother = bdm_static_cast<Person*>(agent);
    // Each potential mother gives birth with a certain probability.
    if (random->Uniform() < sparam->give_birth_probability &&
        mother->age_ <= sparam->max_age && mother->age_ >= sparam->min_age) {
      // Create a child
      auto* new_child = CreateChild(random, mother, sparam);
      // BioDynaMo API: Add agent (child) to simulation
      ctxt->AddAgent(new_child);
    }
  }
};

}  // namespace bdm

#endif  // PERSON_BEHAVIOR_H_
