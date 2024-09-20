#define STIMWALKER_API_EXPORTS
#include "Utils/Timestamp.h"

#include <chrono>

using namespace STIMWALKER_NAMESPACE::utils;

Timestamp::Timestamp() {}

uint64_t Timestamp::timeSinceEpoch() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
      .count();
}