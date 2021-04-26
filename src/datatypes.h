#ifndef DATATYPES_H_
#define DATATYPES_H_

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

namespace bdm {

// Possible illness states. If you adjust this enum, make sure to put the last
// element into the definition of struct Population below.
enum GemsState { kHealthy, kGems1, kGems2, kGems3, kGemsLast };

// Possible sex
enum Sex { kMale, kFemale };

// Possible locations
enum Location { kLoc1, kLoc2, kLoc3 };

// Describes a snapshot of the population in a certain year
struct Population {
  // Constructor to obtain correct size of zero-initialized vectors.
  Population()
      : healthy_female(0),
        healthy_male(0),
        infected_female(GemsState::kGemsLast, 0),
        infected_male(GemsState::kGemsLast, 0),
        age_female(120, 0),
        age_male(120, 0) {}

  // member variables
  int healthy_female;
  int healthy_male;
  std::vector<int> infected_female;
  std::vector<int> infected_male;
  std::vector<int> age_female;
  std::vector<int> age_male;

  // Define inplace add for Population
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
