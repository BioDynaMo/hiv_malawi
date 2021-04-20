#ifndef DATATYPES_H_
#define DATATYPES_H_

#include <vector>

namespace bdm {

struct Population
{
    std::vector<int> year;
    std::vector<int> healthy;
    std::vector<int> infected;
}; 

}// namespace bdm

#endif // DATATYPES_H_