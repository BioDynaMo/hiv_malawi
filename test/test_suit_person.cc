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
#include "person.h"

#define TEST_NAME typeid(*this).name()

namespace bdm {
namespace hiv_malawi {

TEST(PersonTest, Age) {
  Simulation simulation(TEST_NAME);
  auto person = Person();

  // Age
  person.age_ = 1;
  EXPECT_DOUBLE_EQ(person.age_, 1);
  EXPECT_FALSE(person.IsAdult());
  person.age_ = 16;
  EXPECT_TRUE(person.IsAdult());
}

TEST(PersonTest, Sex) {
  Simulation simulation(TEST_NAME);
  auto person = Person();

  // SEX
  person.sex_ = Sex::kMale;
  EXPECT_DOUBLE_EQ(person.sex_, Sex::kMale);
  EXPECT_TRUE(person.IsMale());
  EXPECT_FALSE(person.IsFemale());
  person.sex_ = Sex::kFemale;
  EXPECT_FALSE(person.IsMale());
  EXPECT_TRUE(person.IsFemale());
}

TEST(PersonTest, Partner) {
  Simulation simulation(TEST_NAME);
  auto person = new Person();
  auto partner = new Person();
  auto* rm = simulation.GetResourceManager();
  rm->AddAgent(person);
  rm->AddAgent(partner);
  auto ap_partner = partner->GetAgentPtr<Person>();

  EXPECT_FALSE(person->hasPartner());
  EXPECT_FALSE(partner->hasPartner());

  // Set Partner
  person->SetPartner(ap_partner);
  EXPECT_TRUE(person->hasPartner());
  EXPECT_TRUE(partner->hasPartner());

  // Seperate Partner
  person->SeparateFromPartner();
  EXPECT_FALSE(person->hasPartner());
  EXPECT_FALSE(partner->hasPartner());
}

TEST(PersonTest, MotherChild) {
  Simulation simulation(TEST_NAME);
  auto child = new Person();
  auto mother = new Person();
  auto* rm = simulation.GetResourceManager();
  rm->AddAgent(child);
  rm->AddAgent(mother);
  auto ap_child = child->GetAgentPtr<Person>();
  auto ap_mother = mother->GetAgentPtr<Person>();

  EXPECT_EQ(mother->GetNumberOfChildren(), 0);
  EXPECT_FALSE(mother->IsParentOf(ap_child));
  EXPECT_FALSE(child->IsChildOf(ap_mother));

  // Add child (does not add mother for child)
  mother->AddChild(ap_child);
  EXPECT_EQ(mother->GetNumberOfChildren(), 1);
  EXPECT_TRUE(mother->IsParentOf(ap_child));
  EXPECT_FALSE(child->IsChildOf(ap_mother));

  // Add mother
  child->mother_ = ap_mother;
  EXPECT_EQ(mother->GetNumberOfChildren(), 1);
  EXPECT_TRUE(mother->IsParentOf(ap_child));
  EXPECT_TRUE(child->IsChildOf(ap_mother));

  // Remove child (does not remove mother for child)
  mother->RemoveChild(ap_child);
  EXPECT_EQ(mother->GetNumberOfChildren(), 0);
  EXPECT_FALSE(mother->IsParentOf(ap_child));
  EXPECT_TRUE(child->IsChildOf(ap_mother));

  // Remove mother
  child->mother_ = nullptr;
  EXPECT_EQ(mother->GetNumberOfChildren(), 0);
  EXPECT_FALSE(mother->IsParentOf(ap_child));
  EXPECT_FALSE(child->IsChildOf(ap_mother));
}

TEST(PersonTest, State) {
  Simulation simulation(TEST_NAME);
  auto person = Person();

  // Healty
  person.state_ = GemsState::kHealthy;
  EXPECT_TRUE(person.IsHealthy());
  EXPECT_FALSE(person.IsAcute());
  EXPECT_FALSE(person.IsChronic());
  EXPECT_FALSE(person.IsTreated());
  EXPECT_FALSE(person.IsFailing());

  // Acute
  person.state_ = GemsState::kAcute;
  EXPECT_FALSE(person.IsHealthy());
  EXPECT_TRUE(person.IsAcute());
  EXPECT_FALSE(person.IsChronic());
  EXPECT_FALSE(person.IsTreated());
  EXPECT_FALSE(person.IsFailing());

  // Chronic
  person.state_ = GemsState::kChronic;
  EXPECT_FALSE(person.IsHealthy());
  EXPECT_FALSE(person.IsAcute());
  EXPECT_TRUE(person.IsChronic());
  EXPECT_FALSE(person.IsTreated());
  EXPECT_FALSE(person.IsFailing());

  // Treated
  person.state_ = GemsState::kTreated;
  EXPECT_FALSE(person.IsHealthy());
  EXPECT_FALSE(person.IsAcute());
  EXPECT_FALSE(person.IsChronic());
  EXPECT_TRUE(person.IsTreated());
  EXPECT_FALSE(person.IsFailing());

  // Failing
  person.state_ = GemsState::kFailing;
  EXPECT_FALSE(person.IsHealthy());
  EXPECT_FALSE(person.IsAcute());
  EXPECT_FALSE(person.IsChronic());
  EXPECT_FALSE(person.IsTreated());
  EXPECT_TRUE(person.IsFailing());
}

}  // namespace hiv_malawi
}  // namespace bdm
