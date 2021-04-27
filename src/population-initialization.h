#ifndef POPULATION_INITIALIZATION_H_
#define POPULATION_INITIALIZATION_H_

#include "biodynamo.h"
#include "datatypes.h"
#include "core/agent/agent_pointer.h"
#include "core/agent/cell.h"

#include <iostream>
namespace bdm {

////////////////////////////////////////////////////////////////////////////////
// Helper functions to initialize the popluation
////////////////////////////////////////////////////////////////////////////////

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


////////////////////////////////////////////////////////////////////////////////
// BioDynaMo's Agent / Individual
////////////////////////////////////////////////////////////////////////////////

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
// BioDynaMo's Agent / Individual Behaviours
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

struct RandomMigration : public Behavior {
  BDM_BEHAVIOR_HEADER(RandomMigration, Behavior, 1);

  RandomMigration() {}

  void Run(Agent* agent) override {
    auto* sim = Simulation::GetActive();
    auto* random = sim->GetRandom();
    auto* person = bdm_static_cast<Person*>(agent);

    int migration_direction = static_cast<int>(random->Gaus(0.0, 2.0));
    if (person->location_ + migration_direction < 0) {
      person->location_ += Location::kLocLast + migration_direction;
    } else if (person->location_ + migration_direction > Location::kLocLast) {
      person->location_ += migration_direction - Location::kLocLast;
    } else {
      person->location_ += migration_direction;
    }
  }
};


struct GetOlder : public Behavior {
  BDM_BEHAVIOR_HEADER(GetOlder, Behavior, 1);

  GetOlder() {}

  void Run(Agent* agent) override {
    auto* sim = Simulation::GetActive();
    auto* random = sim->GetRandom();
    auto* person = bdm_static_cast<Person*>(agent);

    // when turning 15, assign risk factors
    if (person->age_ < 15 && person->age_ + 1 > 15) {
      // update risk factors stochastically like in initialization
      if (random->Uniform() > 0.95){
        person->social_behaviour_factor_ = 1;
      }
      if (random->Uniform() > 0.95){
        person->biomedical_factor_ = 1;
      }
    }

    // possibly die - if not, just get older
    bool stay_alive {true};
    // Let's assume a linear increase of the death probability per year for 
    // healty agents starting from 45 to 120.
    if (person->state_ == GemsState::kHealthy){
      if (random->Uniform() < (person->age_ - 45.)/(75.*8.)){
        stay_alive = false;
      }
    }
    // Let's assume a linear increase of the death probability per year for 
    // non-healty agents starting from 5 to 50.
    if (person->state_ != GemsState::kHealthy){
      if (random->Uniform() < (person->age_ - 5.)/(45.*8.)){
        stay_alive = false;
      }
    }
    // hard cut at 120
    if (person->age_ >= 90.0 ){
      // std::cout << "max age kill planned for " << person->GetUid() << std::endl;
      stay_alive = false;
    }
    if (!stay_alive){
      // Person dies, i.e. is removed from simulation.
      // std::cout << "kill person " << person->GetUid() << " of age " << person->age_ << std::endl;
      person->RemoveFromSimulation();
    }
    else {
      // increase age
      person->age_ += 1;
    }
  }
};

struct GiveBirth : public Behavior {
  BDM_BEHAVIOR_HEADER(GiveBirth, Behavior, 1);

  GiveBirth() {}

  // create a single child
  Person* create_child(Random* random_generator, Person* mother){
    // Get all random numbers for initialization
    std::vector<double> rand_num{};
    rand_num.reserve(10);
    for (int i = 0; i < 10; i++) {
      rand_num[i] = random_generator->Uniform();
    }

    // Get a new person
    // Cells are simulated with a spacial uniform grid environment. Typically,
    // cells don't occur on the very same position and therefore the number of
    // cell per grid box is described with a uint16_t. Thus, if we don't asssign
    // random positions, we are bounded to a maximum number of 65535 agents.
    Person* child = new Person(
        {100.0 * rand_num[7], 100.0 * rand_num[8], 100.0 * rand_num[9]});

    // Assign sex
    child->sex_ = sample_sex(rand_num[0]);
    // Assign age - possibly -1 ?
    child->age_ = rand_num[1];
    // Assign location
    child->location_ = mother->location_;
    // Compute risk factors
    child->social_behaviour_factor_ = 0;
    child->biomedical_factor_ = 0;
    // Stores the current GemsState of the child.
    if (mother->state_ == GemsState::kHealthy){
      child->state_ = GemsState::kHealthy;
      // Store the year when the agent got infected
      child->year_of_infection_ = std::numeric_limits<float>::max();
    }
    // let's assume that if a mother is HIV positive, in 80 % of the cases the 
    // child will be hiv positive, too.
    else if (rand_num[2] < 0.8 ) {
      child->state_ = GemsState::kGems1;
      // year of infection to present year, Question: Ask Lukas how to get iter
      child -> year_of_infection_ = 2000;
    }
    // NOTE: we do not assign a specific mother or partner at the moment. Use 
    // nullptr instead.
    child->mother_id_ = AgentPointer<Person>();
    child->partner_id_ = AgentPointer<Person>();

    // Add the "grow and divide" behavior to each cell
    child->AddBehavior(new Infection());
    child->AddBehavior(new RandomMovement());
    child->AddBehavior(new GetOlder());
    if (child->sex_ == Sex::kFemale){
      child->AddBehavior(new GiveBirth());
    }
    return child;
  }


  void Run(Agent* agent) override {
    auto* sim = Simulation::GetActive();
    auto* ctxt = sim->GetExecutionContext();
    auto* random = sim->GetRandom();
    auto* mother = bdm_static_cast<Person*>(agent);
    // Parameter 0.24 is chosen because our GiveBirth Behaviour is based on a 
    // Bernoulli experiment. A binomial distribuition peaks at around 6 for 25 
    // tries and a birth probability of 0.24.
    if (random->Uniform(0.0, 1.0) < 0.24 && mother->age_ <= 40 &&
        mother->age_ >= 15) {
      auto* new_child = create_child(random, mother);
      ctxt->AddAgent(new_child);
    }
  }
};


}  // namespace bdm

#endif  // POPULATION_INITIALIZATION_H_
