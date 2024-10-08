#ifndef __STIMWALKER_DEVICES_GENERIC_ASYNC_DATA_COLLECTOR_H__
#define __STIMWALKER_DEVICES_GENERIC_ASYNC_DATA_COLLECTOR_H__

#include "Devices/Generic/DataCollector.h"
#include <asio.hpp>

namespace STIMWALKER_NAMESPACE::devices {

/// @brief Abstract class for data collectors
class AsyncDataCollector : public DataCollector {
public:
  /// @brief Constructor
  /// @param frameRate The frame rate of the device
  /// @param dataCheckIntervals The interval to check for new data
  /// @param timeSeries The time series to store the data
  AsyncDataCollector(size_t channelCount,
                     const std::chrono::milliseconds &dataCheckIntervals,
                     std::unique_ptr<data::TimeSeries> timeSeries);
  AsyncDataCollector(size_t channelCount,
                     const std::chrono::microseconds &dataCheckIntervals,
                     std::unique_ptr<data::TimeSeries> timeSeries);

protected:
  /// Protected members without Get accessors

  /// @brief Get the async context of the command loop
  /// @return The async context of the command loop
  DECLARE_PROTECTED_MEMBER_NOGET(asio::io_context, AsyncDataContext)

  /// @brief Get the mutex
  /// @return The mutex
  DECLARE_PROTECTED_MEMBER_NOGET(std::mutex, AsyncDataMutex)

  /// @brief Worker thread to keep the device alive
  DECLARE_PROTECTED_MEMBER_NOGET(std::thread, AsyncDataWorker)

  /// @brief Get how long to wait before waking up the worker
  /// @return How long to wait before waking up the worker
  DECLARE_PROTECTED_MEMBER(std::chrono::microseconds,
                           KeepDataWorkerAliveInterval)

  /// @brief Get the keep-alive timer
  /// @return The keep-alive timer
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::steady_timer>,
                                 KeepDataWorkerAliveTimer)

protected:
  bool handleStartRecording() override;

  /// @brief Start collecting data asynchronously. This method replaces the
  /// [handleStartRecording] method that should be implemented by inherited
  /// classes. If starting the recording fails, this method should return false.
  /// @return True if the recording started successfully, false otherwise
  virtual bool handleAsyncStartRecording() = 0;

  void handleStopRecording() override;

  /// @brief Stop collecting data asynchronously. This method replaces the
  /// [handleStopRecording] method that should be implemented by inherited
  /// classes
  virtual void handleAsyncStopRecording() = 0;

protected:
  /// @brief Start the keep-alive mechanism
  virtual void startKeepDataWorkerAlive();

  /// @brief Set a worker thread to keep the device alive
  /// @param timeout The time to wait before sending the next keep-alive
  /// command. This usually is the [KeepWorkerAliveInterval] value, but can be
  /// overridden
  virtual void keepDataWorkerAlive(std::chrono::milliseconds timeout);
  virtual void keepDataWorkerAlive(std::chrono::microseconds timeout);

  /// @brief This method is called by the [keepWorkerAlive] method at regular
  /// intervals (see [KeepWorkerAliveInterval]) If this method is not
  /// overridden, it will do nothing, but will keep the worker alive.
  /// Otherwise, it can be used to get data from the device or to perform any
  /// other action such as analyzing the data
  virtual void dataCheck();

  /// @brief The [m_IgnoreTooSlowWarning] member can be set by an inherited
  /// class to ignore the warning that is displayed when the [dataCheck] method
  /// takes too long to execute compared to the [KeepWorkerAliveInterval]
  DECLARE_PROTECTED_MEMBER(bool, IgnoreTooSlowWarning)
};

} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_GENERIC_ASYNC_DATA_COLLECTOR_H__
