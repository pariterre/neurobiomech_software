#ifndef __STIMWALKER_DEVICES_REMOTE_DEVICE_H__
#define __STIMWALKER_DEVICES_REMOTE_DEVICE_H__

#include "Devices/Generic/Device.h"

namespace STIMWALKER_NAMESPACE::devices {

/// @brief A class representing a device that is remotely connected
/// @note This class is only available on Windows and Linux
class AsyncDevice : public Device {
public:
  /// Constructors
  AsyncDevice();
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
  DECLARE_PROTECTED_MEMBER_NOGET(asio::io_context, AsyncContext)

  /// @brief Get the mutex
  /// @return The mutex
  DECLARE_PROTECTED_MEMBER_NOGET(std::mutex, AsyncMutex)

  /// @brief Worker thread to keep the device alive
  DECLARE_PROTECTED_MEMBER_NOGET(std::thread, AsyncWorker)

  /// @brief Get how long to wait before waking up the worker
  /// @return How long to wait before waking up the worker
  DECLARE_PROTECTED_MEMBER(std::chrono::milliseconds, KeepWorkerAliveInterval)

  /// @brief Get the keep-alive timer
  /// @return The keep-alive timer
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::steady_timer>,
                                 KeepWorkerAliveTimer)

protected:
  void handleConnect() override;
  void handleDisconnect() override;

  /// @brief Handle the actual connection to the device
  virtual void handleAsyncConnect() = 0;

  /// @brief Handle the actual disconnection from the device
  virtual void handleAsyncDisconnect() = 0;

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

  virtual DeviceResponses parseAsyncSendCommand(const DeviceCommands &command,
                                                const std::any &data) = 0;

  /// @brief Start the keep-alive mechanism
  virtual void startKeepWorkerAlive();

  /// @brief Set a worker thread to keep the device alive
  /// @param timeout The time to wait before sending the next keep-alive
  /// command. This usually is the [KeepWorkerAliveInterval] value, but can be
  /// overridden
  virtual void keepWorkerAlive(std::chrono::milliseconds timeout);

  /// @brief Send a PING command to the device, if required. This method is
  /// called by the [keepWorkerAlive] method at regular intervals (see
  /// [KeepWorkerAliveInterval]) If this method is not overridden, it will do
  /// nothing, but still keep the worker alive
  virtual void pingWorker();
};

} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_REMOTE_DEVICE_H__