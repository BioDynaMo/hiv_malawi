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

#include "core/operation/reduction_op.h"
#include "core/operation/operation_registry.h"

#include "bdm-simulation.h"
#include "datatypes.h"
#include "population-initialization.h"

namespace bdm {

BDM_REGISTER_TEMPLATE_OP(ReductionOp, Population, "ReductionOpPopulation", kCpu);

struct get_thread_local_population_statistics : public Functor<void, Agent*, Population*> {
  void operator()(Agent* agent, Population* tl_pop) {
    // question: what's bdm static cast?
    auto* person = bdm_static_cast<Person*>(agent);
    // Note: possibly rewrite with out if/else and check if it's faster
    if (person->sex_ == Sex::kMale){
      tl_pop->age_male[static_cast<int>(person->age_)] += 1;
      if (person->state_ == GemsState::kHealthy){
        tl_pop->healthy_male += 1;
      }
      else{
        tl_pop->infected_male[person->state_-1] += 1;
      }
    }
    else {
      tl_pop->age_female[static_cast<int>(person->age_)] += 1;
      if (person->state_ == GemsState::kHealthy){
        tl_pop->healthy_female += 1;
      }
      else{
        tl_pop->infected_female[person->state_-1] += 1;
      }
    }
  }
}; 


struct add_thread_local_populations : public Functor<Population, const SharedData<Population>&> {
  Population operator()(const SharedData<Population>& tl_populations ) override {
    // Get object for total population
    Population total_pop;
    for (Population tl_population : tl_populations){
      total_pop += tl_population;
    }
    return total_pop;
  }
};


/*
struct CountSIR : public Functor<void, Agent*, Double4*> {
  void operator()(Agent* agent, Double4* tl_result) {
    auto* person = bdm_static_cast<Person*>(agent);
    (*tl_result)[1] += person->state_ == State::kSusceptible;
    (*tl_result)[2] += person->state_ == State::kInfected;
    (*tl_result)[3] += person->state_ == State::kRecovered;
  }
};

// ---------------------------------------------------------------------------
// CalcRates inherits from bdm Functor class. It works on a Double4'ish vector,
// basically computing the percentage of suceptible, infected, and recovered
// agents. It returns a Double4, containing the number of simulated time steps,
// and the percentage of suceptible, infected, and recovered agents in its four
// values.
struct CalcRates : public Functor<Double4, const SharedData<Double4>&> {
  Double4 operator()(const SharedData<Double4>& tl_results) override {
    Double4 result;
    auto* sim = Simulation::GetActive();
    for (auto& el : tl_results) {
      result += el;
    }
    // -1 because an additional cell has been added as a workaround
    auto num_agents = sim->GetResourceManager()->GetNumAgents() - 1;
    result /= num_agents;
    result[0] = sim->GetScheduler()->GetSimulatedSteps();
    return result;
  }
};*/

int Simulate(int argc, const char** argv) {
  Simulation simulation(argc, argv);

  // Define initial model - in this example: single cell at origin
  //auto* rm = simulation.GetResourceManager();
  //auto* cell = new Cell(30);
  //rm->AddAgent(cell);

  // Randomly initialize a population
  auto random = simulation.GetRandom();
  random->SetSeed(1234);
  initialize_population(random, 50);

  // Test population cout
  // std::cout << sizeof(Population) << std::endl;
  // Population pop;
  // std::cout << pop;

  
  // Get population statistics
  auto* get_statistics = NewOperation("ReductionOpPopulation");
  auto* get_statistics_impl = get_statistics->GetImplementation<ReductionOp<Population>>();
  get_statistics_impl->Initialize(new get_thread_local_population_statistics(), new add_thread_local_populations());
  auto* scheduler = simulation.GetScheduler();
  scheduler->ScheduleOp(get_statistics);

  // Don't compute forces
  scheduler->UnscheduleOp(scheduler->GetOps("mechanical forces")[0]);

  // Run simulation for one timestep
  simulation.GetScheduler()->Simulate(4);

  const auto& pop2 = get_statistics_impl->GetResults();
  std::cout << "no populations: " << pop2.size() << std::endl;
  for (auto& p : pop2){
    std::cout << p;
  }

  // DEBUG: check number of agents
  auto* rm = simulation.GetResourceManager();
  std::cout << "Simulation considers " << rm->GetNumAgents() << " persons.\n";

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace bdm
