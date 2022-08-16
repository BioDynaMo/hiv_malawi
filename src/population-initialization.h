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

#ifndef POPULATION_INITIALIZATION_H_
#define POPULATION_INITIALIZATION_H_

#include "biodynamo.h"
#include "sim-param.h"

namespace bdm {
namespace hiv_malawi {

////////////////////////////////////////////////////////////////////////////////
// Helper functions to initialize the entire popluation at the beginning of the
// simulation. Most parameters are defined in sim-param.h
////////////////////////////////////////////////////////////////////////////////

// Gives stochastic age based on hard coded age-distribution
float SampleAge(float rand_num_1, float rand_num_2, int sex,
                const std::vector<float>& age_distribution);

// Gives stochastic location based on hard coded location-distribution
int SampleLocation(float rand_num,
                   const std::vector<float>& location_distribution);

// Gives stochastic sex based on probability
int SampleSex(float rand_num, float probability_male);

// Sample HIV healt state; returns GemsState::kHealthy in 1-initial_inf... of
// the cases
int SampleState(float rand_num_1, float rand_num_2,
                float initial_healthy_probability,
                const std::vector<float>& initial_infection_probability);

// Compute sociobehavioural-factor; return 1 in sociobehavio... of the cases
int ComputeSociobehavioural(float rand_num, int age,
                            float sociobehavioural_risk_probability);

// Compute biomedical-factor; return 1 in biomedical_ris... of the cases
int ComputeBiomedical(float rand_num, int age,
                      float biomedical_risk_probability);

// create a single person
auto CreatePerson(Random* random_generator, SimParam* sparam);

// Initialize an entire population for the BDM simulation
void InitializePopulation();

}  // namespace hiv_malawi
}  // namespace bdm

#endif  // POPULATION_INITIALIZATION_H_
