#ifndef DATATYPES_H_
#define DATATYPES_H_

#include <vector>

namespace bdm {

// Describes a snapshot of the population in a certain year
struct Population
{
    int year;
    int healthy_female;
    int healthy_male;
    std::vector<int> infected_female;
    std::vector<int> infected_male;
    std::vector<int> age_female;
    std::vector<int> age_male;
}; 

// Possible illness states
enum GemsState { kGems1, kGems2, kGems3 };

// Possible sex
enum Sex {kMale, kFemale};

// Possible locations
enum Location {kLoc1,kLoc2,kLoc3};

}// namespace bdm

#endif // DATATYPES_H_