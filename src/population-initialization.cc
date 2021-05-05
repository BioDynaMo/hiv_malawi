#include <limits>
#include <vector>

#include "biodynamo.h"

#include "datatypes.h"
#include "person-behavior.h"
#include "population-initialization.h"

// All hard-coded numbers are taken from Janne's work (Parameters_D1.R)

namespace bdm {

float sample_age(float rand_num_1, float rand_num_2, int sex,
                 const std::vector<float>& age_distribution) {
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

int sample_location(float rand_num,
                    const std::vector<float>& location_distribution) {
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

int sample_sex(float rand_num, float probability_male) {
  if (rand_num <= probability_male) {
    return Sex::kMale;
  } else {
    return Sex::kFemale;
  }
}

int sample_state(float rand_num, float initial_infection_probability) {
  if (rand_num <= initial_infection_probability) {
    return GemsState::kGems1;
  } else {
    return GemsState::kHealthy;
  }
}

int compute_sociobehavioural(float rand_num, int age,
                             float sociobehavioural_risk_probability) {
  if (age <= 15) {
    return 0;
  }
  if (rand_num < sociobehavioural_risk_probability) {
    return 0;
  } else {
    return 1;
  }
}

int compute_biomedical(float rand_num, int age,
                       float biomedical_risk_probability) {
  if (age <= 15) {
    return 0;
  }
  if (rand_num < biomedical_risk_probability) {
    return 0;
  } else {
    return 1;
  }
}

auto create_person(Random* random_generator, const SimParam* sparam) {
  // Get all random numbers for initialization
  std::vector<float> rand_num{};
  rand_num.reserve(10);
  for (int i = 0; i < 10; i++) {
    rand_num[i] = static_cast<float>(random_generator->Uniform());
  }

  // Create new person
  Person* person = new Person();
  // Assign sex
  person->sex_ = sample_sex(rand_num[0], sparam->probability_male);
  // Assign age
  if (person->sex_ == Sex::kMale) {
    person->age_ = sample_age(rand_num[1], rand_num[2], person->sex_,
                              sparam->male_age_distribution);
  } else {
    person->age_ = sample_age(rand_num[1], rand_num[2], person->sex_,
                              sparam->female_age_distribution);
  }
  // Assign location
  person->location_ =
      sample_location(rand_num[3], sparam->location_distribution);
  // Assign the GemsState of the person.
  person->state_ =
      sample_state(rand_num[4], sparam->initial_infection_probability);
  // Compute risk factors
  person->social_behaviour_factor_ = compute_sociobehavioural(
      rand_num[5], person->age_, sparam->sociobehavioural_risk_probability);
  person->biomedical_factor_ = compute_biomedical(
      rand_num[6], person->age_, sparam->biomedical_risk_probability);
  // Store the year when the agent got infected
  person->year_of_infection_ = std::numeric_limits<float>::max();
  // NOTE: we do not assign a specific mother or partner during the population
  // initialization. Use nullptr.
  person->mother_id_ = AgentPointer<Person>();
  person->partner_id_ = AgentPointer<Person>();

  // Add the behaviours to the person.
  person->AddBehavior(new RandomMigration());
  person->AddBehavior(new GetOlder());
  person->AddBehavior(new MatingBehaviour());
  if (person->sex_ == Sex::kFemale && person->age_ > sparam->min_age &&
      person->age_ < sparam->max_age) {
    person->AddBehavior(new GiveBirth());
  }
  return person;
};

void initialize_population() {
#pragma omp parallel
  {
    auto* sim = Simulation::GetActive();
    auto* ctxt = sim->GetExecutionContext();
    auto* random_generator = sim->GetRandom();
    auto* param = sim->GetParam();
    const auto* sparam = param->Get<SimParam>();

#pragma omp for
    for (int x = 0; x < sparam->initial_population_size; x++) {
      auto* new_person = create_person(random_generator, sparam);
      ctxt->AddAgent(new_person);
    }
  }
}

}  // namespace bdm
