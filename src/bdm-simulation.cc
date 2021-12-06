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

  // AM: Construct Environment with numbers of age and socio-behavioral categories.
  auto* env = new CategoricalEnvironment(
      sparam->min_age, sparam->max_age, sparam->nb_age_categories,
      sparam->nb_locations, sparam->nb_sociobehav_categories);

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
  // AM: Define how to compute general prevalence
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
  // AM: Define how to compute prevalence among women
  auto pct_prevalence_women = [](Simulation* sim) {
      // Condition for Count operation, e.g. check if person is infected.
      auto cond_infected_women = L2F([](Agent* a) {
        auto* person = bdm_static_cast<Person*>(a);
        return !(person->IsHealthy()) and person->IsFemale();
      });
      auto cond_women = L2F([](Agent* a) {
        auto* person = bdm_static_cast<Person*>(a);
        return person->IsFemale();
      });
      return static_cast<double>(bdm::experimental::Count(sim, cond_infected_women)) /
             static_cast<double>(bdm::experimental::Count(sim, cond_women));
  };
  // AM: Define how to compute prevalence among men
  auto pct_prevalence_men = [](Simulation* sim) {
        // Condition for Count operation, e.g. check if person is infected.
        auto cond_infected_men = L2F([](Agent* a) {
          auto* person = bdm_static_cast<Person*>(a);
          return !(person->IsHealthy()) and person->IsMale();
        });
        auto cond_men = L2F([](Agent* a) {
          auto* person = bdm_static_cast<Person*>(a);
          return person->IsMale();
        });
        return static_cast<double>(bdm::experimental::Count(sim, cond_infected_men)) /
               static_cast<double>(bdm::experimental::Count(sim, cond_men));
  };
  // AM: Define how to compute general incidence
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
  // AM: Define how to compute proportion of people with high-risk socio-beahviours among hiv+
  auto pct_high_risk_hiv = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_high_risk_hiv = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->HasHighRiskSocioBehav() and !(person->IsHealthy());
    });
    auto cond_hiv = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return !(person->IsHealthy());
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond_high_risk_hiv)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_hiv));
  };
  // AM: Define how to compute proportion of people with low-risk socio-beahviours among hiv+
  auto pct_low_risk_hiv = [](Simulation* sim) {
      // Condition for Count operation, e.g. check if person is infected.
      auto cond_low_risk_hiv = L2F([](Agent* a) {
        auto* person = bdm_static_cast<Person*>(a);
        return person->HasLowRiskSocioBehav() and !(person->IsHealthy());
      });
      auto cond_hiv = L2F([](Agent* a) {
        auto* person = bdm_static_cast<Person*>(a);
        return !(person->IsHealthy());
      });
      return static_cast<double>(bdm::experimental::Count(sim, cond_low_risk_hiv)) /
             static_cast<double>(bdm::experimental::Count(sim, cond_hiv));
  };
  // AM: Define how to compute proportion of people with high-risk socio-beahviours among healthy
  auto pct_high_risk_healthy = [](Simulation* sim) {
      // Condition for Count operation, e.g. check if person is infected.
      auto cond_high_risk_healthy = L2F([](Agent* a) {
        auto* person = bdm_static_cast<Person*>(a);
        return person->HasHighRiskSocioBehav() and person->IsHealthy();
      });
      auto cond_healthy = L2F([](Agent* a) {
        auto* person = bdm_static_cast<Person*>(a);
        return person->IsHealthy();
      });
      return static_cast<double>(bdm::experimental::Count(sim, cond_high_risk_healthy)) /
             static_cast<double>(bdm::experimental::Count(sim, cond_healthy));
  };
  // AM: Define how to compute proportion of people with low-risk socio-beahviours among healthy
  auto pct_low_risk_healthy = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_low_risk_healthy = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->HasLowRiskSocioBehav() and person->IsHealthy();
    });
    auto cond_healthy = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsHealthy();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond_low_risk_healthy)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_healthy));
  };
  // AM: Define how to compute proportion of high-risk socio-beahviours among hiv adult women
  auto pct_high_risk_hiv_women = [](Simulation* sim) {
        // Condition for Count operation, e.g. check if person is infected.
        auto cond_high_risk_hiv_women = L2F([](Agent* a) {
          auto* person = bdm_static_cast<Person*>(a);
          return person->HasHighRiskSocioBehav() and !(person->IsHealthy()) and person->IsAdult() and person->IsFemale();
        });
        auto cond_hiv_women = L2F([](Agent* a) {
          auto* person = bdm_static_cast<Person*>(a);
          return !(person->IsHealthy()) and person->IsAdult() and person->IsFemale();
        });
        return static_cast<double>(bdm::experimental::Count(sim, cond_high_risk_hiv_women)) /
               static_cast<double>(bdm::experimental::Count(sim, cond_hiv_women));
  };
  // AM: Define how to compute proportion of low-risk socio-beahviours among hiv adult women
  auto pct_low_risk_hiv_women = [](Simulation* sim) {
      // Condition for Count operation, e.g. check if person is infected.
      auto cond_low_risk_hiv_women = L2F([](Agent* a) {
        auto* person = bdm_static_cast<Person*>(a);
          return person->HasLowRiskSocioBehav() and !(person->IsHealthy()) and person->IsAdult() and person->IsFemale();
      });
      auto cond_hiv_women = L2F([](Agent* a) {
        auto* person = bdm_static_cast<Person*>(a);
          return !(person->IsHealthy()) and person->IsAdult() and person->IsFemale();
      });
      return static_cast<double>(bdm::experimental::Count(sim, cond_low_risk_hiv_women)) /
             static_cast<double>(bdm::experimental::Count(sim, cond_hiv_women));
  };
  // AM: Define how to compute proportion of high-risk socio-beahviours among hiv adult men
  auto pct_high_risk_hiv_men = [](Simulation* sim) {
        // Condition for Count operation, e.g. check if person is infected.
        auto cond_high_risk_hiv_men = L2F([](Agent* a) {
          auto* person = bdm_static_cast<Person*>(a);
          return person->HasHighRiskSocioBehav() and !(person->IsHealthy()) and person->IsAdult() and person->IsMale();
        });
        auto cond_hiv_men = L2F([](Agent* a) {
          auto* person = bdm_static_cast<Person*>(a);
          return !(person->IsHealthy()) and person->IsAdult() and person->IsMale();
        });
        return static_cast<double>(bdm::experimental::Count(sim, cond_high_risk_hiv_men)) /
               static_cast<double>(bdm::experimental::Count(sim, cond_hiv_men));
  };
  // AM: Define how to compute proportion of low-risk socio-beahviours among hiv adult men
  auto pct_low_risk_hiv_men = [](Simulation* sim) {
      // Condition for Count operation, e.g. check if person is infected.
      auto cond_low_risk_hiv_men = L2F([](Agent* a) {
        auto* person = bdm_static_cast<Person*>(a);
          return person->HasLowRiskSocioBehav() and !(person->IsHealthy()) and person->IsAdult() and person->IsMale();
      });
      auto cond_hiv_men = L2F([](Agent* a) {
        auto* person = bdm_static_cast<Person*>(a);
          return !(person->IsHealthy()) and person->IsAdult() and person->IsMale();
      });
      return static_cast<double>(bdm::experimental::Count(sim, cond_low_risk_hiv_men)) /
             static_cast<double>(bdm::experimental::Count(sim, cond_hiv_men));
  };
  // Define how to get the time values of the TimeSeries
  auto get_year = [](Simulation* sim) {
    // AM: Starting Year Variable set in sim_params
    int start_year = sim->GetParam()->Get<SimParam>()->start_year;
      
    return static_cast<double>(start_year + sim->GetScheduler()->GetSimulatedSteps());
  };
  ts->AddCollector("healthy_agents", count_healthy, get_year);
  ts->AddCollector("infected_agents", count_infected, get_year);

  // AM: Added detailed follow up of HIV states time series
  ts->AddCollector("acute_agents", count_acute, get_year);
  ts->AddCollector("chronic_agents", count_chronic, get_year);
  ts->AddCollector("treated_agents", count_treated, get_year);
  ts->AddCollector("failing_agents", count_failing, get_year);
    
  ts->AddCollector("prevalence", pct_prevalence, get_year);
  ts->AddCollector("prevalence_women", pct_prevalence_women, get_year);
  ts->AddCollector("prevalence_men", pct_prevalence_men, get_year);
    
  ts->AddCollector("incidence", pct_incidence, get_year);
    
  ts->AddCollector("high_risk_sb_hiv", pct_high_risk_hiv, get_year);
  ts->AddCollector("low_risk_sb_hiv", pct_low_risk_hiv, get_year);
  ts->AddCollector("high_risk_sb_healthy", pct_high_risk_healthy, get_year);
  ts->AddCollector("low_risk_sb_healthy", pct_low_risk_healthy, get_year);
    
  ts->AddCollector("high_risk_sb_hiv_women", pct_high_risk_hiv_women, get_year);
  ts->AddCollector("low_risk_sb_hiv_women", pct_low_risk_hiv_women, get_year);
    
  ts->AddCollector("high_risk_sb_hiv_men", pct_high_risk_hiv_men, get_year);
  ts->AddCollector("low_risk_sb_hiv_men", pct_low_risk_hiv_men, get_year);
    
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
