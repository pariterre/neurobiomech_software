#ifndef __NEUROBIO_ANALYZER_CYCLIC_TIMED_EVENTS_ANALYZER_H__
#define __NEUROBIO_ANALYZER_CYCLIC_TIMED_EVENTS_ANALYZER_H__

#include "Analyzer/TimedEventsAnalyzer.h"

namespace NEUROBIO_NAMESPACE::analyzer {
class EventConditions;

class CyclicTimedEventsAnalyzer : public TimedEventsAnalyzer {

public:
  /// @brief Constructor of the CyclicTimedEventsAnalyzer. This
  /// constructor assumes a step is one second long and the phases are stance
  /// phase for 400ms and swing phase for 600ms
  /// @param name The name of the analyzer
  /// @param changePhaseConditions The conditions to change the phase
  /// @param timeDeviceReference The name of the device to get the reference
  /// time
  /// @param learningRate The learning rate of the analyzer
  CyclicTimedEventsAnalyzer(const std::string &name,
                            const std::vector<std::unique_ptr<EventConditions>>
                                &changePhaseConditions,
                            const std::string &timeDeviceReference,
                            double learningRate = 0.2);

  /// @brief Constructor of the CyclicTimedEventsAnalyzer from a json object
  /// @param json The json object to create the analyzer from
  CyclicTimedEventsAnalyzer(const nlohmann::json &json);

  /// @brief Destructor of the CyclicTimedEventsAnalyzer
  ~CyclicTimedEventsAnalyzer() override = default;

protected:
  /// @brief The conditions to change the phase
  DECLARE_PROTECTED_MEMBER(std::vector<std::unique_ptr<EventConditions>>,
                           EventConditions);

  /// @brief The name of the device to get the reference time from
  DECLARE_PROTECTED_MEMBER(std::string, TimeDeviceReferenceName);

  /// @brief The callback to determine if the phase should be incremented
  /// @param data The data to analyze
  bool
  shouldIncrementPhase(const std::map<std::string, data::TimeSeries> &data);

  /// @brief The callback to get the current time
  /// @param data The data to analyze
  std::chrono::system_clock::time_point
  getCurrentTime(const std::map<std::string, data::TimeSeries> &data);
};

} // namespace NEUROBIO_NAMESPACE::analyzer

#endif // __NEUROBIO_ANALYZER_CYCLIC_TIMED_EVENTS_ANALYZER_H__