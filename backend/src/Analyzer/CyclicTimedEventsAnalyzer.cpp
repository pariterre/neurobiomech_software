#include "Analyzer/CyclicTimedEventsAnalyzer.h"

#include "Analyzer/EventConditions.h"
#include "Data/TimeSeries.h"

using namespace NEUROBIO_NAMESPACE::analyzer;

std::vector<std::chrono::milliseconds>
intToMilliseconds(const std::vector<int> &initialPhaseTimes) {
  std::vector<std::chrono::milliseconds> result;
  for (size_t i = 0; i < initialPhaseTimes.size(); i++) {
    result.push_back(std::chrono::milliseconds(initialPhaseTimes[i]));
  }
  return result;
}

std::vector<std::unique_ptr<EventConditions>>
parseEventConditions(const nlohmann::json &json) {
  std::vector<std::unique_ptr<EventConditions>> conditions;
  for (const auto &condition : json) {
    conditions.push_back(std::make_unique<EventConditions>(condition));
  }
  return conditions;
}

CyclicTimedEventsAnalyzer::CyclicTimedEventsAnalyzer(const nlohmann::json &json)
    : m_EventConditions(parseEventConditions(json["events"])),
      m_TimeDeviceReferenceName(json["time_reference_device"]),
      TimedEventsAnalyzer(
          json["name"], intToMilliseconds(json["initial_phase_durations"]),
          [this](const std::map<std::string, data::TimeSeries> &data) {
            return shouldIncrementPhase(data);
          },
          [this](const std::map<std::string, data::TimeSeries> &data) {
            return getCurrentTime(data);
          },
          json["learning_rate"]) {
  EventConditions::collapseNameToIndices(m_EventConditions);
}

bool CyclicTimedEventsAnalyzer::shouldIncrementPhase(
    const std::map<std::string, data::TimeSeries> &data) {
  for (const auto &event : m_EventConditions) {
    if (event->isActive(m_CurrentPhaseIndex, data)) {
      return true;
    }
  }

  // If we get here, none of the conditions were met
  return false;
}

std::chrono::system_clock::time_point CyclicTimedEventsAnalyzer::getCurrentTime(
    const std::map<std::string, data::TimeSeries> &data) {
  // Get the data from the device
  const auto &dataDevice = data.at(m_TimeDeviceReferenceName);

  // Get the current time stamp by adding passed time to starting time
  return dataDevice.getStartingTime() + dataDevice.back().getTimeStamp();
}
