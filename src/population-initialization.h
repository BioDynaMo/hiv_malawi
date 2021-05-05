#ifndef POPULATION_INITIALIZATION_H_
#define POPULATION_INITIALIZATION_H_

#include "biodynamo.h"

namespace bdm {

////////////////////////////////////////////////////////////////////////////////
// Helper functions to initialize the popluation
////////////////////////////////////////////////////////////////////////////////

// Gives stochastic age based on hard coded age-distribution
float sample_age(float rand_num_1, float rand_num_2, int sex);

// Gives stochastic location based on hard coded location-distribution
int sample_location(float rand_num);

// Gives stochastic sex based on probability
int sample_sex(float rand_num);

// Sample HIV healt state; returns GemsState::kHealthy in 97% of the cases
int sample_state(float rand_num);

// Compute sociobehavioural-factor; return 0 in 95% of the cases
int compute_sociobehavioural(float rand_num, int age);

// Compute biomedical-factor; return 0 in 95% of the cases
int compute_biomedical(float rand_num, int age);

// create a single person
auto create_person(Random* random_generator);

// Initialize an entire population for the BDM simulation
void initialize_population(Random* random_generator, int population_size);

}  // namespace bdm

#endif  // POPULATION_INITIALIZATION_H_
