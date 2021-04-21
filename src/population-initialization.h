#ifndef POPULATION_INITIALIZATION_H_
#define POPULATION_INITIALIZATION_H_

#include "core/agent/cell.h"

namespace bdm {

// Gives stochastic age based on hard coded age-distribution
float sample_age(float rand_num_1, float rand_num_2, int sex);

// Gives stochastic location based on hard coded location-distribution
int sample_location(float rand_num);

// Gives stochastic sex based on probability
int sample_sex(float rand_num);

// Compute sociobehavioural-factor; return 0 in 95% of the cases
int compute_sociobehavioural(float rand_num, int age);

// Compute biomedical-factor; return 0 in 95% of the cases
int compute_biomedical(float rand_num, int age);

// Initialize an entire population for the BDM simulation
void initialize_population(int population_size);

// This class describes a single person. A person has a specific position in
// the three dimensional space and one of the three illness states, see above.
class Person : public Cell {
  BDM_AGENT_HEADER(Person, Cell, 1);

 public:
  Person() {}
  explicit Person(const Double3& position) : Base(position) {}
  virtual ~Person() {}

  /// Stores the current GemsState of the person.
  int state_;
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

#endif  // POPULATION_INITIALIZATION_H_
