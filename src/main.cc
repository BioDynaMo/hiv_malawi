////////////////////////////////////////////////////////////////////////////////
//            .___  ___.      ___       __  .__   __.
//            |   \/   |     /   \     |  | |  \ |  |
//            |  \  /  |    /  ^  \    |  | |   \|  |
//            |  |\/|  |   /  /_\  \   |  | |  . `  |
//            |  |  |  |  /  _____  \  |  | |  |\   |
//            |__|  |__| /__/     \__\ |__| |__| \__|
//
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "main.h"
#include "bdm-simulation.h"
#include "stdout-utlis.h"

int main(int argc, const char** argv) {
  /*
  // register parameters that are specific for this simulation
  bdm::Param::RegisterParamGroup(new bdm::SimParam());

  // define additional command line options
  bdm::CommandLineOptions clo(argc, argv);
  clo.AddOption<std::string>("mode", "sim-and-analytical");
  clo.AddOption<double>("beta", "0.06719");
  clo.AddOption<double>("gamma", "0.00521");
  clo.AddOption<uint64_t>("repeat", "10");
  clo.AddOption<bool>("no-legend", "false");
  auto mode = clo.Get<std::string>("mode");

  std::vector<double> seeds;
  bdm::Random r;
  r.SetSeed(2444);
  // get random seeds for simulations
  for (uint64_t i = 0; i < clo.Get<uint64_t>("repeat"); ++i) {
    seeds.push_back(r.Uniform(0, std::numeric_limits<uint16_t>::max()));
  }

  // print seed values
  int i{0};
  for (auto el : seeds) {
    std::cout << "seed[" << i++ << "] = " << el << std::endl;
  }

  // run experiments
  if (mode == "sim-and-analytical") {
    bdm::ExperimentSimAndAnalytical(&clo, seeds);
  } else if (mode == "fit-simulation") {
    bdm::ExperimentFitSimulation(&clo, seeds);
  }

  */
  print_header();

  bdm::Simulate(argc, argv);

  print_closing();
  return 0;
}
