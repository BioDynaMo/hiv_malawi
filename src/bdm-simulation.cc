// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
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
#include "storage.h"
#include "visualize.h"

namespace bdm {

BDM_REGISTER_TEMPLATE_OP(ReductionOp, Population, "ReductionOpPopulation",
                         kCpu);

// Functor to accumulate the information into the population struct.
struct get_thread_local_population_statistics
    : public Functor<void, Agent*, Population*> {
  void operator()(Agent* agent, Population* tl_pop) {
    // question: what's bdm static cast?
    auto* person = bdm_static_cast<Person*>(agent);
    // Note: possibly rewrite with out if/else and check if it's faster
    int age = static_cast<int> (person->age_);
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


// Functor to summarize thread local population into one population struct
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

// BioDynaMo's main simulation
int Simulate(int argc, const char** argv) {
  Simulation simulation(argc, argv);

  // Randomly initialize a population
  {
    Timing timer_init("RUNTIME POPULATION INITIALIZATION: ");
    auto random = simulation.GetRandom();
    // Use custom environment for simulation. The command SetEnvironment is 
    // currently not implemented in the master, it needs to set in BioDynaMo 
    // in simulation.h / simulation.cc
    auto* env = new CategoricalEnvironment();
    simulation.SetEnvironement(env);
    random->SetSeed(1234);
    initialize_population(random, 40000);
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

  // Run simulation for one timestep
  {
    Timing timer_sim("RUNTIME SIMULATION:                ");
    simulation.GetScheduler()->Simulate(100);
  }

  {
    Timing timer_post("RUNTIME POSTPROCESSING:            ");
    const auto& sim_result = get_statistics_impl->GetResults();
    // save_to_disk(sim_result);
    //std::cout << sim_result[0] << std::endl;
    plot_evolution(sim_result);
  }

  return 0;
}

}  // namespace bdm
