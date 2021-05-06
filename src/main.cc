// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN (Tobias Duswald, Lukas Breitwieser, Ahmad Hesam, Fons
// Rademakers) for the benefit of the BioDynaMo collaboration. All Rights 
// Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
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

#include "main.h"
#include "bdm-simulation.h"
#include "stdout-utlis.h"

int main(int argc, const char** argv) {
  print_header();

  bdm::Simulate(argc, argv);

  print_closing();
  return 0;
}
