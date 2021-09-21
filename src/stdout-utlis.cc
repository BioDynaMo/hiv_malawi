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

#include <iostream>
#include <string>
#include "bdm_version.h"

void PrintHeader() {
  std::string header{
      "\n"
      "========================================================================"
      "\n             _  _ _____   __  __  __      _             _           \n"
      "            | || |_ _\\ \\ / / |  \\/  |__ _| |__ ___ __ _(_)         \n"
      "            | __ || | \\ V /  | |\\/| / _` | / _` \\ V  V / |         \n"
      "            |_||_|___| \\_/   |_|  |_\\__,_|_\\__,_|\\_/\\_/|_|       \n"
      "\n"
      "This is a BioDynaMo runtime to simulate the spread of HIV in Malawi.\n"
      "You are using BioDynaMo "};
  bdm::Version ver{};
  std::cout << header << ver.String() << " .\n" << std::endl;
}

void PrintClosing() {
  std::string closing{
      "\nSimulation completed successfully!\n"
      "\n"
      "========================================================================"
      "\n"};
  std::cout << closing << std::endl;
}
