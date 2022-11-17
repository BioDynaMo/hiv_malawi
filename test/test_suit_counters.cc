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
#include "person.h"
#include "sim-param.h"

#define TEST_NAME typeid(*this).name()

namespace bdm {
namespace hiv_malawi {

// Empty environment such that we don't need to worry about any asserts in the
// custom categorical environment.
class EmptyEnvironment : public Environment {
 public:
  // Constructor
  EmptyEnvironment() = default;

  // The remaining public functions are inherited from Environment but not
  // needed here.
  void Clear() override { ; };
  void ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                       const Agent& query, double squared_radius) override{};

  // Not supported Neighbor search for this particular environment. Throws fatal
  // error.
  virtual void ForEachNeighbor(Functor<void, Agent*>& lambda,
                               const Agent& query, void* criteria) override{};

  // Not supported Neighbor search for this particular environment. Throws fatal
  // error.
  virtual void ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                               const Double3& query_position,
                               double squared_radius,
                               const Agent* query_agent = nullptr) override{};

  std::array<int32_t, 6> GetDimensions() const override {
    return std::array<int32_t, 6>();
  };

  std::array<int32_t, 2> GetDimensionThresholds() const override {
    return std::array<int32_t, 2>();
  };

  LoadBalanceInfo* GetLoadBalanceInfo() override { return nullptr; };

  Environment::NeighborMutexBuilder* GetNeighborMutexBuilder() override {
    return nullptr;
  };

 protected:
  void UpdateImplementation() override{};
};

// Test the counting of healty agents
TEST(CounterTest, Healthy) {
  Param::RegisterParamGroup(new SimParam());
  Simulation simulation(TEST_NAME);

  // Add two healty people to simulation
  auto p1 = new Person();
  auto p2 = new Person();
  p1->state_ = GemsState::kHealthy;
  p2->state_ = GemsState::kHealthy;
  auto* rm = simulation.GetResourceManager();
  rm->AddAgent(p1);
  rm->AddAgent(p2);

  // Set empty environment for test purposes
  auto* env = new EmptyEnvironment();
  simulation.SetEnvironment(env);

  // Initialize counters
  DefineAndRegisterCollectors();

  // Run simulation for one simulation
  auto* scheduler = simulation.GetScheduler();
  scheduler->UnscheduleOp(scheduler->GetOps("load balancing")[0]);
  scheduler->Simulate(1);

  // Check if we find the correct number of healthy people
  auto* ts = simulation.GetTimeSeries();
  auto& count1 = ts->GetYValues("healthy_agents");
  EXPECT_EQ(count1[0], 2);
  auto& count2 = ts->GetYValues("infected_agents");
  EXPECT_EQ(count2[0], 0);
}

// Test the counting of acute infected agents
TEST(CounterTest, Acute) {
  Param::RegisterParamGroup(new SimParam());
  Simulation simulation(TEST_NAME);

  // Add two healty people to simulation
  auto p1 = new Person();
  auto p2 = new Person();
  p1->state_ = GemsState::kAcute;
  p2->state_ = GemsState::kAcute;
  auto* rm = simulation.GetResourceManager();
  rm->AddAgent(p1);
  rm->AddAgent(p2);

  // Set empty environment for test purposes
  auto* env = new EmptyEnvironment();
  simulation.SetEnvironment(env);

  // Initialize counters
  DefineAndRegisterCollectors();

  // Run simulation for one simulation
  auto* scheduler = simulation.GetScheduler();
  scheduler->UnscheduleOp(scheduler->GetOps("load balancing")[0]);
  scheduler->Simulate(1);

  // Check if we find the correct number of healthy people
  auto* ts = simulation.GetTimeSeries();
  auto& count1 = ts->GetYValues("acute_agents");
  EXPECT_EQ(count1[0], 2);
  auto& count2 = ts->GetYValues("healthy_agents");
  EXPECT_EQ(count2[0], 0);
}

}  // namespace hiv_malawi
}  // namespace bdm
