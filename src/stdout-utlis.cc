#include <iostream>
#include <string>
#include "version.h"

void print_header(){
  std::string header{
      "\n"
      "========================================================================"
      "\n             _  _ _____   __  __  __      _             _           \n"
      "            | || |_ _\\ \\ / / |  \\/  |__ _| |__ ___ __ _(_)         \n"
      "            | __ || | \\ V /  | |\\/| / _` | / _` \\ V  V / |         \n"
      "            |_||_|___| \\_/   |_|  |_\\__,_|_\\__,_|\\_/\\_/|_|       \n"
      "\n"
      "This is a BioDynaMo runtime to simulate the spread of HIV in Malawi.\n"
      "You are using BioDynaMo "
  };
  bdm::Version ver{};
  std::cout << header << ver.String() << " ." << std::endl; 
  
  std::cout << "\nFor the simulation, we consider the following parameters:\n";
}

void print_closing(){
  std::string closing{
      "\n"
      "========================================================================"
      "\n"
  };
  std::cout << closing << std::endl;
}