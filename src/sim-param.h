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

#ifndef SIM_PARAM_H_
#define SIM_PARAM_H_

#include <vector>
#include "biodynamo.h"
#include "datatypes.h" //AM: Added to access GemState Enum

namespace bdm {

/// This class defines parameters that are specific to this simulation.
struct SimParam : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(SimParam, 1);

  // The number of iterations that BioDynaMo simulates. (#iterations = #years)
  uint64_t number_of_iterations = 60;  // (1960-2020)

  // Number of agents that are present at the first iteration of the simulation
  uint64_t initial_population_size = 3600000;

  //   // Currently this variable is not used, could be used to set a
  //   random_seed
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
  float no_mates_mean = 80.0;//100.0;//AM replaced 2.0 by 100, if considered as casual sex acts; TO DO: Should probably depend of soc-behav risk factor!
  float no_mates_sigma = 100.0;//20.0;
  
  // Death is modeled by a random process. We generate a random number r in
  // [0,1] and check if: r< (age - min) \ (delta * alpha). If that evaluates to
  // true, the agent dies. For healthy and hiv infected individuals, we use
  // different parameters.
  /*float min_healthy = 45.;
  float delta_healthy = 75.;
  float alpha_healthy = 8.;
  float min_hiv = 5.;
  float delta_hiv = 45.;
  float alpha_hiv = 8.;*/
  
  // AM: Mortality rate depending on HIV state, i.e. Acute, Chronic, Treated, Failing (Cumulative probabilities)
  /*const std::vector<float> mortality_rate_hiv{
        0.0, 0.05, 0.06, 0.16};*/

  // Probability of getting infected when having sex with infected agent
  // float infection_probability = 2e-3;//AM replaced 0.01 by O(1e-3) if considered infection probability per sex
  
  // AM added: Probability of getting infected depends on 1) disease state, 2) sex of partners
  // Male-to-female
  float infection_probability_acute_mf = 9.3e-3;
  float infection_probability_chronic_mf = 1.9e-3;
  float infection_probability_treated_mf = 1.3e-4;
  float infection_probability_failing_mf = 7.6e-4;
  // Female-to-male
  float infection_probability_acute_fm = 4.8e-3;
  float infection_probability_chronic_fm = 9.5e-4;
  float infection_probability_treated_fm = 6.5e-4;
  float infection_probability_failing_fm = 3.9e-4;
  // Male-to-male
  float infection_probability_acute_mm = 9.3e-2;
  float infection_probability_chronic_mm = 1.9e-2;
  float infection_probability_treated_mm = 1.3e-3;
  float infection_probability_failing_mm = 7.6e-3;
    
  // AM test: Add Transition Matrix between HIV states. GemState->GemsState->Year and Population_category
  std::vector<std::vector<std::vector<float>>> hiv_transition_matrix = set_hiv_transition_matrix();

  std::vector<std::vector<std::vector<float>>> set_hiv_transition_matrix () {
      std::vector<std::vector<std::vector<float>>> hiv_transition_matrix;
      
      int nb_states = GemsState::kGemsLast;
      hiv_transition_matrix.clear();
      hiv_transition_matrix.resize(nb_states);
      
      int nb_years_categ = 7;
      
      for (int i=0; i<nb_states; i++){
        hiv_transition_matrix[i].resize(nb_years_categ);
        for (int j=0; j<nb_years_categ; j++){
          if (i == GemsState::kAcute){
              // For all years and population categories
              hiv_transition_matrix[i][j].resize(nb_states);
              hiv_transition_matrix[i][j]={
                  0.0,0.0,1.0,1.0,1.0}; // After one year ACUTE, go to CHRONIC
          } else if (i == GemsState::kChronic){
            if (j==0){ // Prior to 2003, for all (women 18-40, children and others)
              hiv_transition_matrix[i][j].resize(nb_states);
              hiv_transition_matrix[i][j]={
                  0.0,0.0,1.0,1.0,1.0}; // NO ART, then stay chronic
            } else if (j==1){ // Between to 2003 and 2010, for women 18-40
              hiv_transition_matrix[i][j].resize(nb_states);
              hiv_transition_matrix[i][j]={
                  0.0,0.0,0.9,1.0,1.0};
            } else if (j == 2) { // Between to 2003 and 2010, for children
                hiv_transition_matrix[i][j].resize(nb_states);
                hiv_transition_matrix[i][j]={
                    0.0,0.0,0.8,1.0,1.0};
            } else if (j == 3) { // Between to 2003 and 2010, for others
                hiv_transition_matrix[i][j].resize(nb_states);
                hiv_transition_matrix[i][j]={
                    0.0,0.0,0.9,1.0,1.0};
            }  else if (j == 4) { // From 2011, for women 18-40
                hiv_transition_matrix[i][j].resize(nb_states);
                hiv_transition_matrix[i][j]={
                    0.0,0.0,0.5,1.0,1.0};
            }  else if (j == 5) { // From 2011, for children
                hiv_transition_matrix[i][j].resize(nb_states);
                hiv_transition_matrix[i][j]={
                    0.0,0.0,0.5,1.0,1.0};
            } else if (j == 6) { // After 2011, for others
                hiv_transition_matrix[i][j].resize(nb_states);
                hiv_transition_matrix[i][j]={
                    0.0,0.0,0.8,1.0,1.0};
            }
          } else if (i == GemsState::kTreated){
            // For all years and population categories
              hiv_transition_matrix[i][j].resize(nb_states);
              hiv_transition_matrix[i][j]={
                  0.0,0.0,0.1,1.0,1.0};
          } else {
              hiv_transition_matrix[i][j].resize(nb_states);
              hiv_transition_matrix[i][j]={
                  0.0,0.0,0.0,0.0,0.0};
          }
        }
      }
      return hiv_transition_matrix;
  };
    
  // AM test: Add Location Mixing Matrix. Location->Location
  std::vector<std::vector<float>> location_mixing_matrix = set_location_mixing_matrix();

  std::vector<std::vector<float>> set_location_mixing_matrix () {
      
    std::vector<std::vector<float>> location_mixing_matrix;
        
    int nb_locations = Location::kLocLast;
    location_mixing_matrix.clear();
    location_mixing_matrix.resize(nb_locations);

    for (int i=0; i<nb_locations; i++){
        location_mixing_matrix[i].resize(nb_locations);
        // Fill all elements with 0.0 except diagonal with 1.0.
        /*fill(location_mixing_matrix[i].begin(),location_mixing_matrix[i].end(), 0.0);
        location_mixing_matrix[i][i]=1.0;*/
        // Fill all elements with 1.0 (Homogeneous mixing)
        fill(location_mixing_matrix[i].begin(),location_mixing_matrix[i].end(), 1.0);
    }
    
    // DEBUG
    /*std::cout << "nb_locations = " << nb_locations << std::endl;
    for (int i=0; i<nb_locations; i++){
      for (int j = 0; j<nb_locations; j++){
          std::cout << location_mixing_matrix[i][j] << ",";
      }
      std::cout<< std::endl;
    }*/ // END DEBUG
    return location_mixing_matrix;
  };

  // Five-years age categories 15-19, 20-24, ...,65-69,70+
  int nb_age_categories = 12; // AM TO DO : Implement function that takes the age and returns the age category

    
  // AM test: Add Age Mixing Matrix. Age Category -> Age Category
  std::vector<std::vector<float>> age_mixing_matrix = set_age_mixing_matrix();

  std::vector<std::vector<float>> set_age_mixing_matrix () {
        
      std::vector<std::vector<float>> age_mixing_matrix;
          
      age_mixing_matrix.clear();
      age_mixing_matrix.resize(nb_age_categories);

      for (int i=0; i<nb_age_categories; i++){
          age_mixing_matrix[i].resize(nb_age_categories);
          // Fill all elements with 1.0. Homogeneous age mixing.
          fill(age_mixing_matrix[i].begin(),age_mixing_matrix[i].end(), 1.0);
      }
      return age_mixing_matrix;
  };
    
  // AM: Socio-beahvoural Mixing matrix. Test with 2x2 in case of boolean feature. // AM TO DO: Generalize to Categorical Feature
  std::vector<std::vector<float>> sociobehav_mixing_matrix = set_sociobehav_mixing_matrix();

    std::vector<std::vector<float>> set_sociobehav_mixing_matrix () {
          
        std::vector<std::vector<float>> sociobehav_mixing_matrix;
        int nb_sociobehav_categories = 2; // AM TO DO: Change to N. ex. Number of elements in new datatype in datatype.h?
        
        sociobehav_mixing_matrix.clear();
        sociobehav_mixing_matrix.resize(nb_age_categories);

        for (int i=0; i<nb_sociobehav_categories; i++){
            sociobehav_mixing_matrix[i].resize(nb_sociobehav_categories);
            // Fill all elements with 1.0. Homogeneous socio-behavioural mixing.
            fill(sociobehav_mixing_matrix[i].begin(),sociobehav_mixing_matrix[i].end(), 1.0);
        }
        return sociobehav_mixing_matrix;
  };
        
  // Probability for agent to be infected at beginning of simulation. You can
  // expect roughly <initial_population_size * initial_infection_probability>
  // infected agents at the beginning of the simulation. These infected
  // individuals can occur anywhere on the age spectrum.
  // float initial_infection_probability = 0.01;
    
  // AM: Probability for agent to be healthy or at a certain HIV progression state at beginning of simulation. Given in a summed up form.
  // First vector x1 = p_Healthy component corresponds to probability of being Healthy.
  // x2 = p_Healthy + p_Acute, x3 = p_Healthy + p_Acute + p_Chronic, x4 = p_Healthy + p_Acute + p_Chronic + p_Treated,
  // x5 = p_Healthy + p_Acute + p_Chronic + p_Treated + p_Failing
  //const std::vector<float> initial_infection_probability{
  //    0.99, 0.99, 1, 1, 1, 1};
    
  //Reduce initial probability infection to 1e-5 instead of 1e-2
  /*const std::vector<float> initial_infection_probability{
        99999e-5, 99999e-5, 1, 1, 1, 1};*/
  const std::vector<float> initial_infection_probability{
          999e-3, 999e-3, 1, 1, 1, 1};

  // Parameter 0.18 is chosen because our GiveBirth Behaviour is based on a
  // Bernoulli experiment. A binomial distribuition peaks at around 6 for 25
  // tries (=40-15) and a birth probability of 0.24. This corresponds to the
  // typical birth rate in the region. We substracted 0.06 to account for child
  // motability and reach a realistic demographic development from 1960-2020.
  float give_birth_probability = 0.18;

  // Probability for agent to be infected at birth, if its mother is infected
  //float birth_infection_probability = 0.1;
  // AM : Probability for agent to be infected at birth, if its mother is infected and treated
  float birth_infection_probability_treated = 0.01;
  // AM : Probability for agent to be infected at birth, if its mother is infected and unterated (i.e. acute, chronic, failing)
  float birth_infection_probability_untreated = 0.2;

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
