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

}  // namespace hiv_malawi

}  // namespace bdm
