#ifndef __STIMWALKER_DEVICES_GENERIC_DATA_COLLECTOR_H__
#define __STIMWALKER_DEVICES_GENERIC_DATA_COLLECTOR_H__

#include "stimwalkerConfig.h"
#include <functional>
#include <vector>

#include "Devices/Data/DataPoint.h"
#include "Devices/Generic/Device.h"
#include "Utils/CppMacros.h"
#include "Utils/StimwalkerEvent.h"

namespace STIMWALKER_NAMESPACE::devices {

/// @brief Abstract class for data collectors
class DataCollector {
public:
  /// @brief Constructor
  /// @param frameRate The frame rate of the device
  DataCollector(size_t channelCount, size_t frameRate)
      : m_DataChannelCount(channelCount), m_FrameRate(frameRate),
        m_IsRecording(false) {}

  /// @brief Destructor
  virtual ~DataCollector() = default;

  /// @brief Start collecting data
  virtual void startRecording() = 0;

  /// @brief Stop collecting data
  virtual void stopRecording() = 0;

protected:
  /// @brief Get the number of channels
  /// @return The number of channels
  DECLARE_PROTECTED_MEMBER(size_t, DataChannelCount)

  /// @brief Get the frame rate
  /// @return The frame rate
  DECLARE_PROTECTED_MEMBER(size_t, FrameRate)

  /// @brief Get if the device is currently recording
  /// @return True if the device is recording, false otherwise
  DECLARE_PROTECTED_MEMBER(bool, IsRecording)

public:
  /// @brief Set the callback function to call when data is collected
  /// @param callback The callback function
  StimwalkerEvent<data::DataPoint> onNewData;

protected:
  /// @brief Method that is internally called when new data are ready. It is
  /// expected to be called by the device and then call the onNewData callback
  /// @param data The new data to handle
  virtual void HandleNewData(const data::DataPoint &data) = 0;
};

} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_GENERIC_DATA_COLLECTOR_H__
