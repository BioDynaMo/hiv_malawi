#ifndef STORAGE_H_
#define STORAGE_H_

#include <vector>
#include "datatypes.h"

namespace bdm {

// This function is currently not implemented. If necessary, this function can 
// be filled with code to save the Population-s at different stages of the 
// simulation to disk. Consider using ROOT files.
int save_to_disk(const std::vector<Population>& populations);

}  // namespace bdm

#endif  // STORAGE_H_
