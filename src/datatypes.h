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

#ifndef DATATYPES_H_
#define DATATYPES_H_

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

namespace bdm {
namespace hiv_malawi {

// Possible illness states. If you adjust this enum, make sure to put the last
// element into the definition of struct Population below. Even better, just
// leave the kGemsLast at the End.
// AM: Healthy + 4 HIV progression states
enum GemsState { kHealthy, kAcute, kChronic, kTreated, kFailing, kGemsLast };

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

// Possible transmission types. Leave the kTransmissionLast at the End.
enum TransmissionType {
  kMotherToChild,
  kCasualPartner,
  kRegularPartner,
  kTransmissionLast
};

}  // namespace hiv_malawi
}  // namespace bdm

#endif  // DATATYPES_H_
