#ifndef VISUALIZE_H_
#define VISUALIZE_H_

#include <vector>
#include "datatypes.h"

namespace bdm {

// This functions takes a vector of Population-s and visualizes the number of 
// healthy and infected agents over the course of the simulation period.
int plot_evolution(const std::vector<Population>& populations);

}  // namespace bdm

#endif  // VISUALIZE_H_
