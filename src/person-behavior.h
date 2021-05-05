#ifndef PERSON_BEHAVIOR_H_
#define PERSON_BEHAVIOR_H_

#include "categorical-environment.h"
#include "datatypes.h"
#include "person.h"
#include "population-initialization.h"

namespace bdm {

////////////////////////////////////////////////////////////////////////////////
// BioDynaMo's Agent / Individual Behaviours
////////////////////////////////////////////////////////////////////////////////

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

struct MatingBehaviour : public Behavior {
  BDM_BEHAVIOR_HEADER(MatingBehaviour, Behavior, 1);

  MatingBehaviour() {}

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
      for (size_t i = 0; i < no_mates; i++) {
        // choose a random female mate at the location
        AgentPointer<Person> mate =
            env->GetRamdomAgentAtLocation(person->location_);
        if (mate == nullptr) {
          Log::Fatal("MatingBehaviour()",
                     "Received nullptr as AgentPointer mate.");
        }
        // Scenario healthy male has intercourse with infected female
        if (mate->state_ != GemsState::kHealthy &&
            person->state_ == GemsState::kHealthy &&
            random->Uniform() < sparam->infection_probability) {
          person->state_ = GemsState::kGems1;
        }
        // Scenario infected male has intercourse with healthy female
        else if (mate->state_ == GemsState::kHealthy &&
                 person->state_ != GemsState::kHealthy &&
                 random->Uniform() < sparam->infection_probability) {
          mate->state_ = GemsState::kGems1;
        } else {
          ;  // if both are infected or both are healthy, do nothing
        }
      }
    }
  }
};

struct GetOlder : public Behavior {
  BDM_BEHAVIOR_HEADER(GetOlder, Behavior, 1);

  GetOlder() {}

  void Run(Agent* agent) override {
    auto* sim = Simulation::GetActive();
    auto* random = sim->GetRandom();
    auto* param = sim->GetParam();
    const auto* sparam = param->Get<SimParam>();
    auto* person = bdm_static_cast<Person*>(agent);

    // If between min_age and max_age, reassign risk factors
    if (person->age_ >= sparam->min_age && sparam->min_age <= sparam->max_age) {
      // update risk factors stochastically like in initialization
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

    // possibly die - if not, just get older
    bool stay_alive{true};
    // Let's assume a linear increase of the death probability per year for
    // healty agents.
    if (person->state_ == GemsState::kHealthy) {
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

struct GiveBirth : public Behavior {
  BDM_BEHAVIOR_HEADER(GiveBirth, Behavior, 1);

  GiveBirth() {}

  // create a single child
  Person* create_child(Random* random_generator, Person* mother,
                       const SimParam* sparam) {
    // Create new child
    Person* child = new Person();
    // Assign sex
    child->sex_ =
        sample_sex(random_generator->Uniform(), sparam->probability_male);
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
      // Store the year when the agent got infected
      child->year_of_infection_ = std::numeric_limits<float>::max();
    }
    // let's assume that if a mother is HIV positive, the child will be HIV
    // positive, too. (with a certain probability)
    else if (random_generator->Uniform() <
             sparam->birth_infection_probability) {
      child->state_ = GemsState::kGems1;
      // year of infection to present year, Question: Ask Lukas how to get iter
      child->year_of_infection_ = 2000;
    }
    // NOTE: we do not assign a specific mother or partner at the moment. Use
    // nullptr instead.
    child->mother_id_ = AgentPointer<Person>();
    child->partner_id_ = AgentPointer<Person>();

    // Add the "grow and divide" behavior to each cell
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
      auto* new_child = create_child(random, mother, sparam);
      ctxt->AddAgent(new_child);
    }
  }
};

}  // namespace bdm

#endif  // PERSON_BEHAVIOR_H_
