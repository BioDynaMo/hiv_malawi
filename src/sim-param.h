#ifndef SIM_PARAM_H_
#define SIM_PARAM_H_

#include <vector>
#include "biodynamo.h"

namespace bdm {

/// This class defines parameters that are specific to this simulation.
struct SimParam : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(SimParam, 1);

  // The number of iterations that BioDynaMo simulates. (#iterations = #years)
  uint64_t number_of_iterations = 60; // (1960-2060)

  // Number of agents that are present at the first iteration of the simulation
  uint64_t initial_population_size = 3600000;

//   // Currently this variable is not used, could be used to set a random_seed 
//   // for BioDynaMo's random generator.
//   uint64_t random_seed = 1234;

  // Age when agents start to engage in sexual activities, e.g. possibly give 
  // birth, infect, or get infected
  int min_age = 15;

  // Age when agents stop to engage in sexual activities, e.g. possibly give 
  // birth, infect, or get infected
  int max_age = 40;

  // Maximum age that an agent can reach. Once the simulation is calibrated and 
  // the parameters are fitted, one may consider removing all healthy agents 
  // from the simulation that are older than max_age.
  int age_of_death = 90;

  // The migration of agents is modeled by a random process. We sample from a 
  // Gausian distribution to determine to which neighbouring category an agent 
  // will migrate or if it stay at the same place. The parameters of the 
  // Gaussian are:
  float migration_mean = 0.0;
  float migration_sigma = 2.0;

  // The mating behaviour is modeled with a random process. For each male agent,
  // we sample the number of female sex partners per year from a Gaussian 
  // distribution, which parameters are:
  float no_mates_mean = 2.0;
  float no_mates_sigma = 1.0;

  // Death is modeled by a random process. We generate a random number r in
  // [0,1] and check if: r< (age - min) \ (delta * alpha). If that evaluates to
  // true, the agent dies. For healthy and hiv infected individuals, we use
  // different parameters.
  float min_healthy = 45.;
  float delta_healthy = 75.;
  float alpha_healthy = 8.;
  float min_hiv = 5.;
  float delta_hiv = 45.;
  float alpha_hiv = 8.;

  // Probability of getting infected when having sex with infected agent
  float infection_probability = 0.1;

  // Probability for agent to be infected at beginning of simulation. You can 
  // expect roughly <initial_population_size * initial_infection_probability>
  // infected agents at the beginning of the simulation. These infected 
  // individuals can occur anywhere on the age spectrum.
  float initial_infection_probability = 0.01;

  // Parameter 0.18 is chosen because our GiveBirth Behaviour is based on a
  // Bernoulli experiment. A binomial distribuition peaks at around 6 for 25
  // tries (=40-15) and a birth probability of 0.24. This corresponds to the
  // typical birth rate in the region. We substracted 0.06 to account for child 
  // motability and reach a realistic demographic development from 1960-2020.
  float give_birth_probability = 0.18;

  // Probability for agent to be infected at birth, if its mother is infected
  float birth_infection_probability = 0.1;

  // Probability of creating a male agent, used in population initialization and
  // giving birth
  float probability_male = 0.499;

  // Assign a risk factor of 1 with the following probabilities
  float sociobehavioural_risk_probability = 0.05;
  float biomedical_risk_probability = 0.05;

  // Age distribution for population initialization. Given in a summed up form.
  // Each vector component corresponds to a age bin of 5 years, e.g. x1 (0-5),
  // x2 (6-10). Usually, these vectors are given in probabilities p1, p2, p3, ..
  // Here: x1 = p1, x2 = p1+p2, x3 = p1+p2+p3, ..
  const std::vector<float> male_age_distribution{
      0.156, 0.312, 0.468, 0.541, 0.614, 0.687, 0.76,  0.833, 0.906,
      0.979, 0.982, 0.985, 0.988, 0.991, 0.994, 0.997, 1};
  const std::vector<float> female_age_distribution{
      0.156, 0.312, 0.468, 0.54,  0.612, 0.684, 0.756, 0.828, 0.9,
      0.972, 0.976, 0.98,  0.984, 0.988, 0.992, 0.996, 1};
  
  // Location distribution for population initialization, same logic as for age
  // distribution.
  const std::vector<float> location_distribution{
      0.012, 0.03,  0.031, 0.088, 0.104, 0.116, 0.175, 0.228, 0.273, 0.4,
      0.431, 0.453, 0.498, 0.517, 0.54,  0.569, 0.645, 0.679, 0.701, 0.736,
      0.794, 0.834, 0.842, 0.86,  0.903, 0.925, 0.995, 1,     1};
  
};

}  // namespace bdm

#endif  // SIM_PARAM_H_
