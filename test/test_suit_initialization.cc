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
// #include "biodynamo.h"
// #include "modules/tumor_cell.h"
// #include "sim_param.h"

#define TEST_NAME typeid(*this).name()

namespace bdm {

TEST(InitializationTest, TrivalTest) { EXPECT_DOUBLE_EQ(0, 0); }

}  // namespace bdm
