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
#include "datatypes.h"
#include "population-initialization.h"
#include "storage.h"
#include "visualize.h"

namespace bdm {

BDM_REGISTER_TEMPLATE_OP(ReductionOp, Population, "ReductionOpPopulation",
                         kCpu);

struct get_thread_local_population_statistics
    : public Functor<void, Agent*, Population*> {
  void operator()(Agent* agent, Population* tl_pop) {
    // question: what's bdm static cast?
    auto* person = bdm_static_cast<Person*>(agent);
    // Note: possibly rewrite with out if/else and check if it's faster
    if (person->sex_ == Sex::kMale) {
      tl_pop->age_male[static_cast<int>(person->age_)] += 1;
      if (person->state_ == GemsState::kHealthy) {
        tl_pop->healthy_male += 1;
      } else {
        tl_pop->infected_male[person->state_ - 1] += 1;
      }
    } else {
      tl_pop->age_female[static_cast<int>(person->age_)] += 1;
      if (person->state_ == GemsState::kHealthy) {
        tl_pop->healthy_female += 1;
      } else {
        tl_pop->infected_female[person->state_ - 1] += 1;
      }
    }
  }
};

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

int Simulate(int argc, const char** argv) {
  Simulation simulation(argc, argv);

  // Define initial model - in this example: single cell at origin
  // auto* rm = simulation.GetResourceManager();
  // auto* cell = new Cell(30);
  // rm->AddAgent(cell);

  // Randomly initialize a population
  {
    Timing timer_init("RUNTIME POPULATION INITIALIZATION: ");
    auto random = simulation.GetRandom();
    random->SetSeed(1234);
    initialize_population(random, 2000);
  }

  // DEBUG
  // Test population cout
  // std::cout << sizeof(Population) << std::endl;
  // Population pop;
  // std::cout << pop;
  // std::cout << "person memory " << sizeof(Person) << std::endl;

  // Get population statistics
  auto* get_statistics = NewOperation("ReductionOpPopulation");
  auto* get_statistics_impl =
      get_statistics->GetImplementation<ReductionOp<Population>>();
  get_statistics_impl->Initialize(new get_thread_local_population_statistics(),
                                  new add_thread_local_populations());
  auto* scheduler = simulation.GetScheduler();
  scheduler->ScheduleOp(get_statistics);

  // Don't compute forces
  scheduler->UnscheduleOp(scheduler->GetOps("mechanical forces")[0]);

  // Run simulation for one timestep
  {
    Timing timer_sim("RUNTIME SIMULATION:                ");
    simulation.GetScheduler()->Simulate(100);
  }

  {
    Timing timer_post("RUNTIME POSTPROCESSING:            ");
    const auto& sim_result = get_statistics_impl->GetResults();
    // std::cout << "no populations: " << sim_result.size() << std::endl;
    // std::cout << sim_result[0];
    // save_to_disk(sim_result);
    plot_evolution(sim_result);
  }

  // DEBUG: check number of agents
  // auto* rm = simulation.GetResourceManager();
  // std::cout << "Simulation considers " << rm->GetNumAgents() << "
  // persons.\n";

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace bdm
