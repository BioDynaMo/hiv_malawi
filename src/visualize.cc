#include <iostream>
#include <numeric>
#include <vector>

#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"

#include "datatypes.h"

namespace bdm {

int plot_evolution(std::vector<Population> populations) {
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
  // Define canvas
  TCanvas canvas;
  canvas.SetCanvasSize(600, 250);
  canvas.SetGrid();
  // Define Multigraph environment
  TMultiGraph multigraph;
  multigraph.SetTitle("HIV in Malawi;time step;number of people");
  // Define Graph for healthy people and add to multi graph
  TGraph *THealthy = new TGraph(sim_step.size(), &(sim_step[0]), &(healthy[0]));
  THealthy->SetLineColor(kBlue);
  THealthy->SetMarkerColor(kBlue);
  multigraph.Add(THealthy, "AC");
  // Define Graph for infected people and add to multi graph
  TGraph *TInfected =
      new TGraph(sim_step.size(), &(sim_step[0]), &(infected[0]));
  TInfected->SetLineColor(kRed);
  TInfected->SetMarkerColor(kRed);
  multigraph.Add(TInfected, "AC");
  // Draw and save output
  multigraph.Draw("A");
  canvas.SaveAs("simulation_hiv.svg");
  return 1;
}

}  // namespace bdm