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

class DeviceCommands {
public:
  DeviceCommands() = delete;
  DeviceCommands(int value) : m_Value(value) {}

  virtual std::string toString() const { return "UNKNOWN"; };

protected:
  DECLARE_PROTECTED_MEMBER(int, Value);
};

class DeviceResponses {
public:
  static constexpr int OK = 0;
  static constexpr int NOK = 1;
  static constexpr int COMMAND_NOT_FOUND = 2;
  static constexpr int DEVICE_NOT_CONNECTED = 3;

  /// @brief Constructor from int
  /// @param value The value of the response
  DeviceResponses(int value) : m_Value(value) {}

  /// @brief Constructor from another DeviceResponses object
  /// @param other The other DeviceResponses object
  DeviceResponses(DeviceResponses &&other) noexcept = default;

  /// @brief Assignation operator
  /// @param other The other DeviceResponses object
  /// @return The current DeviceResponses object
  DeviceResponses &operator=(DeviceResponses &&other) noexcept = default;

  /// @brief Copy constructor
  /// @param other The other DeviceResponses object
  DeviceResponses(const DeviceResponses &other) = default;

  /// @brief Assignation operator
  /// @param other The other DeviceResponses object
  /// @return The current DeviceResponses object
  DeviceResponses &operator=(const DeviceResponses &other) = default;

  /// @brief Equality operator
  /// @param other The other DeviceResponses object to compare with
  /// @return True if the two objects are equal, false otherwise
  bool operator==(const DeviceResponses &other) const {
    return m_Value == other.m_Value;
  }

  /// @brief Inequality operator
  /// @param other The other DeviceResponses object to compare with
  /// @return True if the two objects are different, false otherwise
  bool operator!=(const DeviceResponses &other) const {
    return m_Value != other.m_Value;
  }

  /// @brief Get the string representation of the response
  /// @return The string representation of the response
  virtual std::string toString() const {
    switch (m_Value) {
    case OK:
      return "OK";
    case NOK:
      return "NOK";
    case COMMAND_NOT_FOUND:
      return "COMMAND_NOT_FOUND";
    case DEVICE_NOT_CONNECTED:
      return "DEVICE_NOT_CONNECTED";
    default:
      return "UNKNOWN";
    }
  }

protected:
  DECLARE_PROTECTED_MEMBER(int, Value);
};

/// @brief A class representing a device that is remotely connected
/// @note This class is only available on Windows and Linux
class AsyncDevice : public Device {
public:
  /// Constructors
  AsyncDevice();
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

  /// @brief Get how long to wait before waking up the worker
  /// @return How long to wait before waking up the worker
  DECLARE_PROTECTED_MEMBER(std::chrono::milliseconds, KeepWorkerAliveInterval)

  /// @brief Get the keep-alive timer
  /// @return The keep-alive timer
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::steady_timer>,
                                 KeepWorkerAliveTimer)

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
  virtual DeviceResponses parseSendCommand(const DeviceCommands &command,
                                           const std::any &data) = 0;
};

} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_REMOTE_DEVICE_H__