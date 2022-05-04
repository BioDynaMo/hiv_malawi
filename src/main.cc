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

////////////////////////////////////////////////////////////////////////////////
//            .___  ___.      ___       __  .__   __.
//            |   \/   |     /   \     |  | |  \ |  |
//            |  \  /  |    /  ^  \    |  | |   \|  |
//            |  |\/|  |   /  /_\  \   |  | |  . `  |
//            |  |  |  |  /  _____  \  |  | |  |\   |
//            |__|  |__| /__/     \__\ |__| |__| \__|
//
////////////////////////////////////////////////////////////////////////////////

#include "bdm-simulation.h"
#include "stdout-utlis.h"

int main(int argc, const char** argv) {
  PrintHeader();

  bdm::hiv_malawi::Simulate(argc, argv);

  PrintClosing();
  return 0;
}
