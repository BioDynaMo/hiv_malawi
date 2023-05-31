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

#include <gtest/gtest.h>
#include <numeric>
#include <random>
#include "population-initialization.h"

#define TEST_NAME typeid(*this).name()

namespace bdm {

namespace hiv_malawi {

// Test the population initialization function SampleSex
TEST(InitializationTest, SampleSex) {
  // Promblem setup
  const float probability_male = 0.2;
  const float probability_female = 1 - probability_male;
  const int n_samples = 100000;
  std::vector<int> sex(n_samples);
  EXPECT_EQ(sex.size(), n_samples);

  // Random number generator
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.0, 1.0);

  // Female counter
  int female_counter = 0;

  for (size_t i = 0; i < n_samples; i++) {
    const double random_number = dis(gen);
    if (random_number > probability_male) {
      // Count females that we expect
      female_counter += 1;
    }
    // Sample Sex with function
    sex[i] = SampleSex(random_number, probability_male);
  }

  // Sum up all values in the vector to compute the probability to observe
  // males / females in the sample
  int num_females = std::accumulate(sex.begin(), sex.end(), 0);
  const double probability_female_measured =
      static_cast<double>(num_females) / n_samples;
  const double probability_male_measured =
      static_cast<double>(n_samples - num_females) / n_samples;

  // Compare expected number of females with females sampled via SampleSex()
  EXPECT_EQ(num_females, female_counter);

  // Check if we observe the correct probabilites in the samples as compared to
  // the problem setup.
  EXPECT_LT(abs(probability_female - probability_female_measured), 0.01);
  EXPECT_LT(abs(probability_male - probability_male_measured), 0.01);
}

// TEST(InitializationTest, AcuteChronic) {
//   const float initial_prevalence = 0.2;
//   const float initial_healthy_probability = 1-initial_prevalence;
//   const std::vector<float> initial_infection_probability{1.0, 1.0, 1.0, 1.0}
//   ; const int n_samples = 100000; std::vector<int> stage(n_samples);
//   EXPECT_EQ(stage.size(), n_samples);

//   // Random number generator
//   std::random_device rd;
//   std::mt19937 gen(rd());
//   std::uniform_real_distribution<> dis(0.0, 1.0);

//   // Counters
//   int healthy_counter = 0;
//   int acute_counter = 0;
//   int chronic_counter = 0;
//   int treated_counter = 0;
//   int failing_counter = 0;

//   for (size_t i = 0; i < n_samples; i++) {
//     const double random_number_1 = dis(gen);
//     const double random_number_2 = dis(gen);
//     if (random_number_1 < initial_healthy_probability) {
//       // Count females that we expect
//       healthy_counter += 1;
//     }
//     if (random_number_1 >= initial_healthy_probability) {
//       if (random_number_2 < initial_infection_probability[0]) {
//         acute_counter += 1;
//       } else {
//         if (random_number_2 < initial_infection_probability[1]) {
//           chronic_counter += 1;
//         } else {
//           if (random_number_2 < initial_infection_probability[2]) {
//             treated_counter += 1;
//           } else {
//             if (random_number_2 < initial_infection_probability[3]) {
//               failing_counter += 1;
//             }
//           }
//         }
//       }
//     }
//     // Sample Stage with function
//     stage[i] = SampleState(random_number_1, random_number_2,
//     initial_healthy_probability, initial_infection_probability );
//   }

//   int num_healthy = std::accumulate(stage.begin(), stage.end(), 0);
//   int num_acute = std::accumulate(stage.begin(), stage.end(), 1);
//   int num_chronic = std::accumulate(stage.begin(), stage.end(), 2);
//   int num_treated = std::accumulate(stage.begin(), stage.end(), 3);
//   int num_failing = std::accumulate(stage.begin(), stage.end(), 4);
//   const double probability_healthy_measured =
//       static_cast<double>(num_healthy) / n_samples;
//   const double probability_acute_measured =
//       static_cast<double>(num_acute) / n_samples;
//   const double probability_chronic_measured =
//       static_cast<double>(num_chronic) / n_samples;
//   const double probability_treated_measured =
//       static_cast<double>(num_treated) / n_samples;
//   const double probability_failing_measured =
//       static_cast<double>(num_failing) / n_samples;

//   EXPECT_EQ(num_healthy, healthy_counter)
//   EXPECT_EQ(num_acute, acute_counter)
//   EXPECT_EQ(num_chronic, chronic_counter)
//   EXPECT_EQ(num_treated, treated_counter)
//   EXPECT_EQ(num_failing, failing_counter)
// }

}  // namespace hiv_malawi

}  // namespace bdm
