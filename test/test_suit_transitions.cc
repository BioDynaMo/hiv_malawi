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

#include <gtest/gtest.h>
#include "analyze.h"
#include "biodynamo.h"
#include "categorical-environment.h"
#include "person-behavior.h"
#include "person.h"
#include "sim-param.h"

#define TEST_NAME typeid(*this).name()

namespace bdm {

namespace hiv_malawi {

// Test if an infected, acute female agent infects a healthy male agent.
TEST(TransitionTest, FemaleToMale) {
  // Register Sim Param
  Param::RegisterParamGroup(new SimParam());

  // Set the probability male to female to 1.0
  auto set_param = [&](Param* param) {
    auto* sparam = param->Get<SimParam>();
    sparam->infection_probability_acute_fm = 1.0;
  };

  // Create simulation object
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();

  // Add a healthy male to the simulation
  auto male = new Person();
  male->state_ = GemsState::kHealthy;
  male->sex_ = Sex::kMale;
  male->age_ = 20;
  male->location_ = 0;
  male->biomedical_factor_ = 0;
  male->social_behaviour_factor_ = 0;
  male->AddBehavior(new MatingBehaviour());    // Add mating behavior
  auto ap_male = male->GetAgentPtr<Person>();  // Get agent pointer
  rm->AddAgent(male);

  // Add an infected (acute) female to the simulation
  auto female = new Person();
  female->state_ = GemsState::kAcute;
  female->sex_ = Sex::kFemale;
  female->age_ = 20;
  female->location_ = 0;
  female->biomedical_factor_ = 0;
  female->social_behaviour_factor_ = 0;
  // auto ap_female = female->GetAgentPtr<Person>();
  rm->AddAgent(female);

  // Set the custom environment
  auto* env = new CategoricalEnvironment(15, 40, 1, 1, 1);
  simulation.SetEnvironment(env);

  // Run simulation for one simulation time step
  auto* scheduler = simulation.GetScheduler();
  scheduler->UnscheduleOp(scheduler->GetOps("load balancing")[0]);
  scheduler->Simulate(1);

  // Check if the male agent is not infected and in the state accute
  EXPECT_TRUE(ap_male->state_ == GemsState::kAcute);
  // Check if the male agent received the infectino via a casual transmission
  EXPECT_TRUE(ap_male->CasualTransmission());
}

TEST(TransitionTest, MaleToFemale) {
  // Register Sim Param
  Param::RegisterParamGroup(new SimParam());

  // Set the probability male to female to 1.0
  auto set_param = [&](Param* param) {
    auto* sparam = param->Get<SimParam>();
    sparam->infection_probability_acute_mf = 1.0;
  };

  // Create simulation object
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();

  // Add a healthy female to the simulation
  auto female = new Person();
  female->state_ = GemsState::kHealthy;
  female->sex_ = Sex::kFemale;
  female->age_ = 20;
  female->location_ = 0;
  female->biomedical_factor_ = 0;
  female->social_behaviour_factor_ = 0;
  
  auto ap_female = female->GetAgentPtr<Person>();  // Get agent pointer
  rm->AddAgent(female);

  // Add an infected (acute) male to the simulation
  auto male = new Person();
  male->state_ = GemsState::kAcute;
  male->sex_ = Sex::kMale;
  male->age_ = 20;
  male->location_ = 0;
  male->biomedical_factor_ = 0;
  male->social_behaviour_factor_ = 0;
  male->AddBehavior(new MatingBehaviour());   // JE: added mating behavior
  // auto ap_male = male->GetAgentPtr<Person>();
  rm->AddAgent(male);

  // Set the custom environment
  auto* env = new CategoricalEnvironment(15, 40, 1, 1, 1);
  simulation.SetEnvironment(env);

  // Run simulation for one simulation time step
  auto* scheduler = simulation.GetScheduler();
  scheduler->UnscheduleOp(scheduler->GetOps("load balancing")[0]);
  scheduler->Simulate(1);

  // Check if the female agent is not infected and in the state accute
  EXPECT_TRUE(ap_female->state_ == GemsState::kAcute);
  // Check if the female agent received the infectino via a casual transmission
  EXPECT_TRUE(ap_female->CasualTransmission());
}

}  // namespace hiv_malawi
}  // namespace bdm
