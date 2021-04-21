
#ifndef PERSON_H_
#define PERSON_H_

#include "core/agent/cell.h"

namespace bdm {

// This class describes a single person. A person has a specific position in
// the three dimensional space and one of the three illness states, see above.
class Person : public Cell {
  BDM_AGENT_HEADER(Person, Cell, 1);

 public:
  Person() {}
  explicit Person(const Double3& position) : Base(position) {}
  virtual ~Person() {}

  /// Stores the current GemsState of the person.
  int state_ = GemsState::kGems1;
  // Stores the id of the agent
  // int id_; NOT NEEDED
  // Stores the age of the agent
  float age_;
  // Stores the sex of the agent
  float sex_;
  // Stores the location as categorical variable
  int location_;
  // Stores a factor representing the socio-behavioural risk
  int social_behaviour_factor_;
  // Stores a factor representing the biomedical risk
  int biomedical_factor_;
  // Stores if an agent is infected or not
  bool infected_;
  // Store the year when the agent got infected
  float year_of_infection_;
  // Stores the ID of the mother
  int mother_id;
  // Stores the id of the partner
  int partner_id_;
};

}  // namespace bdm

#endif  // PERSON_H_