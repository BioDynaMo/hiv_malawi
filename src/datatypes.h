// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN (Tobias Duswald, Lukas Breitwieser, Ahmad Hesam, Fons
// Rademakers) for the benefit of the BioDynaMo collaboration. All Rights
// Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef DATATYPES_H_
#define DATATYPES_H_

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

namespace bdm {

// Possible illness states. If you adjust this enum, make sure to put the last
// element into the definition of struct Population below. Even better, just
// leave the kGemsLast at the End.
enum GemsState { kHealthy, kGems1, kGems2, kGems3, kGemsLast };

// Possible sex
enum Sex { kMale, kFemale };

// Possible locations.
enum Location {
  kLoc1,
  kLoc2,
  kLoc3,
  kLoc4,
  kLoc5,
  kLoc6,
  kLoc7,
  kLoc8,
  kLoc9,
  kLoc10,
  kLoc11,
  kLoc12,
  kLoc13,
  kLoc14,
  kLoc15,
  kLoc16,
  kLoc17,
  kLoc18,
  kLoc19,
  kLoc20,
  kLoc21,
  kLoc22,
  kLoc23,
  kLoc24,
  kLoc25,
  kLoc26,
  kLoc27,
  kLoc28,
  // This is not an actual location, it needs to be on the last position of the
  // enum to determine it's length and use in other functions.
  kLocLast
};

// For each year / simulation step, we describe the population with the struct
// Population.
struct Population {
  // Constructor to obtain correct size of zero-initialized vectors.
  Population()
      : healthy_female(0),
        healthy_male(0),
        infected_female(GemsState::kGemsLast, 0),
        infected_male(GemsState::kGemsLast, 0),
        age_female(120, 0),
        age_male(120, 0) {}

  // Number of healthy females
  int healthy_female;
  // Number of healthy males
  int healthy_male;
  // Number of infected females, we count different GemsStates separately in
  // the vector components.
  std::vector<int> infected_female;
  // Number of infected males, we count different GemsStates separately in
  // the vector components.
  std::vector<int> infected_male;
  // Age distribution for the female population
  std::vector<int> age_female;
  // Age distribution for the male population
  std::vector<int> age_male;

  // Define inplace add for Population. This is necessary to extract the
  // Population in parallel from the set of agents.
  Population& operator+=(const Population& other_population) {
    // add vectors for age_male
    std::transform(this->age_male.begin(), this->age_male.end(),
                   other_population.age_male.begin(), this->age_male.begin(),
                   std::plus<int>());
    // add vectors infected male
    std::transform(this->infected_male.begin(), this->infected_male.end(),
                   other_population.infected_male.begin(),
                   this->infected_male.begin(), std::plus<int>());
    // add vectors age_female
    std::transform(this->age_female.begin(), this->age_female.end(),
                   other_population.age_female.begin(),
                   this->age_female.begin(), std::plus<int>());
    // add vectors infected_female
    std::transform(this->infected_female.begin(), this->infected_female.end(),
                   other_population.infected_female.begin(),
                   this->infected_female.begin(), std::plus<int>());
    // add up healthy_male
    this->healthy_male += other_population.healthy_male;
    // add up healthy_female
    this->healthy_female += other_population.healthy_female;

    return *this;
  }

  // This is a helper function such that a Population "pop" object can be
  // printed as: std::cout << pop << std::endl;
  friend std::ostream& operator<<(std::ostream& out,
                                  const Population& population) {
    out << "Population Information: \n";
    out << "healthy_male    : " << population.healthy_male << " \n";
    out << "healthy_female  : " << population.healthy_female << " \n";
    out << "infected_male   : "
        << std::accumulate(population.infected_male.begin(),
                           population.infected_male.end(), 0);
    out << "\ninfected_female : "
        << std::accumulate(population.infected_female.begin(),
                           population.infected_female.end(), 0);
    out << "\n\nage        male      female\n";
    for (int age = 0; age < std::max(population.age_female.size(),
                                     population.age_male.size());
         age++) {
      out << std::setw(3) << age << "    " << std::setw(8)
          << population.age_male[age] << "    " << std::setw(8)
          << population.age_female[age] << " \n";
    }
    out << std::endl;
    return out;
  }
};

}  // namespace bdm

#endif  // DATATYPES_H_
