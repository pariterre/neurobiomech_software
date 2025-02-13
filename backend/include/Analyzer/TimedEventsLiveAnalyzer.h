#ifndef __NEUROBIO_ANALYZER_TIMED_EVENT_ANALYZER_H__
#define __NEUROBIO_ANALYZER_TIMED_EVENT_ANALYZER_H__

#include "Analyzer/EventPrediction.h"
#include "Analyzer/LiveAnalyzer.h"
#include "Utils/CppMacros.h"
#include <chrono>
#include <functional>
#include <vector>
namespace NEUROBIO_NAMESPACE::analyzer {

class TimedEventsLiveAnalyzer : public LiveAnalyzer {

public:
  /// @brief Constructor of the TimedEventsLiveAnalyzer
  /// @param initialPhaseTimes The initial times (ms) for each event
  /// @param shouldIncrementPhase If the current phase should be incremented
  /// @param getCurrentTime The current time based on the devices
  /// @param learningRate The learning rate of the analyzer
  TimedEventsLiveAnalyzer(
      const std::vector<std::chrono::milliseconds> &initialPhaseTimes,
      const std::function<bool(const std::map<size_t, const data::TimeSeries &>
                                   &)> &shouldIncrementPhase,
      const std::function<std::chrono::system_clock::time_point(
          const std::map<size_t, const data::TimeSeries &> &)> &getCurrentTime,
      double learningRate = 0.2);

public:
  /// @brief Destructor of the TimedEventsLiveAnalyzer
  ~TimedEventsLiveAnalyzer() override;

public:
  std::unique_ptr<Prediction>
  predict(const std::map<size_t, const data::TimeSeries &> &data) override;

protected:
  /// @brief The callback to determine if the phase should be incremented
  DECLARE_PROTECTED_MEMBER(
      std::function<bool(const std::map<size_t, const data::TimeSeries &> &)>,
      ShouldIncrementPhase);

  /// @brief The callback to get the current time
  DECLARE_PROTECTED_MEMBER(
      std::function<std::chrono::system_clock::time_point(
          const std::map<size_t, const data::TimeSeries &> &)>,
      GetCurrentTime);

  /// @brief Increment the model to the next phase
  void incrementModel();

  /// @brief The current phase index
  DECLARE_PROTECTED_MEMBER(size_t, CurrentPhaseIndex);

  /// @brief The time event model (one value representing the expected time
  /// events in a step)
  DECLARE_PROTECTED_MEMBER(std::vector<std::chrono::milliseconds>,
                           TimeEventModel);

  /// @brief The time event model (one value representing the expected time
  /// events in a step)
  DECLARE_PROTECTED_MEMBER(std::vector<std::chrono::milliseconds>,
                           NextTimeEventModel);

  /// @brief The learning rate of the analyzer
  DECLARE_PROTECTED_MEMBER(double, LearningRate);

  /// @brief Is the first pass or not
  DECLARE_PROTECTED_MEMBER(bool, FirstPass);

  /// @brief The time stamp of the last analyzed data
  DECLARE_PROTECTED_MEMBER(std::chrono::system_clock::time_point,
                           LastAnalyzedTimeStamp);

  /// @brief The time stamp of the current phase
  DECLARE_PROTECTED_MEMBER(std::chrono::milliseconds, CurrentPhaseTime);
};

} // namespace NEUROBIO_NAMESPACE::analyzer

#endif // __NEUROBIO_ANALYZER_TIMED_EVENT_ANALYZER_H__