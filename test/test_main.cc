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

int main(int argc, char** argv) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
