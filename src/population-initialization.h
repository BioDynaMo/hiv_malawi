#ifndef POPULATION_INITIALIZATION_H_
#define POPULATION_INITIALIZATION_H_

#include "biodynamo.h"
#include "sim-param.h"

namespace bdm {

////////////////////////////////////////////////////////////////////////////////
// Helper functions to initialize the entire popluation at the beginning of the
// simulation. Most parameters are defined in sim-param.h
////////////////////////////////////////////////////////////////////////////////

// Gives stochastic age based on hard coded age-distribution
float sample_age(float rand_num_1, float rand_num_2, int sex,
                 const std::vector<float>& age_distribution);

// Gives stochastic location based on hard coded location-distribution
int sample_location(float rand_num,
                    const std::vector<float>& location_distribution);

// Gives stochastic sex based on probability
int sample_sex(float rand_num, float probability_male);

// Sample HIV healt state; returns GemsState::kHealthy in 1-initial_inf... of 
// the cases
int sample_state(float rand_num, float initial_infection_probability);

// Compute sociobehavioural-factor; return 1 in sociobehavio... of the cases
int compute_sociobehavioural(float rand_num, int age,
                             float sociobehavioural_risk_probability);

// Compute biomedical-factor; return 1 in biomedical_ris... of the cases
int compute_biomedical(float rand_num, int age,
                       float biomedical_risk_probability);

// create a single person
auto create_person(Random* random_generator, SimParam* sparam);

// Initialize an entire population for the BDM simulation
void initialize_population();

}  // namespace bdm

#endif  // POPULATION_INITIALIZATION_H_
