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

#ifndef PERSON_H_
#define PERSON_H_

#include "biodynamo.h"
#include "core/simulation.h"
#include "datatypes.h"

namespace bdm {
////////////////////////////////////////////////////////////////////////////////
// BioDynaMo's Agent / Individual
////////////////////////////////////////////////////////////////////////////////

class Person : public Cell {
  // BioDynaMo API
  BDM_AGENT_HEADER(Person, Cell, 1);

 public:
  Person() {
    mother_ = AgentPointer<Person>(); //AgentPointer object representing a nullptr
    partner_ = AgentPointer<Person>(); //AgentPointer object representing a nullptr
    children_.clear();
    children_.reserve(100);
    protected_ = false;
  }
  explicit Person(const Double3& position) : Base(position) {}
  virtual ~Person() {}

  /// Stores the current GemsState of the person.
  int state_;
  // Stores the age of the agent
  float age_;
  // Stores the sex of the agent
  int sex_;
  // Stores the location as categorical variable
  int location_;
  // Stores a factor representing the socio-behavioural risk
  int social_behaviour_factor_;
  // Stores a factor representing the biomedical risk
  int biomedical_factor_;
  // Protect a person against death. Currently only used for mothers in the year
  // in which they give birth and if sparam->protect_mothers_at_birth is true.
  // The associated member functions LockProtection, UnlockProtection, and
  // IsProtected appear in the GiveBirth and GetOlder Behaviors.
  bool protected_;

  ///! The aguments below are currently either not used or repetitive.
  // // Stores if an agent is infected or not
  // bool infected_;
  // // Store the year when the agent got infected
  // float year_of_infection_;
  // Stores the ID of the mother. Useful to unlink child from mother, when child
  // dies.
  AgentPointer<Person> mother_;
  // Stores the IDs of the children. Useful, when mother migrates, and takes her
  // children. Unlink mother from child, when mother dies
  std::vector<AgentPointer<Person>> children_;
  // Stores the ID of the regular partner. Useful for infection in 
  // serodiscordant regular relationships, and family migration. 
  AgentPointer<Person> partner_;

  // Returns True if the agent is healthy
  bool IsHealthy() { return state_ == GemsState::kHealthy; }
  // AM: Added below functions for more detailed follow up of HIV state
  // Returns True if the agent is infected in acute state
  bool IsAcute() { return state_ == GemsState::kAcute; }
  // Returns True if the agent is infected in chronic state
  bool IsChronic() { return state_ == GemsState::kChronic; }
  // Returns True if the agent is infected and treated state
  bool IsTreated() { return state_ == GemsState::kTreated; }
  // Returns True if the agent is infected in failing treatement state
  bool IsFailing() { return state_ == GemsState::kFailing; }
  // Returns True if the agent has high-risk socio-behaviours
  bool HasHighRiskSocioBehav() { return social_behaviour_factor_ == 1; }
  // Returns True if the agent is at low-risk socio-behaviours
  bool HasLowRiskSocioBehav() { return social_behaviour_factor_ == 0; }
  // Returns True if the agent is adult, is at least 15 years old
  bool IsAdult() { return age_ >= 15; }
  // Returns True if the agent is a male
  bool IsMale() { return sex_ == Sex::kMale; }
  // Returns True if the agent is a female
  bool IsFemale() { return sex_ == Sex::kFemale; }

  // AM - Get Age Category from 0 to no_age_categories. 5-years interval
  // categories from min_age.
  int GetAgeCategory(size_t min_age, size_t no_age_categories) {
    int age_category;
    if (age_ >= min_age + (no_age_categories - 1) * 5) {
      age_category = no_age_categories - 1;
    } else {
      age_category = (int)(age_ - min_age) / 5;
    }
    // DEBUG:
    // std::cout << "age " << age_ << " --> age_category " << age_category << "
    // (min_age " << min_age << ",  no_age_categories " << no_age_categories <<
    // ")" << std::endl;
    return age_category;
  }

  void AddChild(AgentPointer<Person> child) {
    /*if (child->location_ != location_){
        Log::Warning("Person::AddChild()", "Adding a child who is at a different
    location");
    }*/
    children_.push_back(child);
  }

  void RemoveChild(AgentPointer<Person> child) {
    bool found = false;
    for (int c = 0; c < GetNumberOfChildren(); c++) {
      if (children_[c] == child) {
        found = true;
        // std::cout << "Before, nb_children = " << GetNumberOfChildren() <<
        // std::endl;
        children_.erase(children_.begin() + c);
        // std::cout << " => Found and removed child from mother's list of
        // children" << std::endl; std::cout << "Afrer, nb_children = " <<
        // GetNumberOfChildren() << std::endl;
        break;
      }
    }
    if (!found) {
      Log::Warning("Person::RemoveChild()",
                   "Child to be removed not found in mother's list of "
                   "children. Age = ",
                   child->age_, " Mother:", this->GetAgentPtr(),
                   " Age mother:", this->age_,
                   " Num children:", children_.size());
    }
  }

  bool IsParentOf(AgentPointer<Person> child) {
    bool found = false;
    for (int c = 0; c < GetNumberOfChildren(); c++) {
      if (children_[c] == child) {
        found = true;
        break;
      }
    }
    return found;
  }

  bool IsChildOf(AgentPointer<Person> mother) { return mother_ == mother; }

  bool hasPartner() {return partner_ != nullptr;}
  bool IsPartnerOf(AgentPointer<Person> partner) { return partner_ == partner; }

  int GetNumberOfChildren() { return children_.size(); }

  // Activates the protection of an agent against death.
  void LockProtection() { protected_ = true; }
  // Deactivates the protection of an agent against death.
  void UnockProtection() { protected_ = false; }
  // Returns if an agent is protected against death.
  bool IsProtected() { return protected_; }
};

}  // namespace bdm

#endif  // PERSON_H_
