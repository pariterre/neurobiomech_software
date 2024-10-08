#ifndef __STIMWALKER_DEVICES_GENERIC_DATA_COLLECTOR_H__
#define __STIMWALKER_DEVICES_GENERIC_DATA_COLLECTOR_H__

#include "stimwalkerConfig.h"
#include <functional>
#include <vector>

#include "Data/TimeSeries.h"
#include "Devices/Generic/Device.h"
#include "Utils/CppMacros.h"
#include "Utils/StimwalkerEvent.h"

namespace STIMWALKER_NAMESPACE::devices {

/// @brief Abstract class for data collectors
class DataCollector {
public:
  /// @brief Constructor
  /// @param channelCount The number of channels
  DataCollector(size_t channelCount)
      : m_DataChannelCount(channelCount), m_IsRecording(false) {}

  /// @brief Destructor
  virtual ~DataCollector() = default;

  /// @brief Get the DataCollector name
  /// @return The name of the DataCollector
  virtual std::string dataCollectorName() const = 0;

  /// @brief Start collecting data
  virtual void startRecording();

  /// @brief Stop collecting data
  virtual void stopRecording();

protected:
  /// @brief Method to handle the start recording command. This is called on the
  /// thread recording the data before starting the recording. If something goes
  /// wrong, an exception should be thrown
  virtual bool handleStartRecording() = 0;

  /// @brief Method to handle the stop recording command. This is called on the
  /// thread recording the data before stopping the recording. If something goes
  /// wrong, an exception should be thrown
  virtual void handleStopRecording() = 0;

  /// @brief Get the number of channels
  /// @return The number of channels
  DECLARE_PROTECTED_MEMBER(size_t, DataChannelCount)

  /// @brief Get if the device is currently recording
  /// @return True if the device is recording, false otherwise
  DECLARE_PROTECTED_MEMBER(bool, IsRecording)

  /// @brief The timeseries data of the previous/current recording
  DECLARE_PROTECTED_MEMBER_NOGET(data::TimeSeries, TimeSeries)

public:
  const data::TimeSeries &getTrialData() const;

  /// @brief Set the callback function to call when data is collected
  /// @param callback The callback function
  StimwalkerEvent<data::DataPoint> onNewData;

protected:
  /// @brief This method adds a new data point to the data collector. This
  /// method should not be overridden but must be called by the inherited class
  /// when new data are ready. Once the new data are added, the [onNewData]
  /// callback is called
  /// @param data The new data to add
  virtual void addDataPoint(const data::DataPoint &data);

  /// @brief Add a vector of data points to the data collector. This method is
  /// strictly equivalent to calling [addDataPoint] for each data point in the
  /// vector, except that the notification is done only once at the end. It is
  /// called with the last data point in the vector
  /// @param dataPoints The data points to add
  virtual void addDataPoints(const std::vector<data::DataPoint> &dataPoints);

  /// @brief This method is useless and only serves as a reminder that the
  /// inherited class should call [addDataPoint] when new data are ready
  virtual void handleNewData(const data::DataPoint &data) = 0;
};

} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_GENERIC_DATA_COLLECTOR_H__
