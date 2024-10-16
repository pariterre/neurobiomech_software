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
  /// @param timeSeriesGenerator The time series generator function
  AsyncDataCollector(size_t channelCount,
                     const std::chrono::microseconds &dataCheckIntervals,
                     const std::function<std::unique_ptr<data::TimeSeries>()>
                         &timeSeriesGenerator);
  ~AsyncDataCollector() override;

protected:
  /// Protected members without Get accessors

  /// @brief Get the async context of the command loop
  /// @return The async context of the command loop
  DECLARE_PRIVATE_MEMBER_NOGET(asio::io_context, AsyncDataContext)

  /// @brief Get the mutex
  /// @return The mutex
  DECLARE_PROTECTED_MEMBER_NOGET(std::mutex, AsyncDataMutex)

  /// @brief Worker thread to keep the device alive
  DECLARE_PRIVATE_MEMBER_NOGET(std::thread, AsyncDataWorker)

  /// @brief Get how long to wait before waking up the worker
  /// @return How long to wait before waking up the worker
  DECLARE_PROTECTED_MEMBER(std::chrono::microseconds,
                           KeepDataWorkerAliveInterval)

  /// @brief Get the keep-alive timer
  /// @return The keep-alive timer
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::steady_timer>,
                                 KeepDataWorkerAliveTimer)

public:
  /// @brief Start the data streaming in a asynchronous way (non-blocking). It
  /// is the responsability of the caller to wait for the streaming to actually
  /// start before continuing with other operations
  void startDataStreamingAsync();

  /// @brief Start data streaming in a synchronous way (blocking)
  bool startDataStreaming() override;

  /// @brief Stop data streaming in a synchronous way (blocking).
  bool stopDataStreaming() override;

  bool startRecording() override;
  bool stopRecording() override;

protected:
  /// @brief Stop the worker threads. This can be called by the destructor of
  /// the inherited class so it stops the worker threads before the object is
  /// fully destroyed
  void stopDataCollectorWorkers();

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
