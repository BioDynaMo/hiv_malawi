#include "core/environment/environment.h"
#include "core/resource_manager.h"

#include "datatypes.h"
#include "population-initialization.h"

#include <random>

namespace bdm {

// DataType to store all agents at a certain location. Usa as
// AgentsAtLocation agents; agents[location][some_number]
// For our purposes, our
class AgentVector {
 private:
  // vector of AgentPointers
  std::vector<Agent*> agents_;
  // Has the vector been shuffled or not
  bool shuffled_;
  // Iter to go through vector and obtain random Agents
  size_t iter;

  // Shuffle the vector agents_
  void shuffle_();

 public:
  // default constructor
  AgentVector();
  // destructor
  ~AgentVector();
  // copy constructor
  AgentVector(const AgentVector& other);
  // copy assignment
  AgentVector& operator=(const AgentVector& other);
  // NO move operations
  AgentVector(AgentVector&&) = default;
  AgentVector& operator=(AgentVector&&) = default;

  // Get the number of agents in the vector
  size_t GetNumAgents() { return agents_.size(); }

  // Get a radom agent from the vector agents_
  Agent* GetRandomAgent();

  // Add an AgentPointer to the vector agents_
  void AddAgent(Agent* agent);

  // Delete vector entries and resize vector to 0
  void Clear();
};

class CategoricalEnvironment : public Environment {
 public:
  // Default constructor
  CategoricalEnvironment(int min_age = 15, int max_age = 40, size_t n_loc = 28)
      : min_age_(min_age), max_age_(max_age), female_agents_(n_loc) {}

  void Update() override {
    // for (auto FemalesAtLoc : female_agents_) {
    //   FemalesAtLoc.Clear();
    // }
    // auto* rm = Simulation::GetActive()->GetResourceManager();
    // rm->ForEachAgent([](Agent* agent) {
    //   auto* person = bdm_static_cast<Person*>(agent);

    //   // if (person->sex_ == Sex::kFemale){
    //   //   female_agents_[person->location_].AddAgent(person->GetAgentPtr());
    //   // }
    //   // else {
    //   //   ;
    //   // }
    // });
  };

  void AddAgentToLocation(size_t loc, Agent* agent) {
    female_agents_[loc].AddAgent(agent);
  }

  Agent* GetRamdomAgentAtLocation(size_t loc) {
    return female_agents_[loc].GetRandomAgent();
  }

  void Clear() override { ; };

  // Setter functions to access private member variables
  void SetNumLocations(size_t num_locations);
  void SetMinAge(int min_age);
  void SetMaxAge(int max_age);

  // Getter functions to access private member variables
  size_t GetNumLocations() { return female_agents_.size(); };
  int GetMinAge() { return min_age_; };
  int GetMaxAge() { return max_age_; };

  // The remaining public functinos are inherited from Environment but not
  // needed here.
  void ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                       const Agent& query, double squared_radius) override;

  const std::array<int32_t, 6>& GetDimensions() const override ;

  const std::array<int32_t, 2>& GetDimensionThresholds() const override ;

  LoadBalanceInfo* GetLoadBalanceInfo() override;

  Environment::NeighborMutexBuilder* GetNeighborMutexBuilder() override ;

 private:
  // minimal age for sexual interaction
  int min_age_;
  // maximal age for sexual interaction
  int max_age_;
  // Vector to store all female agents of within a certain age interval
  // [min_age_, max_age_].
  std::vector<AgentVector> female_agents_;
};

}  // namespace bdm
