#ifndef __STIMWALKER_DEVICES_GENERIC_DEVICE_H__
#define __STIMWALKER_DEVICES_GENERIC_DEVICE_H__

#include "stimwalkerConfig.h"

#include "Utils/CppMacros.h"
#include <iostream>

namespace STIMWALKER_NAMESPACE::devices {

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