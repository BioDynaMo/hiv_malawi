#include <cassert>
#include <iostream>
#include <vector>

#include "core/simulation.h"

#include "datatypes.h"
#include "person.h"
#include "population-initialization.h"

// All hard-coded numbers are taken from Janne's work (Parameters_D1.R)

namespace bdm {

float sample_age(float rand_num_1, float rand_num_2, int sex) {
  // use different age distributions for male and female
  std::vector<float> age_distribution{};
  // if (sex == Sex::kMale) {
  if (sex == 0) {
    age_distribution = {0.156, 0.312, 0.468, 0.541, 0.614, 0.687,
                        0.76,  0.833, 0.906, 0.979, 0.982, 0.985,
                        0.988, 0.991, 0.994, 0.997, 1};
  } else {
    age_distribution = {0.156, 0.312, 0.468, 0.54,  0.612, 0.684,
                        0.756, 0.828, 0.9,   0.972, 0.976, 0.98,
                        0.984, 0.988, 0.992, 0.996, 1};
  }
  for (int i = 0; i < age_distribution.size(); i++) {
    if (rand_num_1 <= age_distribution[i]) {
      return 5 * (i + rand_num_2);
    } else {
      continue;
    }
  }
  std::cout << "Warning: sample_age could not sampe the age. \n"
            << "Received rand_num number: " << rand_num_1 << std::endl;
  return 0;
}

int sample_location(float rand_num) {
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
  std::cout << "Warning: sample_location could not find location \n"
            << "Received rand_num number: " << rand_num << std::endl;
  return 0;
}

int sample_sex(float rand_num) {
  if (rand_num <= 0.499) {
    return Sex::kMale;
  } else {
    return Sex::kFemale;
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

void initialize_population(int population_size) {

  auto create_person = [&]() {
    Person* person = new Person({0.0,0.0,0.0});
    person->SetDiameter(2.0);
    // Add the "grow and divide" behavior to each cell
    //person->AddBehavior(new GrowthDivision());
    return person;
  };

  //#pragma omp parallel
  {
    auto* sim = Simulation::GetActive();
    auto* ctxt = sim->GetExecutionContext();

    //#pragma omp for
    for (int x = 0; x < population_size; x++) {
      auto* new_person = create_person();
      ctxt->AddAgent(new_person);
    }
  }
}

}  // namespace bdm
