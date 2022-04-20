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

#include <ctime>
#include <iostream>
#include <numeric>
#include <vector>

#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TSystem.h"

#include "datatypes.h"

#include "biodynamo.h"
#include "core/util/log.h"
#include "person.h"
#include "sim-param.h"

namespace bdm {
namespace hiv_malawi {

using experimental::Counter;

void DefineAndRegisterCollectors() {
  // Get population statistics, i.e. extract data from simulation
  // Get the pointer to the TimeSeries
  auto* ts = Simulation::GetActive()->GetTimeSeries();
  
  // Define how to get the time values of the TimeSeries
  auto get_year = [](Simulation* sim) {
    // AM: Starting Year Variable set in sim_params
    int start_year = sim->GetParam()->Get<SimParam>()->start_year;

    return static_cast<double>(start_year +
                               sim->GetScheduler()->GetSimulatedSteps());
  };
  
  // Define how to count the healthy individuals
  auto healthy = [](Agent* a) {
    auto* person = bdm_static_cast<Person*>(a);
    return person->IsHealthy();
  };
  ts->AddCollector("healthy_agents", new Counter<double>(healthy), get_year);

  // Define how to count the infected individuals
  auto infected = [](Agent* a) {
    auto* person = bdm_static_cast<Person*>(a);
    return !(person->IsHealthy());
  };
  ts->AddCollector("infected_agents", new Counter<double>(infected), get_year);

  // AM: Define how to count the infected acute individuals
  auto acute = [](Agent* a) {
    auto* person = bdm_static_cast<Person*>(a);
    return person->IsAcute();
  };
  ts->AddCollector("acute_agents", new Counter<double>(acute), get_year);
  
  // AM: Define how to count the infected chronic individuals
  auto chronic = [](Agent* a) {
    auto* person = bdm_static_cast<Person*>(a);
    return person->IsChronic();
  };
  ts->AddCollector("chronic_agents", new Counter<double>(chronic), get_year);
  
  // AM: Define how to count the infected treated individuals
  auto treated = [](Agent* a) {
    auto* person = bdm_static_cast<Person*>(a);
    return person->IsTreated();
  };
  ts->AddCollector("treated_agents", new Counter<double>(treated), get_year);

  // AM: Define how to count the infected failing individuals
  auto failing = [](Agent* a) {
    auto* person = bdm_static_cast<Person*>(a);
    return person->IsFailing();
  };
  ts->AddCollector("failing_agents", new Counter<double>(failing), get_year);

  // AM: Define how to count the individuals infected at birth
  auto mtct = [](Agent* a) {
    auto* person = bdm_static_cast<Person*>(a);
    return person->MTCTransmission();
  };
  ts->AddCollector("mtct_agents", new Counter<double>(mtct), get_year);

  // AM: Define how to count the individuals infected through casual mating
  auto casual = [](Agent* a) {
    auto* person = bdm_static_cast<Person*>(a);
    return person->CasualTransmission();
  };
  ts->AddCollector("casual_transmission_agents", new Counter<double>(casual),
                   get_year);

  // AM: Define how to count the individuals infected through regular mating
  auto regular = [](Agent* a) {
    auto* person = bdm_static_cast<Person*>(a);
    return person->RegularTransmission();
  };
  ts->AddCollector("regular_transmission_agents", new Counter<double>(regular),
                   get_year);

  // AM: Define how to count the individuals that were infected by an Acute HIV
  // partner/Mother
  auto acute_transmission = [](Agent* a) {
    auto* person = bdm_static_cast<Person*>(a);
    return person->AcuteTransmission();
  };
  ts->AddCollector("acute_transmission", new Counter<double>(acute_transmission), 
                   get_year);

  // AM: Define how to count the individuals that were infected by an Chronic
  // HIV partner/Mother
  auto chronic_transmission = [](Agent* a) {
    auto* person = bdm_static_cast<Person*>(a);
    return person->ChronicTransmission();
  };
  ts->AddCollector("chronic_transmission", new Counter<double>(chronic_transmission),
                   get_year);

  // AM: Define how to count the individuals that were infected by an Treated
  // HIV partner/Mother
  auto treated_transmission = [](Agent* a) {
    auto* person = bdm_static_cast<Person*>(a);
    return person->TreatedTransmission();
  };
  ts->AddCollector("treated_transmission", new Counter<double>(treated_transmission),
                   get_year);

  // AM: Define how to count the individuals that were infected by an Failing
  // HIV partner/Mother
  auto failing_transmission = [](Agent* a) {
    auto* person = bdm_static_cast<Person*>(a);
    return person->FailingTransmission();
  };
  ts->AddCollector("failing_transmission", new Counter<double>(failing_transmission),
                   get_year);

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

  
  // AM: Added detailed follow up of HIV states time series
  
  
  
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
}

// -----------------------------------------------------------------------------
int PlotAndSaveTimeseries() {
  // Get pointers for simulation and TimeSeries data
  auto sim = Simulation::GetActive();
  auto *ts = sim->GetTimeSeries();

  // Save the TimeSeries Data as JSON to the folder <date_time>
  ts->SaveJson(Concat(sim->GetOutputDir(), "/data.json"));

  // Create a bdm LineGraph that visualizes the TimeSeries data
  bdm::experimental::LineGraph g(ts, "Population - Healthy/Infected", "Time",
                                 "Number of agents", true);
  g.Add("healthy_agents", "Healthy", "L", kBlue, 1.0);
  g.Add("infected_agents", "HIV", "L", kRed, 1.0);
  g.Draw();
  g.SaveAs(Concat(sim->GetOutputDir(), "/simulation_hiv"),
           {".svg", ".png"});

  // Create a bdm LineGraph that visualizes the TimeSeries data
  bdm::experimental::LineGraph g2(ts, "HIV stages", "Time", "Number of agents",
                                  true);
  // g2.Add("healthy_agents", "Healthy", "L", kBlue, 1.0, 1);
  g2.Add("infected_agents", "HIV", "L", kOrange, 1.0, 1);
  g2.Add("acute_agents", "Acute", "L", kRed, 1.0, 10);
  g2.Add("chronic_agents", "Chronic", "L", kMagenta, 1.0, 10);
  g2.Add("treated_agents", "Treated", "L", kGreen, 1.0, 10);
  g2.Add("failing_agents", "Failing", "L", kGray, 1.0, 10);
  g2.Draw();
  g2.SaveAs(Concat(sim->GetOutputDir(),
                   "/simulation_hiv_with_states"),
            {".svg", ".png"});

  // Create a bdm LineGraph that visualizes the TimeSeries data
  bdm::experimental::LineGraph g2_2(ts, "Transmission", "Time",
                                    "Number of agents", true);
  g2_2.Add("mtct_agents", "MTCT", "L", kGreen, 1.0, 3);
  g2_2.Add("casual_transmission_agents", "Casual Transmission", "L", kRed, 1.0,
           3);
  g2_2.Add("regular_transmission_agents", "Regular Transmission", "L", kBlue,
           1.0, 3);
  g2_2.Draw();
  g2_2.SaveAs(Concat(sim->GetOutputDir(),
                     "/simulation_transmission_types"),
              {".svg", ".png"});

  // Create a bdm LineGraph that visualizes the TimeSeries data
  bdm::experimental::LineGraph g2_3(ts, "Source of infection - HIV stage",
                                    "Time", "Number of agents", true);
  g2_3.Add("acute_transmission", "Infected by Acute", "L", kRed, 1.0, 10);
  g2_3.Add("chronic_transmission", "Infected by Chronic", "L", kMagenta, 1.0,
           10);
  g2_3.Add("treated_transmission", "Infected by Treated", "L", kGreen, 1.0, 10);
  g2_3.Add("failing_transmission", "Infected by Failing", "L", kGray, 1.0, 10);
  g2_3.Draw();
  g2_3.SaveAs(Concat(sim->GetOutputDir(),
                     "/simulation_transmission_sources"),
              {".svg", ".png"});
  // Create a bdm LineGraph that visualizes the TimeSeries data
  bdm::experimental::LineGraph g3(ts, "HIV", "Time", "", true);
  g3.Add("prevalence", "Prevalence", "L", kOrange, 1.0, 3, 1, kOrange, 1.0, 5);
  g3.Add("prevalence_women", "Prevalence - Women", "L", kRed, 1.0, 3, 1, kRed,
         1.0, 10);
  g3.Add("prevalence_men", "Prevalence - Men", "L", kBlue, 1.0, 3, 1, kBlue,
         1.0, 10);

  g3.Add("prevalence_15_49", "Prevalence (15-49)", "L", kOrange, 1.0, 1, 1);
  g3.Add("prevalence_women_15_49", "Prevalence - Women (15-49)", "L", kRed, 1.0,
         1, 1);
  g3.Add("prevalence_men_15_49", "Prevalence - Men (15-49)", "L", kBlue, 1.0, 1,
         1);

  g3.Add("incidence", "Incidence", "L", kRed, 1.0, 3, 1, kRed, 1.0, 5);

  g3.Draw();
  g3.SaveAs(Concat(sim->GetOutputDir(),
                   "/simulation_hiv_prevalence_incidence"),
            {".svg", ".png"});

  // Create a bdm LineGraph that visualizes the TimeSeries data
  bdm::experimental::LineGraph g4(ts, "my result", "Time", "Proportion", true);
  g4.Add("high_risk_sb_hiv", "High Risk SB - HIV", "L", kRed, 1.0, 1);
  g4.Add("low_risk_sb_hiv", "Low Risk SB - HIV", "L", kBlue, 1.0, 1);
  g4.Add("high_risk_sb_healthy", "High Risk SB - Healthy", "L", kOrange, 1.0,
         1);
  g4.Add("low_risk_sb_healthy", "Low Risk SB - Healthy", "L", kGreen, 1.0, 1);
  g4.Add("high_risk_sb_hiv_women", "High Risk SB - HIV Women", "L", kRed, 1.0,
         10);
  g4.Add("low_risk_sb_hiv_women", "Low Risk SB - HIV Women", "L", kBlue, 1.0,
         10);

  g4.Add("high_risk_sb_hiv_men", "High Risk SB - HIV Men", "L", kRed, 1.0, 2);
  g4.Add("low_risk_sb_hiv_men", "Low Risk SB - HIV Men", "L", kBlue, 1.0, 2);

  g4.Add("high_risk_sb_healthy_women", "High Risk SB - Healthy Women", "L",
         kOrange, 1.0, 10);
  g4.Add("low_risk_sb_healthy_women", "Low Risk SB - Healthy Women", "L",
         kGreen, 1.0, 10);

  g4.Add("high_risk_sb_healthy_men", "High Risk SB - Healthy Men", "L", kOrange,
         1.0, 2);
  g4.Add("low_risk_sb_healthy_men", "Low Risk SB - Healthy Men", "L", kGreen,
         1.0, 2);
  g4.Draw();
  g4.SaveAs(Concat(sim->GetOutputDir(),
                   "/simulation_sociobehaviours"),
            {".svg", ".png"});

  // Create a bdm LineGraph that visualizes the TimeSeries data
  bdm::experimental::LineGraph g5(ts, "Casual sex partners", "Time", "Number",
                                  true);

  g5.Add("mean_nocas_men_low_sb", "Mean - Men w/ Low Risk SB", "L", kGreen, 1.0,
         2);
  g5.Add("mean_nocas_men_high_sb", "Mean - Men w/ High Risk SB", "L", kRed, 1.0,
         2);
  g5.Add("mean_nocas_women_low_sb", "Mean - Women w/ Low Risk SB", "L", kGreen,
         1.0, 1);
  g5.Add("mean_nocas_women_high_sb", "Mean - Women w/ High Risk SB", "L", kRed,
         1.0, 1);

  g5.Draw();
  g5.SaveAs(Concat(sim->GetOutputDir(),
                   "/simulation_casual_mating_mean"),
            {".svg", ".png"});

  // Create a bdm LineGraph that visualizes the TimeSeries data
  bdm::experimental::LineGraph g6(ts, "Casual sex partners", "Time", "Number",
                                  true);

  g6.Add("total_nocas_men_low_sb", "Total - Men w/ Low Risk SB", "L", kGreen,
         1.0, 2);
  g6.Add("total_nocas_men_high_sb", "Total - Men w/ High Risk SB", "L", kRed,
         1.0, 2);
  g6.Add("total_nocas_women_low_sb", "Total - Women w/ Low Risk SB", "L",
         kGreen, 1.0, 1);
  g6.Add("total_nocas_women_high_sb", "Total - Women w/ High Risk SB", "L",
         kRed, 1.0, 1);

  g6.Draw();
  g6.SaveAs(Concat(sim->GetOutputDir(),
                   "/simulation_casual_mating_total"),
            {".svg", ".png"});

  // Print info for user to let him/her know where to find simulation results
  std::string info =
      Concat("<PlotAndSaveTimeseries> ", "Results of simulation were saved to ",
             sim->GetOutputDir(), "/");
  std::cout << "Info: " << info << std::endl;

  return 0;
}

}  // namespace hiv_malawi
}  // namespace bdm
