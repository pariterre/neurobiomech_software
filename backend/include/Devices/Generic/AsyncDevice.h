#ifndef __STIMWALKER_DEVICES_GENERIC_REMOTE_DEVICE_H__
#define __STIMWALKER_DEVICES_GENERIC_REMOTE_DEVICE_H__

#include "Devices/Generic/Device.h"
#include <asio.hpp>

namespace STIMWALKER_NAMESPACE::devices {

/// @brief A class representing a device that is remotely connected
/// @note This class is only available on Windows and Linux
class AsyncDevice : public Device {
public:
  /// @brief Constructor
  /// @param keepAliveInterval The interval to keep the device alive
  AsyncDevice(const std::chrono::microseconds &keepAliveInterval);
  AsyncDevice(const AsyncDevice &other) = delete;

  /// @brief Send a command to the device without waiting for a response
  /// @param command The command to send to the device
  /// @param data The optional data to send to the device
  DeviceResponses sendFast(const DeviceCommands &command);
  DeviceResponses sendFast(const DeviceCommands &command, const char *data);
  DeviceResponses sendFast(const DeviceCommands &command, const std::any &data);

protected:
  /// Protected members without Get accessors

  /// @brief Get the async context of the command loop
  /// @return The async context of the command loop
  DECLARE_PRIVATE_MEMBER_NOGET(asio::io_context, AsyncDeviceContext)

  /// @brief Get the mutex
  /// @return The mutex
  DECLARE_PROTECTED_MEMBER_NOGET(std::mutex, AsyncDeviceMutex)

  /// @brief Worker thread to keep the device alive
  DECLARE_PRIVATE_MEMBER_NOGET(std::thread, AsyncDeviceWorker)

  /// @brief Get how long to wait before waking up the worker
  /// @return How long to wait before waking up the worker
  DECLARE_PROTECTED_MEMBER(std::chrono::microseconds,
                           KeepDeviceWorkerAliveInterval)

  /// @brief Get the keep-alive timer
  /// @return The keep-alive timer
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::steady_timer>,
                                 KeepDeviceWorkerAliveTimer)

public:
  /// @brief Start the connection in a asynchronous way (non-blocking). It is
  /// the responsability of the caller to wait for the connection to start
  /// before continuing
  void connectAsync();

  /// @brief  Start the connection in a synchronous way (blocking)
  void connect() override;

  /// @brief Start the disconnection in a synchronous way (blocking).
  void disconnect() override;

protected:
  /// @brief Stop the worker threads. This can be called by the destructor of
  /// the inherited class so it stops the worker threads before the object is
  /// fully destroyed
  virtual void stopDeviceWorkers();

protected:
  /// @brief Send a command to the device without waiting for a response
  /// @param command The command to send to the device
  /// @param data The optional data to send to the device
  /// @return The response from the device
  DeviceResponses sendInternalFast(const DeviceCommands &command,
                                   const std::any &data);

  DeviceResponses parseSendCommand(const DeviceCommands &command,
                                   const std::any &data) override;

  DeviceResponses parseSendCommand(const DeviceCommands &command,
                                   const std::any &data, bool ignoreResponse);

  /// @brief This method replaces the [parseSendCommand] that should be
  /// implemented by the inherited class. This method is called by the worker
  /// thread to send a command to the device
  /// @param command The command to send
  /// @param data The data to send
  virtual DeviceResponses parseAsyncSendCommand(const DeviceCommands &command,
                                                const std::any &data) = 0;

  /// @brief Start the keep-alive mechanism
  virtual void startKeepDeviceWorkerAlive();

  /// @brief Set a worker thread to keep the device alive
  /// @param timeout The time to wait before sending the next keep-alive
  /// command. This usually is the [KeepWorkerAliveInterval] value, but can be
  /// overridden
  virtual void keepDeviceWorkerAlive(std::chrono::milliseconds timeout);
  virtual void keepDeviceWorkerAlive(std::chrono::microseconds timeout);

  /// @brief Send a PING command to the device, if required. This method is
  /// called by the [keepWorkerAlive] method at regular intervals (see
  /// [KeepWorkerAliveInterval]) If this method is not overridden, it will do
  /// nothing, but still keep the worker alive
  virtual void pingDeviceWorker();
};

} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_REMOTE_DEVICE_H__