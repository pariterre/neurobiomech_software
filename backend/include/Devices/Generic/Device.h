#ifndef __STIMWALKER_DEVICES_GENERIC_DEVICE_H__
#define __STIMWALKER_DEVICES_GENERIC_DEVICE_H__

#include "stimwalkerConfig.h"

#include "Utils/CppMacros.h"
#include <any>
#include <iostream>
#include <vector>

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

/// @brief Abstract class for devices
class Device {
public:
  /// @brief Constructor
  Device() = default;
  Device(const Device &other) = delete;

  /// @brief Destructor. For technical reasons, every classes that inherit from
  /// this class should have a virtual destructor that calls [disconnect] by
  /// themselves
  virtual ~Device();

  /// @brief Get the name of the device
  /// @return The name of the device
  virtual std::string deviceName() const = 0;

  /// @brief Connect to the actual device
  virtual void connect();

  /// @brief Disconnect from the actual device
  virtual void disconnect();

protected:
  /// @brief Handle the actual connexion to the device
  /// @return True if the connection was successful, false otherwise
  virtual bool handleConnect() = 0;

  /// @brief Handle the actual disconnection from the device
  /// @return True if the disconnection was successful, false otherwise
  virtual bool handleDisconnect() = 0;

  /// @brief Get if the device is connected
  /// @return True if the device is connected, false otherwise
  DECLARE_PROTECTED_MEMBER(bool, IsConnected)

  /// @brief Has failed to connect. This is always false unless the
  /// it actually failed to connect
  DECLARE_PROTECTED_MEMBER(bool, HasFailedToConnect)

  /// Send methods
public:
  /// @brief Send a command to the device
  /// @param command The command to send to the device
  /// @param data The optional data to send to the device
  DeviceResponses send(const DeviceCommands &command);
  DeviceResponses send(const DeviceCommands &command, bool data);
  DeviceResponses send(const DeviceCommands &command, const char *data);
  DeviceResponses send(const DeviceCommands &command, const std::any &data);

protected:
  /// @brief Send a command to the device. This method is called by the public
  /// [send] method
  /// @param command The command to send to the device
  /// @param data The data to send to the device
  /// @return The response from the device
  DeviceResponses sendInternal(const DeviceCommands &command,
                               const std::any &data);

  virtual DeviceResponses parseSendCommand(const DeviceCommands &command,
                                           const std::any &data) = 0;
};

} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_GENERIC_DEVICE_H__