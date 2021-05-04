#include "categorical-environment.h"
#include "core/util/log.h"

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

// AgentVector::AgentVector() : agents_(0), shuffled_(false) {
//   agents_.reserve(10000);
// }

// AgentVector::~AgentVector() {
//   for (auto& ptr : agents_) {
//     delete ptr;
//   }
//   agents_.resize(0);
// };

// AgentVector::AgentVector(const AgentVector& other)
//     : agents_(other.agents_.size()), shuffled_(other.shuffled_) {
//   for (auto ptr : other.agents_) {
//     agents_.push_back(ptr);
//   }
// };

// AgentVector& AgentVector::operator=(const AgentVector& other) {
//   agents_.resize(other.agents_.size());
//   shuffled_ = other.shuffled_;
//   for (auto ptr : other.agents_) {
//     agents_.push_back(ptr);
//   }
//   return *this;
// };

AgentPointer<Person> AgentVector::GetRandomAgent() {
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
  // try {
  //   agents_.push_back(agent);
  // } catch (const std::exception& e) {
  //   Log::Fatal("AgentVector::AddAgent()", "Exception:", e.what(),
  //              "| Agent: ", agent, "| agents_.size(): ", agents_.size());
  // }
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
