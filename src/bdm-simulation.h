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

#ifndef BDM_SIMULAION_H_
#define BDM_SIMULAION_H_

#include <fstream>
#include <iostream>

#include "core/operation/operation_registry.h"
#include "core/operation/reduction_op.h"

#include "categorical-environment.h"
#include "custom-operations.h"
#include "population-initialization.h"
#include "sim-param.h"
#include "visualize.h"

namespace bdm {
namespace hiv_malawi {

////////////////////////////////////////////////////////////////////////////////
// BioDynaMo's main simulation
////////////////////////////////////////////////////////////////////////////////
inline int Simulate(int argc, const char** argv) {
  // Register the Siulation parameter
  Param::RegisterParamGroup(new SimParam());

  // Initialize the Simulation
  auto set_param = [&](Param* param) {
    param->show_simulation_step = 1;
    param->remove_output_dir_contents = false;
  };
  Simulation simulation(argc, argv, set_param);

  // Get a pointer to the param object
  auto* param = simulation.GetParam();
  // Get a pointer to an instance of SimParam
  auto* sparam = param->Get<SimParam>();

  // AM: Construct Environment with numbers of age and socio-behavioral
  // categories.
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

  // AM: Define how to count the individuals infected at birth
  auto count_MTC_transmission = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_MTCT = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->MTCTransmission();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond_MTCT));
  };

  // AM: Define how to count the individuals infected through casual mating
  auto count_casual_transmission = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_casual = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->CasualTransmission();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond_casual));
  };

  // AM: Define how to count the individuals infected through regular mating
  auto count_regular_transmission = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_regular = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->RegularTransmission();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond_regular));
  };

  // AM: Define how to count the individuals that were infected by an Acute HIV
  // partner/Mother
  auto count_acute_transmission = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->AcuteTransmission();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond));
  };

  // AM: Define how to count the individuals that were infected by an Chronic
  // HIV partner/Mother
  auto count_chronic_transmission = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->ChronicTransmission();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond));
  };

  // AM: Define how to count the individuals that were infected by an Treated
  // HIV partner/Mother
  auto count_treated_transmission = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->TreatedTransmission();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond));
  };

  // AM: Define how to count the individuals that were infected by an Failing
  // HIV partner/Mother
  auto count_failing_transmission = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->FailingTransmission();
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond));
  };

  // Define how to compute mean number of casual partners for males with
  // low-risk sociobehaviours
  auto mean_nocas_men_low_sb = [](Simulation* sim) {
    // Condition for Count operation, check if the person is a male with
    // low-risk behaviours.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return (person->IsMale() && person->IsAdult() && person->age_ < 50 &&
              person->HasLowRiskSocioBehav());
    });
    // Sum agents data
    auto sum_data = L2F([](Agent* agent, uint64_t* tl_result) {
      *tl_result += bdm_static_cast<Person*>(agent)->no_casual_partners_;
    });
    SumReduction<uint64_t> combine_tl_results;
    return static_cast<double>(bdm::experimental::Reduce(
               sim, sum_data, combine_tl_results, &cond)) /
           static_cast<double>(bdm::experimental::Count(sim, cond));
  };

  // Define how to compute total number of casual partners for males with
  // low-risk sociobehaviours
  auto total_nocas_men_low_sb = [](Simulation* sim) {
    // Condition for Count operation, check if the person is a male with
    // low-risk behaviours.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return (person->IsMale() && person->IsAdult() && person->age_ < 50 &&
              person->HasLowRiskSocioBehav());
    });
    // Sum agents data
    auto sum_data = L2F([](Agent* agent, uint64_t* tl_result) {
      *tl_result += bdm_static_cast<Person*>(agent)->no_casual_partners_;
    });
    SumReduction<uint64_t> combine_tl_results;
    return static_cast<double>(
        bdm::experimental::Reduce(sim, sum_data, combine_tl_results, &cond));
  };

  // Define how to compute mean number of casual partners for males with
  // lhighow-risk sociobehaviours
  auto mean_nocas_men_high_sb = [](Simulation* sim) {
    // Condition for Count operation, check if the person is a male with
    // high-risk behaviours.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return (person->IsMale() && person->IsAdult() && person->age_ < 50 &&
              person->HasHighRiskSocioBehav());
    });
    // Sum agents data
    auto sum_data = L2F([](Agent* agent, uint64_t* tl_result) {
      *tl_result += bdm_static_cast<Person*>(agent)->no_casual_partners_;
    });
    SumReduction<uint64_t> combine_tl_results;
    return static_cast<double>(bdm::experimental::Reduce(
               sim, sum_data, combine_tl_results, &cond)) /
           static_cast<double>(bdm::experimental::Count(sim, cond));
  };

  // Define how to compute mean number of casual partners for males with
  // high-risk sociobehaviours
  auto total_nocas_men_high_sb = [](Simulation* sim) {
    // Condition for Count operation, check if the person is a male with
    // high-risk behaviours.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return (person->IsMale() && person->IsAdult() && person->age_ < 50 &&
              person->HasHighRiskSocioBehav());
    });
    // Sum agents data
    auto sum_data = L2F([](Agent* agent, uint64_t* tl_result) {
      *tl_result += bdm_static_cast<Person*>(agent)->no_casual_partners_;
    });
    SumReduction<uint64_t> combine_tl_results;
    return static_cast<double>(
        bdm::experimental::Reduce(sim, sum_data, combine_tl_results, &cond));
  };

  // Define how to compute mean number of casual partners for females with
  // low-risk sociobehaviours
  auto mean_nocas_women_low_sb = [](Simulation* sim) {
    // Condition for Count operation, check if the person is a male with
    // low-risk behaviours.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return (person->IsFemale() && person->IsAdult() && person->age_ < 50 &&
              person->HasLowRiskSocioBehav());
    });
    // Sum agents data
    auto sum_data = L2F([](Agent* agent, uint64_t* tl_result) {
      *tl_result += bdm_static_cast<Person*>(agent)->no_casual_partners_;
    });
    SumReduction<uint64_t> combine_tl_results;
    return static_cast<double>(bdm::experimental::Reduce(
               sim, sum_data, combine_tl_results, &cond)) /
           static_cast<double>(bdm::experimental::Count(sim, cond));
  };

  // Define how to compute mean number of casual partners for males with
  // low-risk sociobehaviours
  auto total_nocas_women_low_sb = [](Simulation* sim) {
    // Condition for Count operation, check if the person is a male with
    // low-risk behaviours.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return (person->IsFemale() && person->IsAdult() && person->age_ < 50 &&
              person->HasLowRiskSocioBehav());
    });
    // Sum agents data
    auto sum_data = L2F([](Agent* agent, uint64_t* tl_result) {
      *tl_result += bdm_static_cast<Person*>(agent)->no_casual_partners_;
    });
    SumReduction<uint64_t> combine_tl_results;
    return static_cast<double>(
        bdm::experimental::Reduce(sim, sum_data, combine_tl_results, &cond));
  };

  // Define how to compute mean number of casual partners for females with
  // high-risk sociobehaviours
  auto mean_nocas_women_high_sb = [](Simulation* sim) {
    // Condition for Count operation, check if the person is a male with
    // high-risk behaviours.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return (person->IsFemale() && person->IsAdult() && person->age_ < 50 &&
              person->HasHighRiskSocioBehav());
    });
    // Sum agents data
    auto sum_data = L2F([](Agent* agent, uint64_t* tl_result) {
      *tl_result += bdm_static_cast<Person*>(agent)->no_casual_partners_;
    });
    SumReduction<uint64_t> combine_tl_results;
    return static_cast<double>(bdm::experimental::Reduce(
               sim, sum_data, combine_tl_results, &cond)) /
           static_cast<double>(bdm::experimental::Count(sim, cond));
  };

  // Define how to compute mean number of casual partners for males with
  // lhighow-risk sociobehaviours
  auto total_nocas_women_high_sb = [](Simulation* sim) {
    // Condition for Count operation, check if the person is a female with
    // high-risk behaviours.
    auto cond = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return (person->IsFemale() && person->IsAdult() && person->age_ < 50 &&
              person->HasHighRiskSocioBehav());
    });
    // Sum agents data
    auto sum_data = L2F([](Agent* agent, uint64_t* tl_result) {
      *tl_result += bdm_static_cast<Person*>(agent)->no_casual_partners_;
    });
    SumReduction<uint64_t> combine_tl_results;
    return static_cast<double>(
        bdm::experimental::Reduce(sim, sum_data, combine_tl_results, &cond));
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

  // AM: Define how to compute general prevalence
  auto pct_prevalence_15_49 = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_infected = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return !(person->IsHealthy()) && person->age_ >= 15 && person->age_ <= 49;
    });
    auto cond_all = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->age_ >= 15 && person->age_ <= 49;
    });
    return static_cast<double>(bdm::experimental::Count(sim, cond_infected)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_all));
  };

  // AM: Define how to compute prevalence among women
  auto pct_prevalence_women = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_infected_women = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return !(person->IsHealthy()) && person->IsFemale();
    });
    auto cond_women = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsFemale();
    });
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_infected_women)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_women));
  };

  // AM: Define how to compute prevalence among women
  auto pct_prevalence_women_15_49 = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_infected_women = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return !(person->IsHealthy()) && person->IsFemale() &&
             person->age_ >= 15 && person->age_ <= 49;
    });
    auto cond_women = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsFemale() && person->age_ >= 15 && person->age_ <= 49;
    });
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_infected_women)) /
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
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_infected_men)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_men));
  };

  // AM: Define how to compute prevalence among men
  auto pct_prevalence_men_15_49 = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_infected_men = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return !(person->IsHealthy()) && person->IsMale() && person->age_ >= 15 &&
             person->age_ <= 49;
    });
    auto cond_men = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsMale() && person->age_ >= 15 && person->age_ <= 49;
    });
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_infected_men)) /
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
  // AM: Define how to compute proportion of people with high-risk
  // socio-beahviours among hiv+
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
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_high_risk_hiv)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_hiv));
  };
  // AM: Define how to compute proportion of people with low-risk
  // socio-beahviours among hiv+
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
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_low_risk_hiv)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_hiv));
  };
  // AM: Define how to compute proportion of people with high-risk
  // socio-beahviours among healthy
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
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_high_risk_healthy)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_healthy));
  };
  // AM: Define how to compute proportion of people with low-risk
  // socio-beahviours among healthy
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
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_low_risk_healthy)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_healthy));
  };
  // AM: Define how to compute proportion of high-risk socio-beahviours among
  // hiv adult women
  auto pct_high_risk_hiv_women = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_high_risk_hiv_women = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->HasHighRiskSocioBehav() and !(person->IsHealthy()) and
             person->IsAdult() and person->IsFemale();
    });
    auto cond_hiv_women = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return !(person->IsHealthy()) and person->IsAdult() and
             person->IsFemale();
    });
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_high_risk_hiv_women)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_hiv_women));
  };
  // AM: Define how to compute proportion of low-risk socio-beahviours among hiv
  // adult women
  auto pct_low_risk_hiv_women = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_low_risk_hiv_women = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->HasLowRiskSocioBehav() and !(person->IsHealthy()) and
             person->IsAdult() and person->IsFemale();
    });
    auto cond_hiv_women = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return !(person->IsHealthy()) and person->IsAdult() and
             person->IsFemale();
    });
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_low_risk_hiv_women)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_hiv_women));
  };
  // AM: Define how to compute proportion of high-risk socio-beahviours among
  // hiv adult men
  auto pct_high_risk_hiv_men = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_high_risk_hiv_men = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->HasHighRiskSocioBehav() and !(person->IsHealthy()) and
             person->IsAdult() and person->IsMale();
    });
    auto cond_hiv_men = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return !(person->IsHealthy()) and person->IsAdult() and person->IsMale();
    });
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_high_risk_hiv_men)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_hiv_men));
  };
  // AM: Define how to compute proportion of low-risk socio-beahviours among hiv
  // adult men
  auto pct_low_risk_hiv_men = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_low_risk_hiv_men = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->HasLowRiskSocioBehav() and !(person->IsHealthy()) and
             person->IsAdult() and person->IsMale();
    });
    auto cond_hiv_men = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return !(person->IsHealthy()) and person->IsAdult() and person->IsMale();
    });
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_low_risk_hiv_men)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_hiv_men));
  };
  // AM: Define how to compute proportion of high-risk socio-beahviours among
  // healthy adult women
  auto pct_high_risk_healthy_women = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_high_risk_healthy_women = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->HasHighRiskSocioBehav() and person->IsHealthy() and
             person->IsAdult() and person->IsFemale();
    });
    auto cond_healthy_women = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsHealthy() and person->IsAdult() and person->IsFemale();
    });
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_high_risk_healthy_women)) /
           static_cast<double>(
               bdm::experimental::Count(sim, cond_healthy_women));
  };
  // AM: Define how to compute proportion of low-risk socio-beahviours among
  // healthy adult women
  auto pct_low_risk_healthy_women = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_low_risk_healthy_women = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->HasLowRiskSocioBehav() and person->IsHealthy() and
             person->IsAdult() and person->IsFemale();
    });
    auto cond_healthy_women = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsHealthy() and person->IsAdult() and person->IsFemale();
    });
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_low_risk_healthy_women)) /
           static_cast<double>(
               bdm::experimental::Count(sim, cond_healthy_women));
  };
  // AM: Define how to compute proportion of high-risk socio-beahviours among
  // healthy adult men
  auto pct_high_risk_healthy_men = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_high_risk_healthy_men = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->HasHighRiskSocioBehav() and person->IsHealthy() and
             person->IsAdult() and person->IsMale();
    });
    auto cond_healthy_men = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsHealthy() and person->IsAdult() and person->IsMale();
    });
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_high_risk_healthy_men)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_healthy_men));
  };
  // AM: Define how to compute proportion of low-risk socio-beahviours among
  // healthy adult men
  auto pct_low_risk_healthy_men = [](Simulation* sim) {
    // Condition for Count operation, e.g. check if person is infected.
    auto cond_low_risk_healthy_men = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->HasLowRiskSocioBehav() and person->IsHealthy() and
             person->IsAdult() and person->IsMale();
    });
    auto cond_healthy_men = L2F([](Agent* a) {
      auto* person = bdm_static_cast<Person*>(a);
      return person->IsHealthy() and person->IsAdult() and person->IsMale();
    });
    return static_cast<double>(
               bdm::experimental::Count(sim, cond_low_risk_healthy_men)) /
           static_cast<double>(bdm::experimental::Count(sim, cond_healthy_men));
  };

  // Define how to get the time values of the TimeSeries
  auto get_year = [](Simulation* sim) {
    // AM: Starting Year Variable set in sim_params
    int start_year = sim->GetParam()->Get<SimParam>()->start_year;

    return static_cast<double>(start_year +
                               sim->GetScheduler()->GetSimulatedSteps());
  };
  ts->AddCollector("healthy_agents", count_healthy, get_year);
  ts->AddCollector("infected_agents", count_infected, get_year);

  // AM: Added detailed follow up of HIV states time series
  ts->AddCollector("acute_agents", count_acute, get_year);
  ts->AddCollector("chronic_agents", count_chronic, get_year);
  ts->AddCollector("treated_agents", count_treated, get_year);
  ts->AddCollector("failing_agents", count_failing, get_year);

  ts->AddCollector("mtct_agents", count_MTC_transmission, get_year);
  ts->AddCollector("casual_transmission_agents", count_casual_transmission,
                   get_year);
  ts->AddCollector("regular_transmission_agents", count_regular_transmission,
                   get_year);

  ts->AddCollector("acute_transmission", count_acute_transmission, get_year);
  ts->AddCollector("chronic_transmission", count_chronic_transmission,
                   get_year);
  ts->AddCollector("treated_transmission", count_treated_transmission,
                   get_year);
  ts->AddCollector("failing_transmission", count_failing_transmission,
                   get_year);

  ts->AddCollector("mean_nocas_men_low_sb", mean_nocas_men_low_sb, get_year);
  ts->AddCollector("mean_nocas_men_high_sb", mean_nocas_men_high_sb, get_year);
  ts->AddCollector("mean_nocas_women_low_sb", mean_nocas_women_low_sb,
                   get_year);
  ts->AddCollector("mean_nocas_women_high_sb", mean_nocas_women_high_sb,
                   get_year);

  ts->AddCollector("total_nocas_men_low_sb", total_nocas_men_low_sb, get_year);
  ts->AddCollector("total_nocas_men_high_sb", total_nocas_men_high_sb,
                   get_year);
  ts->AddCollector("total_nocas_women_low_sb", total_nocas_women_low_sb,
                   get_year);
  ts->AddCollector("total_nocas_women_high_sb", total_nocas_women_high_sb,
                   get_year);

  ts->AddCollector("prevalence", pct_prevalence, get_year);
  ts->AddCollector("prevalence_women", pct_prevalence_women, get_year);
  ts->AddCollector("prevalence_men", pct_prevalence_men, get_year);

  ts->AddCollector("prevalence_15_49", pct_prevalence_15_49, get_year);
  ts->AddCollector("prevalence_women_15_49", pct_prevalence_women_15_49,
                   get_year);
  ts->AddCollector("prevalence_men_15_49", pct_prevalence_men_15_49, get_year);

  ts->AddCollector("incidence", pct_incidence, get_year);

  ts->AddCollector("high_risk_sb_hiv", pct_high_risk_hiv, get_year);
  ts->AddCollector("low_risk_sb_hiv", pct_low_risk_hiv, get_year);
  ts->AddCollector("high_risk_sb_healthy", pct_high_risk_healthy, get_year);
  ts->AddCollector("low_risk_sb_healthy", pct_low_risk_healthy, get_year);

  ts->AddCollector("high_risk_sb_hiv_women", pct_high_risk_hiv_women, get_year);
  ts->AddCollector("low_risk_sb_hiv_women", pct_low_risk_hiv_women, get_year);

  ts->AddCollector("high_risk_sb_hiv_men", pct_high_risk_hiv_men, get_year);
  ts->AddCollector("low_risk_sb_hiv_men", pct_low_risk_hiv_men, get_year);

  ts->AddCollector("high_risk_sb_healthy_women", pct_high_risk_healthy_women,
                   get_year);
  ts->AddCollector("low_risk_sb_healthy_women", pct_low_risk_healthy_women,
                   get_year);

  ts->AddCollector("high_risk_sb_healthy_men", pct_high_risk_healthy_men,
                   get_year);
  ts->AddCollector("low_risk_sb_healthy_men", pct_low_risk_healthy_men,
                   get_year);

  // Unschedule some default operations
  auto* scheduler = simulation.GetScheduler();
  // Don't compute forces
  scheduler->UnscheduleOp(scheduler->GetOps("mechanical forces")[0]);
  // Don't run load balancing, not working with custom environment.
  scheduler->UnscheduleOp(scheduler->GetOps("load balancing")[0]);

  // Add a operation that resets the number of casual partners at the beginning
  // of each iteration
  OperationRegistry::GetInstance()->AddOperationImpl(
      "ResetCasualPartners", OpComputeTarget::kCpu, new ResetCasualPartners());
  auto* reset_casual_partners = NewOperation("ResetCasualPartners");
  scheduler->ScheduleOp(reset_casual_partners, OpType::kPreSchedule);

  // Add operation that extracts arbitrary information at the end of each
  // iteration
  OperationRegistry::GetInstance()->AddOperationImpl(
      "ExtractInformation", OpComputeTarget::kCpu,
      new ReductionOp<PopulationData>());
  auto* extract_information = NewOperation("ExtractInformation");
  auto* extract_information_impl =
      extract_information->GetImplementation<ReductionOp<PopulationData>>();
  extract_information_impl->Initialize(new GetPopulationDataThreadLocal(),
                                       new CombinePopulationData());
  scheduler->ScheduleOp(extract_information);

  // Run simulation for <number_of_iterations> timesteps
  {
    Timing timer_sim("RUNTIME");
    scheduler->Simulate(sparam->number_of_iterations);
  }

  {
    Timing timer_post("RUNTIME POSTPROCESSING:            ");

    // Generate ROOT plot to visualize the number of healthy and infected
    // individuals over time.
    PlotAndSaveTimeseries();
  }

  // Here one could do something with the results of the extracted information
  // e.g. print to stdout or export to some files.
  {
    const std::vector<PopulationData>& panel_data =
        extract_information_impl->GetResults();
    std::ofstream filewriter;
    int local_cntr = 1;
    for (auto& p : panel_data) {
      std::string filename = simulation.GetOutputDir() + "/population_data_" +
                             std::to_string(local_cntr) + ".out";
      std::cout << filename << std::endl;
      filewriter.open(filename);
      p.Print(filewriter);
      filewriter.close();
      local_cntr++;
    }
  }

  // DEBUG - AM - TO DO: Works only when selection depended soloely on locations
  /*env->NormalizeMateLocationFrequencies();
  env->PrintMateLocationFrequencies();*/

  return 0;
}

}  // namespace hiv_malawi
}  // namespace bdm

#endif  // BDM_SIMULAION_H_
