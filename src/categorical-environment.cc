#include "categorical-environment.h"

namespace bdm {

////////////////////////////////////////////////////////////////////////////////
// AgentVector
////////////////////////////////////////////////////////////////////////////////

void AgentVector::shuffle_() {
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(agents_.begin(), agents_.end(), g);
  shuffled_ = true;
}

AgentPointer<Person> AgentVector::GetRandomAgent() {
  if (agents_.size() == 0) {
    Log::Fatal("AgentVector::GetRandomAgent()",
               "There are no females available for mating in one of your "
               "regions. Consider increasing the number of Agents.");
  }
  if (!shuffled_) {
    shuffle_();
  }
  if (iter == agents_.size()) {
    shuffle_();
    iter = 0;
  }
  return agents_[iter++];
}

void AgentVector::AddAgent(AgentPointer<Person> agent) {
  agents_.push_back(agent);
}

void AgentVector::Clear() {
  shuffled_ = false;
  agents_.clear();
  agents_.reserve(10000);
}

////////////////////////////////////////////////////////////////////////////////
// CategoricalEnvironment
////////////////////////////////////////////////////////////////////////////////

void CategoricalEnvironment::SetNumLocations(size_t num_locations) {
  female_agents_.resize(num_locations);
}

void CategoricalEnvironment::SetMinAge(int min_age) {
  if (min_age >= 0 && min_age <= 120) {
    min_age_ = min_age;
  } else {
    Log::Fatal("CategoricalEnvironment::SetMinAge()",
               "SetMinAge ignored, min_age must be in [0,120].");
  }
}

void CategoricalEnvironment::SetMaxAge(int max_age) {
  if (max_age >= 0 && max_age <= 120) {
    max_age_ = max_age;
  } else {
    Log::Fatal("CategoricalEnvironment::SetMaxAge()",
               "SetMinAge ignored, max_age must be in [0,120].");
  }
}

void CategoricalEnvironment::ForEachNeighbor(
    Functor<void, Agent*, double>& lambda, const Agent& query,
    double squared_radius) {
  ;
};

const std::array<int32_t, 6>& CategoricalEnvironment::GetDimensions() const {
  static std::array<int32_t, 6> arr;
  return arr;
};

const std::array<int32_t, 2>& CategoricalEnvironment::GetDimensionThresholds()
    const {
  static std::array<int32_t, 2> arr;
  return arr;
};

LoadBalanceInfo* CategoricalEnvironment::GetLoadBalanceInfo() {
  Log::Fatal("CategoricalEnvironment::GetLoadBalanceInfo()",
             "LoadBalancing not supported for this environment.");
  return nullptr;
};

Environment::NeighborMutexBuilder*
CategoricalEnvironment::GetNeighborMutexBuilder() {
  return nullptr;
};

// Function for Debug - prints number of females per location.
void CategoricalEnvironment::DescribePopulation() {
  size_t cntr{0};
  size_t sum{0};
  std::cout << "[ ";
  for (auto loc : female_agents_) {
    std::cout << loc.GetNumAgents() << " " << loc.IsShuffled() << " (" << cntr
              << ")  ";
    sum += loc.GetNumAgents();
    if (cntr % 5 == 0) {
      std::cout << "\n  ";
    }
    cntr++;
  }
  std::cout << " ]\nsum = " << sum << std::endl;
};

}  // namespace bdm
