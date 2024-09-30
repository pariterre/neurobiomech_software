#ifndef __STIMWALKER_DEVICES_GENERIC_DEVICE_H__
#define __STIMWALKER_DEVICES_GENERIC_DEVICE_H__

#include "stimwalkerConfig.h"

#include "Utils/CppMacros.h"
#include <iostream>

namespace STIMWALKER_NAMESPACE::devices {

class DeviceCommands {
public:
  DeviceCommands() = delete;
  DeviceCommands(int value) : m_Value(value) {}

  virtual std::string toString() const {
    throw std::runtime_error("This method should be overriden");
  };

protected:
  DECLARE_PROTECTED_MEMBER(int, Value);
};

class DeviceResponses {
public:
  static constexpr int OK = 0;
  static constexpr int NOK = 1;
  static constexpr int COMMAND_NOT_FOUND = 2;

  // Constructor from int
  DeviceResponses(int value) : m_Value(value) {}

  // Use default move semantics
  DeviceResponses(DeviceResponses &&other) noexcept = default;
  DeviceResponses &operator=(DeviceResponses &&other) noexcept = default;

  // Use default copy semantics
  DeviceResponses(const DeviceResponses &other) = default;
  DeviceResponses &operator=(const DeviceResponses &other) = default;

  // String representation of the object
  virtual std::string toString() const {
    switch (m_Value) {
    case OK:
      return "OK";
    case NOK:
      return "NOK";
    case COMMAND_NOT_FOUND:
      return "COMMAND_NOT_FOUND";
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

  /// @brief Destructor
  virtual ~Device() = default;

  /// @brief Connect to the actual device
  virtual void connect() = 0;

  /// @brief Disconnect from the actual device
  virtual void disconnect() = 0;

protected:
  /// @brief Get if the device is connected
  /// @return True if the device is connected, false otherwise
  DECLARE_PROTECTED_MEMBER(bool, IsConnected)
};

} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_GENERIC_DEVICE_H__