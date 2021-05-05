#include <cassert>
#include <iostream>
#include <limits>
#include <vector>

#include "biodynamo.h"
#include "core/agent/agent_pointer.h"
#include "core/simulation.h"
#include "core/util/log.h"

#include "datatypes.h"
#include "population-initialization.h"

// All hard-coded numbers are taken from Janne's work (Parameters_D1.R)

namespace bdm {

float sample_age(float rand_num_1, float rand_num_2, int sex) {
  // use different age distributions for male and female
  std::vector<float> age_distribution{};
  if (sex == Sex::kMale) {
    age_distribution = {0.156, 0.312, 0.468, 0.541, 0.614, 0.687,
                        0.76,  0.833, 0.906, 0.979, 0.982, 0.985,
                        0.988, 0.991, 0.994, 0.997, 1};
  } else if (sex == Sex::kFemale) {
    age_distribution = {0.156, 0.312, 0.468, 0.54,  0.612, 0.684,
                        0.756, 0.828, 0.9,   0.972, 0.976, 0.98,
                        0.984, 0.988, 0.992, 0.996, 1};
  } else {
    Log::Fatal("sample_age()",
               "Received sex that is neither male (0) nor female (1): ", sex);
  }
  for (int i = 0; i < age_distribution.size(); i++) {
    if (rand_num_1 <= age_distribution[i]) {
      return 5 * (i + rand_num_2);
    } else {
      continue;
    }
  }
  Log::Warning("sample_age()",
               "Could not sample the age. Recieved inputs:", rand_num_1, ", ",
               rand_num_2, ", ", sex, ". Use age 0.");
  return 0;
}

int sample_location(float rand_num) {
  // The current vector leads to locactions ranging from 0 to 27
  std::vector<float> location_distribution{
      0.012, 0.03,  0.031, 0.088, 0.104, 0.116, 0.175, 0.228, 0.273, 0.4,
      0.431, 0.453, 0.498, 0.517, 0.54,  0.569, 0.645, 0.679, 0.701, 0.736,
      0.794, 0.834, 0.842, 0.86,  0.903, 0.925, 0.995, 1,     1};
  for (int i = 0; i < location_distribution.size(); i++) {
    if (rand_num <= location_distribution[i]) {
      return i;
    } else {
      continue;
    }
  }
  Log::Warning("sample_location()",
               "Could not sample the location. Recieved inputs: ", rand_num,
               ". Use location 0.");
  return 0;
}

int sample_sex(float rand_num) {
  if (rand_num <= 0.499) {
    return Sex::kMale;
  } else {
    return Sex::kFemale;
  }
}

int sample_state(float rand_num) {
  if (rand_num <= 0.97) {
    return GemsState::kHealthy;
  } else {
    return GemsState::kGems1;
  }
}

int compute_sociobehavioural(float rand_num, int age) {
  if (age <= 15) {
    return 0;
  }
  if (rand_num < 0.95) {
    return 0;
  } else {
    return 1;
  }
}

int compute_biomedical(float rand_num, int age) {
  if (age <= 15) {
    return 0;
  }
  if (rand_num < 0.95) {
    return 0;
  } else {
    return 1;
  }
}

auto create_person(Random* random_generator) {
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
  Person* person = new Person(
      {100.0 * rand_num[7], 100.0 * rand_num[8], 100.0 * rand_num[9]});

  person->SetDiameter(.1);

  // Assign sex
  person->sex_ = sample_sex(rand_num[0]);
  // Assign age
  person->age_ = sample_age(rand_num[1], rand_num[2], person->sex_);
  // Assign location
  person->location_ = sample_location(rand_num[3]);
  // Assign the GemsState of the person.
  person->state_ = sample_state(rand_num[4]);
  // Compute risk factors
  person->social_behaviour_factor_ =
      compute_sociobehavioural(rand_num[5], person->age_);
  person->biomedical_factor_ = compute_biomedical(rand_num[6], person->age_);
  // Store the year when the agent got infected
  person->year_of_infection_ = std::numeric_limits<float>::max();
  // NOTE: we do not assign a specific mother or partner during the population
  // initialization. Use nullptr.
  person->mother_id_ = AgentPointer<Person>();
  person->partner_id_ = AgentPointer<Person>();

  // Add the behaviours to the person.
  // person->AddBehavior(new Infection());
  // person->AddBehavior(new RandomMovement());
  person->AddBehavior(new RandomMigration());
  person->AddBehavior(new GetOlder());
  person->AddBehavior(new MatingBehaviour());
  if (person->sex_ == Sex::kFemale && person->age_ > 15 && person->age_ < 40) {
    person->AddBehavior(new GiveBirth());
  }
  return person;
};

void initialize_population(Random* random_generator, int population_size) {
#pragma omp parallel
  {
    auto* sim = Simulation::GetActive();
    auto* ctxt = sim->GetExecutionContext();

#pragma omp for
    for (int x = 0; x < population_size; x++) {
      auto* new_person = create_person(random_generator);
      ctxt->AddAgent(new_person);
    }
  }
}

}  // namespace bdm
