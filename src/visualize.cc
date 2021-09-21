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

namespace bdm {

int PlotAndSaveTimeseries() {
  // Get pointers for simulation and TimeSeries data
  auto sim = Simulation::GetActive();
  auto *ts = sim->GetTimeSeries();

  // Create a new folder (build/)output/<data_time> to store the results of
  // the specific simulation run.
  time_t rawtime;
  struct tm *timeinfo;
  char buffer[80];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, sizeof(buffer), "%Y-%m-%d-%H:%M:%S", timeinfo);
  std::string time_stamp(buffer);
  // Define create_folder command for ROOT
  std::string create_folder =
      Concat("mkdir ", sim->GetOutputDir(), "/", time_stamp);
  // Create folder with ROOT
  gSystem->Exec(&create_folder[0]);

  // Save the TimeSeries Data as JSON to the folder <date_time>
  ts->SaveJson(Concat(sim->GetOutputDir(), "/", time_stamp, "/data.json"));

  // Create a bdm LineGraph that visualizes the TimeSeries data
  bdm::experimental::LineGraph g(ts, "my result", "Time", "Number of agents",
                                 true);
  g.Add("healthy_agents", "Healthy", "L", kBlue, 1.0);
  g.Add("infected_agents", "HIV", "L", kRed, 1.0);
  g.Draw();
  g.SaveAs(Concat(sim->GetOutputDir(), "/", time_stamp, "/simulation_hiv"),
           {".svg", ".png"});

  // Create a bdm LineGraph that visualizes the TimeSeries data
  bdm::experimental::LineGraph g2(ts, "my result", "Time", "Number of agents",
                                   true);
  g2.Add("healthy_agents", "Healthy", "L", kBlue, 1.0, 1);
  g2.Add("infected_agents", "HIV", "L", kOrange, 1.0, 1);
  g2.Add("acute_agents", "Acute", "L", kRed, 1.0, 10);
  g2.Add("chronic_agents", "Chronic", "L", kMagenta, 1.0, 10);
  g2.Add("treated_agents", "Treated", "L", kGreen, 1.0, 10);
  g2.Add("failing_agents", "Failing", "L", kGray, 1.0, 10);
  g2.Add("prevalence", "Prevalence", "L", kOrange, 1.0, 3, 1, kOrange, 1.0, 5);
  g2.Add("incidence", "Incidence", "L", kRed, 1.0, 3, 1, kRed, 1.0, 5);

  g2.Draw();
  g2.SaveAs(Concat(sim->GetOutputDir(), "/", time_stamp, "/simulation_hiv_with_states"),
             {".svg", ".png"});
    
  // Create a bdm LineGraph that visualizes the TimeSeries data
  bdm::experimental::LineGraph g3(ts, "my result", "Time", "Number of agents",
                                     true);
  g3.Add("prevalence", "Prevalence", "L", kOrange, 1.0, 3, 1, kOrange, 1.0, 5);
  g3.Add("incidence", "Incidence", "L", kRed, 1.0, 3, 1, kRed, 1.0, 5);

  g3.Draw();
  g3.SaveAs(Concat(sim->GetOutputDir(), "/", time_stamp, "/simulation_hiv_prevalence_incidence"),
               {".svg", ".png"});
    
  // Print info for user to let him/her know where to find simulation results
  std::string info =
      Concat("<PlotAndSaveTimeseries> ", "Results of simulation were saved to ",
             sim->GetOutputDir(), "/", time_stamp, "/");
  std::cout << "Info: " << info << std::endl;

  return 0;
}

}  // namespace bdm
