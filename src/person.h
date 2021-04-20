
#ifndef PERSON_H_
#define PERSON_H_

#include "core/agent/cell.h"

namespace bdm {

/// Possible Person states, either susceptible, infected, or recoverd.
enum State { kSusceptible, kInfected, kRecovered };

// This class describes a single person. A person has a specific position in
// the three dimensional space and one of the three illness states, see above.
class Person : public Cell {
  BDM_AGENT_HEADER(Person, Cell, 1);

 public:
  Person() {}
  explicit Person(const Double3& position) : Base(position) {}
  virtual ~Person() {}

  /// This data member stores the current state of the person.
  int state_ = State::kSusceptible;
};

}  // namespace bdm

#endif  // PERSON_H_