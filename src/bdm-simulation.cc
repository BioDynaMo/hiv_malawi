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
  auto set_param = [&](Param* param) {
    param->show_simulation_step = true;
    param->remove_output_dir_contents = false;
  };
  Simulation simulation(argc, argv, set_param);

  // Get a pointer to the param object
  auto* param = simulation.GetParam();
  // Get a pointer to an instance of SimParam
  auto* sparam = param->Get<SimParam>();

  // AM: added age_category and sociobehavioral category
  // constructor.
  auto* env = new CategoricalEnvironment
    (sparam->min_age, sparam->max_age, sparam->nb_age_categories, Location::kLocLast, sparam->nb_sociobehav_categories);
    
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
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsHealthy();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond));
  };
  // Define how to count the infected individuals
  auto count_infected = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return !(person->IsHealthy());
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond));
  };

  // AM: Define how to count the infected acute individuals
  auto count_acute = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsAcute();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond));
  };

  // AM: Define how to count the infected chronic individuals
  auto count_chronic = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsChronic();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond));
  };

  // AM: Define how to count the infected treated individuals
  auto count_treated = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsTreated();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond));
  };

  // AM: Define how to count the infected failing individuals
  auto count_failing = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsFailing();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond));
  };

  auto pct_prevalence = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_infected = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return !(person->IsHealthy());
    });
    auto cond_all = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return !(person->IsHealthy()) or person->IsHealthy();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond_infected)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_all));
  };

  auto pct_incidence = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_acute = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsAcute();
    });
    auto cond_all = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return !(person->IsHealthy()) or person->IsHealthy();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond_acute)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_all));
  };

  // Define how to get the time values of the TimeSeries
  auto get_year = [](Simulation* sim) {
    return static_cast<double>(1960 + sim->GetScheduler()->GetSimulatedSteps());
  };
  ts->AddCollector("healthy_agents", count_healthy, get_year);
  ts->AddCollector("infected_agents", count_infected, get_year);

  // AM: Added detailed follow up of HIV states time series
  ts->AddCollector("acute_agents", count_acute, get_year);
  ts->AddCollector("chronic_agents", count_chronic, get_year);
  ts->AddCollector("treated_agents", count_treated, get_year);
  ts->AddCollector("failing_agents", count_failing, get_year);
  ts->AddCollector("prevalence", pct_prevalence, get_year);
  ts->AddCollector("incidence", pct_incidence, get_year);

  // Unschedule some default operations
  auto* scheduler = simulation.GetScheduler();
  // Don't compute forces
  scheduler->UnscheduleOp(scheduler->GetOps("mechanical forces")[0]);
  // Don't run load balancing, not working with custom environment.
  scheduler->UnscheduleOp(scheduler->GetOps("load balancing")[0]);

  // Run simulation for <number_of_iterations> timesteps
  {
    Timing timer_sim("RUNTIME");
    simulation.GetScheduler()->Simulate(sparam->number_of_iterations);
  }

  {
    Timing timer_post("RUNTIME POSTPROCESSING:            ");

    // Generate ROOT plot to visualize the number of healthy and infected
    // individuals over time.
    PlotAndSaveTimeseries();
  }
    
  // DEBUG - AM - TO DO: Works only when selection depended soloely on locations
  /*env->NormalizeMateLocationFrequencies();
  env->PrintMateLocationFrequencies();*/

  return 0;
}

}  // namespace bdm
