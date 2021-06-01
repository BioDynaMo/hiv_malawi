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

#include <iostream>

#include "core/operation/operation_registry.h"
#include "core/operation/reduction_op.h"

#include "bdm-simulation.h"
#include "categorical-environment.h"
#include "population-initialization.h"
#include "sim-param.h"
#include "visualize.h"

namespace bdm {

// Initialize parameter group Uid, part of the BioDynaMo API, needs to be part
// of a cc file, depends on #include "sim-param.h". With this, we can access the
// simulation parameters anywhere in the simulation.
const ParamGroupUid SimParam::kUid = ParamGroupUidGenerator::Get()->NewUid();


////////////////////////////////////////////////////////////////////////////////
// BioDynaMo's main simulation
////////////////////////////////////////////////////////////////////////////////
int Simulate(int argc, const char** argv) {
  // Register the Siulation parameter
  Param::RegisterParamGroup(new SimParam());

  // Initialize the Simulation
  Simulation simulation(argc, argv);

  // Get a pointer to the param object
  auto* param = simulation.GetParam();
  // Get a pointer to an instance of SimParam
  auto* sparam = param->Get<SimParam>();

  // Use custom environment for simulation. The command SetEnvironment is
  // currently not implemented in the master, it needs to set in BioDynaMo
  // in simulation.h / simulation.cc (see README.md)
  auto* env = new CategoricalEnvironment(sparam->min_age, sparam->max_age);
  simulation.SetEnvironment(env);

  // Randomly initialize a population
  {
    Timing timer_init("RUNTIME POPULATION INITIALIZATION: ");
    InitializePopulation();
  }

  // Get population statistics, i.e. extract data from simulation
  // Get the pointer to the TimeSeries
  auto* ts = simulation.GetTimeSeries();
  // Define how to count the healthy individuals
  auto count_healthy = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is healhy.
    auto cond = L2F([](Agent* a){ 
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsHealthy(); 
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond));
  };
  // Define how to count the infected individuals
  auto count_infected = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond = L2F([](Agent* a){ 
      auto* person = bdm_static_cast<Person*>(a);
      return !(person->IsHealthy()); 
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond));
  };
  // Define how to get the time values of the TimeSeries
  auto get_year = [](Simulation* sim) {
    return static_cast<double>(1960 + sim->GetScheduler()->GetSimulatedSteps());
  };
  ts->AddCollector("healthy_agents", count_healthy, get_year);
  ts->AddCollector("infected_agents", count_infected, get_year);
  

  // Unschedule some default operations
  auto* scheduler = simulation.GetScheduler();
  // Don't compute forces
  scheduler->UnscheduleOp(scheduler->GetOps("mechanical forces")[0]);
  // Don't run load balancing, not working with custom environment.
  scheduler->UnscheduleOp(scheduler->GetOps("load balancing")[0]);

  // Run simulation for <number_of_iterations> timesteps
  {
    Timing timer_sim("RUNTIME SIMULATION:                ");
    simulation.GetScheduler()->Simulate(sparam->number_of_iterations);
  }

  {
    Timing timer_post("RUNTIME POSTPROCESSING:            ");

    // Generate ROOT plot to visualize the number of healthy and infected
    // individuals over time.
    PlotAndSaveTimeseries();
  }

  return 0;
}

}  // namespace bdm
