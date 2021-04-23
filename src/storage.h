#ifndef STORAGE_H_
#define STORAGE_H_

#include <vector>
#include "datatypes.h"

namespace bdm {

int save_to_disk(std::vector<Population> populations);

}  // namespace bdm

#endif  // STORAGE_H_
