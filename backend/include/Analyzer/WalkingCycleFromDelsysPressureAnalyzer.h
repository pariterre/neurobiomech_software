#ifndef __NEUROBIO_ANALYZER_WALKING_CYCLE_FROM_DELSYS_PRESSURE_ANALYZER_H__
#define __NEUROBIO_ANALYZER_WALKING_CYCLE_FROM_DELSYS_PRESSURE_ANALYZER_H__

#include "Analyzer/TimedEventsLiveAnalyzer.h"

namespace NEUROBIO_NAMESPACE::analyzer {

class WalkingCycleFromDelsysPressureAnalyzer : public TimedEventsLiveAnalyzer {

public:
  /// @brief Constructor of the WalkingCycleFromDelsysPressureAnalyzer. This
  /// constructor assumes a step is one second long and the phases are stance
  /// phase for 400ms and swing phase for 600ms
  /// @param delsysDeviceIndex The Delsys device index to gather the data from
  /// @param channelIndex The channel index to gather the data from
  /// @param heelStrikeThreshold The minimum threshold to detect the heel strike
  /// @param toeOffThreshold The minimum threshold to detect the toe off
  /// @param learningRate The learning rate of the analyzer
  WalkingCycleFromDelsysPressureAnalyzer(size_t delsysDeviceIndex,
                                         size_t channelIndex,
                                         double heelStrikeThreshold,
                                         double toeOffThreshold,
                                         double learningRate = 0.2);

public:
  /// @brief Destructor of the WalkingCycleFromDelsysPressureAnalyzer
  ~WalkingCycleFromDelsysPressureAnalyzer() override = default;

protected:
  /// @brief The Delsys device index
  DECLARE_PROTECTED_MEMBER(size_t, DelsysDeviceIndex);

  /// @brief The Delsys channel to gather the data from
  DECLARE_PROTECTED_MEMBER(size_t, ChannelIndex);

  /// @brief The minimum threshold to detect the heel strike
  DECLARE_PROTECTED_MEMBER(double, HeelStrikeThreshold);

  /// @brief The minimum threshold to detect the toe off
  DECLARE_PROTECTED_MEMBER(double, ToeOffThreshold);

  /// @brief The callback to determine if the phase should be incremented
  /// @param data The data to analyze
  bool shouldIncrementPhase(const std::map<size_t, data::TimeSeries> &data);

  /// @brief The callback to get the current time
  /// @param data The data to analyze
  std::chrono::system_clock::time_point
  getCurrentTime(const std::map<size_t, data::TimeSeries> &data);
};

} // namespace NEUROBIO_NAMESPACE::analyzer

#endif // __NEUROBIO_ANALYZER_WALKING_CYCLE_FROM_DELSYS_PRESSURE_ANALYZER_H__