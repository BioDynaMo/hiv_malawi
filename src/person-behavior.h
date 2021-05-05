#ifndef PERSON_BEHAVIOR_H_
#define PERSON_BEHAVIOR_H_

#include "person.h"
#include "datatypes.h"
#include "categorical-environment.h"
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

    int migration_direction = static_cast<int>(random->Gaus(0.0, 2.0));
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
    auto* person = bdm_static_cast<Person*>(agent);

    // Randomly determine the number of mates
    int no_mates = static_cast<int>(random->Gaus(3.0, 1.0));
    // Probability to get infected if mating with infected individual
    float infection_probability{0.7};

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
            random->Uniform() < infection_probability) {
          person->state_ = GemsState::kGems1;
        }
        // Scenario infected male has intercourse with healthy female
        else if (mate->state_ == GemsState::kHealthy &&
                 person->state_ != GemsState::kHealthy &&
                 random->Uniform() < infection_probability) {
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
    auto* person = bdm_static_cast<Person*>(agent);

    // when turning 15, assign risk factors
    if (person->age_ < 15 && person->age_ + 1 > 15) {
      // update risk factors stochastically like in initialization
      if (random->Uniform() > 0.95) {
        person->social_behaviour_factor_ = 1;
      }
      if (random->Uniform() > 0.95) {
        person->biomedical_factor_ = 1;
      }
    }

    // possibly die - if not, just get older
    bool stay_alive{true};
    // Let's assume a linear increase of the death probability per year for
    // healty agents starting from 45 to 120.
    if (person->state_ == GemsState::kHealthy) {
      if (random->Uniform() < (person->age_ - 45.) / (75. * 8.)) {
        stay_alive = false;
      }
    }
    // Let's assume a linear increase of the death probability per year for
    // non-healty agents starting from 5 to 50.
    if (person->state_ != GemsState::kHealthy) {
      if (random->Uniform() < (person->age_ - 5.) / (45. * 8.)) {
        stay_alive = false;
      }
    }
    // hard cut at 90
    if (person->age_ >= 90.0) {
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
  Person* create_child(Random* random_generator, Person* mother) {
    // Get all random numbers for initialization
    std::vector<double> rand_num{};
    rand_num.reserve(10);
    for (int i = 0; i < 10; i++) {
      rand_num[i] = random_generator->Uniform();
    }

    // Get a new person
    // Cells are simulated with a spacial uniform grid environment. Typically,
    // cells don't occur on the very same position and therefore the number of
    // cell per grid box is described with a uint16_t. Thus, if we don't asssign
    // random positions, we are bounded to a maximum number of 65535 agents.
    Person* child = new Person(
        {100.0 * rand_num[7], 100.0 * rand_num[8], 100.0 * rand_num[9]});

    // Assign sex
    child->sex_ = sample_sex(rand_num[0]);
    // Assign age - possibly -1 ?
    child->age_ = rand_num[1];
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
    // let's assume that if a mother is HIV positive, in 80 % of the cases the
    // child will be hiv positive, too.
    else if (rand_num[2] < 0.8) {
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
    child->AddBehavior(new MatingBehaviour());
    child->AddBehavior(new GetOlder());
    if (child->sex_ == Sex::kFemale){
      child->AddBehavior(new GiveBirth());
    }
    return child;
  }

  void Run(Agent* agent) override {
    auto* sim = Simulation::GetActive();
    auto* ctxt = sim->GetExecutionContext();
    auto* random = sim->GetRandom();
    auto* mother = bdm_static_cast<Person*>(agent);
    // Parameter 0.24 is chosen because our GiveBirth Behaviour is based on a
    // Bernoulli experiment. A binomial distribuition peaks at around 6 for 25
    // tries and a birth probability of 0.24.
    if (random->Uniform(0.0, 1.0) < 0.24 && mother->age_ <= 40 &&
        mother->age_ >= 15) {
      auto* new_child = create_child(random, mother);
      ctxt->AddAgent(new_child);
    }
  }
};

}  // namespace bdm

#endif  // PERSON_BEHAVIOR_H_
