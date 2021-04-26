#ifndef POPULATION_INITIALIZATION_H_
#define POPULATION_INITIALIZATION_H_

#include "biodynamo.h"
#include "core/agent/agent_pointer.h"
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

// create a single person
auto create_person(Random* random_generator);

// Initialize an entire population for the BDM simulation
void initialize_population(Random* random_generator, int population_size);

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
  int sex_;
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
  AgentPointer<Person> mother_id_;
  // Stores the id of the partner
  AgentPointer<Person> partner_id_;
};

////////////////////////////////////////////////////////////////////////////////
// Behaviour of persons defined below
////////////////////////////////////////////////////////////////////////////////
struct CheckSurrounding : public Functor<void, Agent*, double> {
  Person* self_;

  CheckSurrounding(Person* self) : self_(self) {}

  // This function operator will be called for every other person within
  // `infection_radius`
  void operator()(Agent* neighbor, double squared_distance) override {
    auto* other = bdm_static_cast<const Person*>(neighbor);
    auto* sim = Simulation::GetActive();
    auto* random = sim->GetRandom();
    if (other->state_ == GemsState::kHealthy && random->Uniform() < 0.05) {
      self_->state_ =
          static_cast<int>(random->Uniform(1.0, GemsState::kGemsLast));
    }
  }
};

struct Infection : public Behavior {
  BDM_BEHAVIOR_HEADER(Infection, Behavior, 1);

  Infection() {}

  void Run(Agent* a) override {
    auto* sim = Simulation::GetActive();
    auto* random = sim->GetRandom();
    // auto* param = sim->GetParam();
    // auto* sparam = param->Get<SimParam>();

    auto* person = bdm_static_cast<Person*>(a);
    if (person->state_ == GemsState::kHealthy &&
        // random->Uniform(0, 1) <= sparam->infection_probablity) {
        random->Uniform(0, 1) <= 0.10) {
      auto* ctxt = sim->GetExecutionContext();
      CheckSurrounding check(person);
      // ForEachNeighbor executes "check" for all neighbors in
      // sqrt(infection_radius)
      ctxt->ForEachNeighbor(check, *person, 2.0);
    }
  }
};

struct RandomMovement : public Behavior {
  BDM_BEHAVIOR_HEADER(RandomMovement, Behavior, 1);

  RandomMovement() {}

  void Run(Agent* agent) override {
    auto* sim = Simulation::GetActive();
    auto* random = sim->GetRandom();
    auto* param = sim->GetParam();
    // auto* sparam = param->Get<SimParam>();

    const auto& position = agent->GetPosition();
    auto rand_movement = random->UniformArray<3>(-1, 1).Normalize();
    auto new_pos = position + rand_movement;  // * sparam->agent_speed;
    // Implements periodic boundary conditions for position
    for (auto& el : new_pos) {
      // Compute floating-point remainder of division "el/param->max_bound"
      el = std::fmod(el, param->max_bound);
      // Put "el" into valid boudaries.
      el = el < 0 ? param->max_bound + el : el;
    }
    agent->SetPosition(new_pos);
  }
};

}  // namespace bdm

#endif  // POPULATION_INITIALIZATION_H_
