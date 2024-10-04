#ifndef __STIMWALKER_DEVICES_GENERIC_ASYNC_DATA_COLLECTOR_H__
#define __STIMWALKER_DEVICES_GENERIC_ASYNC_DATA_COLLECTOR_H__

#include "Devices/Generic/DataCollector.h"

namespace STIMWALKER_NAMESPACE::devices {

/// @brief Abstract class for data collectors
class AsyncDataCollector : public DataCollector {
public:
  /// @brief Constructor
  /// @param frameRate The frame rate of the device
  AsyncDataCollector(size_t channelCount, size_t frameRate)
      : m_DataChannelCount(channelCount), m_FrameRate(frameRate),
        m_IsRecording(false) {}

protected:
  /// @brief Start collecting data
  void handleStartRecording() override;

  /// @brief Stop collecting data
  void handleStopRecording() override;
};

} // namespace STIMWALKER_NAMESPACE::devices

#endif __STIMWALKER_DEVICES_GENERIC_ASYNC_DATA_COLLECTOR_H__
