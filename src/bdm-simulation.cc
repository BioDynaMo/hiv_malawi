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
#include "storage.h"
#include "visualize.h"

namespace bdm {

// Initialize parameter group Uid, part of the BioDynaMo API, needs to be part
// of a cc file, depends on #include "sim-param.h". With this, we can access the
// simulation parameters anywhere in the simulation.
const ParamGroupUid SimParam::kUid = ParamGroupUidGenerator::Get()->NewUid();

// Register custom reduction operation to extract population information at each
// timestep.
BDM_REGISTER_TEMPLATE_OP(ReductionOp, Population, "ReductionOpPopulation",
                         kCpu);

// Functor to determine entries of the Populatoin struct from a set of agents.
// Is executed by each thread and will result in OMP_NUM_THREADS Population-s.
struct get_thread_local_population_statistics
    : public Functor<void, Agent*, Population*> {
  void operator()(Agent* agent, Population* tl_pop) {
    // question: what's bdm static cast?
    auto* person = bdm_static_cast<Person*>(agent);
    // Note: possibly rewrite with out if/else and check if it's faster
    int age = static_cast<int>(person->age_);
    if (person->sex_ == Sex::kMale) {
      tl_pop->age_male[age] += 1;
      if (person->state_ == GemsState::kHealthy) {
        tl_pop->healthy_male += 1;
      } else {
        tl_pop->infected_male[person->state_ - 1] += 1;
      }
    } else {
      tl_pop->age_female[age] += 1;
      if (person->state_ == GemsState::kHealthy) {
        tl_pop->healthy_female += 1;
      } else {
        tl_pop->infected_female[person->state_ - 1] += 1;
      }
    }
  }
};

// Functor to summarize thread local Population-s into one Population struct
struct add_thread_local_populations
    : public Functor<Population, const SharedData<Population>&> {
  Population operator()(const SharedData<Population>& tl_populations) override {
    // Get object for total population
    Population total_pop;
    for (Population tl_population : tl_populations) {
      total_pop += tl_population;
    }
    return total_pop;
  }
};

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
  simulation.SetEnvironement(env);

  // ToDo: print simulation parameter

  // Randomly initialize a population
  {
    Timing timer_init("RUNTIME POPULATION INITIALIZATION: ");
    initialize_population();
  }

  // Get population statistics, i.e. extract data from simulation
  auto* get_statistics = NewOperation("ReductionOpPopulation");
  auto* get_statistics_impl =
      get_statistics->GetImplementation<ReductionOp<Population>>();
  get_statistics_impl->Initialize(new get_thread_local_population_statistics(),
                                  new add_thread_local_populations());
  auto* scheduler = simulation.GetScheduler();
  scheduler->ScheduleOp(get_statistics);

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
    const auto& sim_result = get_statistics_impl->GetResults();

    // // Write simulatoin data to disk for further external investigations
    // save_to_disk(sim_result);

    // // Print population at time step 0 to shell
    // std::cout << sim_result[0] << std::endl;

    // Generate ROOT plot to visualize the number of healthy and infected
    // individuals over time.
    plot_evolution(sim_result);
  }

  return 0;
}

}  // namespace bdm
