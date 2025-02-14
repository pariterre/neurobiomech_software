#include "Analyzer/WalkingCycleFromDelsysPressureAnalyzer.h"

#include "Data/TimeSeries.h"

using namespace NEUROBIO_NAMESPACE::analyzer;

WalkingCycleFromDelsysPressureAnalyzer::WalkingCycleFromDelsysPressureAnalyzer(
    size_t delsysDeviceIndex, size_t channelIndex, double heelStrikeThreshold,
    double toeOffThreshold, double learningRate)
    : m_DelsysDeviceIndex(delsysDeviceIndex), m_ChannelIndex(channelIndex),
      m_HeelStrikeThreshold(heelStrikeThreshold),
      m_ToeOffThreshold(toeOffThreshold),
      TimedEventsLiveAnalyzer(
          {std::chrono::milliseconds(400), std::chrono::milliseconds(600)},
          [this](const std::map<size_t, data::TimeSeries> &data) {
            return shouldIncrementPhase(data);
          },
          [this](const std::map<size_t, data::TimeSeries> &data) {
            return getCurrentTime(data);
          },
          learningRate) {}

bool WalkingCycleFromDelsysPressureAnalyzer::shouldIncrementPhase(
    const std::map<size_t, data::TimeSeries> &data) {
  // Get the data from the device
  double channel =
      data.at(m_DelsysDeviceIndex).back().getData().at(m_ChannelIndex);

  if (m_CurrentPhaseIndex == 0 && channel >= m_HeelStrikeThreshold) {
    return true;
  } else if (m_CurrentPhaseIndex == 1 && channel <= m_ToeOffThreshold) {
    return true;
  }
  return false;
}

std::chrono::system_clock::time_point
WalkingCycleFromDelsysPressureAnalyzer::getCurrentTime(
    const std::map<size_t, data::TimeSeries> &data) {
  // Get the data from the device
  const auto &dataDevice = data.at(m_DelsysDeviceIndex);

  // Get the current time stamp by adding passed time to starting time
  return dataDevice.getStartingTime() + dataDevice.back().getTimeStamp();
}
