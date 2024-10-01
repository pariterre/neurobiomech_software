#ifndef __STIMWALKER_DEVICES_REMOTE_DEVICE_H__
#define __STIMWALKER_DEVICES_REMOTE_DEVICE_H__

#include "stimwalkerConfig.h"

#include <asio.hpp>
#include <iostream>
#include <vector>

#include "Devices/Generic/Device.h"
#include "Exceptions.h"
#include "Utils/CppMacros.h"

namespace STIMWALKER_NAMESPACE::devices {

/// @brief A class representing a device that is remotely connected
/// @note This class is only available on Windows and Linux
class AsyncDevice : public Device {
public:
  /// Constructors
  AsyncDevice() = default;
  AsyncDevice(const AsyncDevice &other) = delete;
  ~AsyncDevice();

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

  /// Methods
public:
  /// @brief This is a concrete method that starts the worker. When overriding
  /// this class, one should override the [handleConnect] method instead
  void connect() override;

  /// @brief Disconnect from the actual device
  void disconnect() override;

  /// @brief Send a command to the device
  /// @param command The command to send to the device
  /// @param data The optional data to send to the device
  DeviceResponses send(const DeviceCommands &command);
  DeviceResponses send(const DeviceCommands &command, const char *data);
  DeviceResponses send(const DeviceCommands &command, const std::any &data);

  /// @brief Send a command to the device without waiting for a response
  /// @param command The command to send to the device
  /// @param data The optional data to send to the device
  DeviceResponses sendFast(const DeviceCommands &command);
  DeviceResponses sendFast(const DeviceCommands &command, const char *data);
  DeviceResponses sendFast(const DeviceCommands &command, const std::any &data);

protected:
  /// @brief Initialize the device. When using an async device, one should not
  /// override the connect method but this one instead
  virtual void handleConnect() = 0;

  /// @brief Send a command to the device. This method is called by the public
  /// [send] method
  /// @param command The command to send to the device
  /// @param data The data to send to the device
  /// @param ignoreResponse True to ignore the response, false otherwise
  /// @return The response from the device
  DeviceResponses sendInternal(const DeviceCommands &command,
                               const std::any &data, bool ignoreResponse);

  /// @brief Parse a command received from the user and send to the device
  /// @param command The command to parse
  /// @param data The data to parse
  virtual DeviceResponses parseCommand(const DeviceCommands &command,
                                       const std::any &data);
};

} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_REMOTE_DEVICE_H__