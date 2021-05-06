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
#include <numeric>
#include <vector>

#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"

#include "datatypes.h"

namespace bdm {

int plot_evolution(const std::vector<Population> &populations) {
  // Extract infromation from populations
  std::vector<double> healthy(populations.size());
  std::vector<double> infected(populations.size());
  std::vector<double> sim_step(
      populations.size());  // double for ROOT compatib.
  for (int i = 0; i < populations.size(); i++) {
    healthy[i] = populations[i].healthy_male + populations[i].healthy_female;
    infected[i] = std::accumulate(populations[i].infected_male.begin(),
                                  populations[i].infected_male.end(), 0) +
                  std::accumulate(populations[i].infected_female.begin(),
                                  populations[i].infected_female.end(), 0);
    sim_step[i] = i;
  }
  // Define canvas, see ROOT documentation for details. Note that ROOT is fully
  // included in BioDynaMo and can be used to analyze the simulation data.
  TCanvas canvas;
  canvas.SetCanvasSize(600, 250);
  canvas.SetGrid();
  // Define Multigraph, see ROOT documentation
  TMultiGraph multigraph;
  multigraph.SetTitle("HIV in Malawi;simulation steps;number of people");
  // Define Graph (ROOT) for healthy people and add to multi graph
  TGraph *THealthy = new TGraph(sim_step.size(), &(sim_step[0]), &(healthy[0]));
  THealthy->SetLineColor(kBlue);
  THealthy->SetMarkerColor(kBlue);
  multigraph.Add(THealthy, "AC");
  // Define Graph (ROOT) for infected people and add to multi graph
  TGraph *TInfected =
      new TGraph(sim_step.size(), &(sim_step[0]), &(infected[0]));
  TInfected->SetLineColor(kRed);
  TInfected->SetMarkerColor(kRed);
  multigraph.Add(TInfected, "AC");
  // Draw and save output
  multigraph.Draw("A");
  canvas.SaveAs("simulation_hiv.svg");
  return 0;
}

}  // namespace bdm