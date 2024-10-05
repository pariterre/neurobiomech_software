#ifndef __STIMWALKER_DEVICES_USB_DEVICE_H__
#define __STIMWALKER_DEVICES_USB_DEVICE_H__

#include "stimwalkerConfig.h"

#include "Devices/Generic/SerialPortDevice.h"
#include "Utils/CppMacros.h"

namespace STIMWALKER_NAMESPACE::devices {

/// @brief A class representing a USB device
/// @note This class is only available on Windows and Linux
class UsbDevice : public SerialPortDevice {
public:
  /// Constructors
public:
  /// @brief Constructor
  /// @param port The port name of the device
  /// @param vid The vendor ID of the device
  /// @param pid The product ID of the device
  UsbDevice(const std::string &port, const std::string &vid,
            const std::string &pid);
  UsbDevice(const UsbDevice &other) = delete;

  std::string deviceName() const override;

  /// @brief Factory method to create a UsbDevice object from a vendor ID
  /// and product ID. Throws an exception if the device is not found
  /// @param vid The vendor ID of the device
  /// @param pid The product ID of the device
  /// @return A UsbDevice object with the specified vendor ID and product
  /// ID. Throws an exception if the device is not found
  static UsbDevice fromVidAndPid(const std::string &vid,
                                 const std::string &pid);

protected:
  /// @brief Get the vendor ID of the device
  /// @return The vendor ID of the device
  DECLARE_PROTECTED_MEMBER(std::string, Vid)

  /// @brief Get the product ID of the device
  /// @return The product ID of the device
  DECLARE_PROTECTED_MEMBER(std::string, Pid)

  /// Methods
protected:
  /// @brief Parse a command received from the user and send to the device
  /// @param command The command to parse
  /// @param data The data to parse
  virtual DeviceResponses parseAsyncSendCommand(const DeviceCommands &command,
                                                const std::any &data) override;

  /// Static helper methods
public:
  /// @brief Static method to list all USB devices connected to the system
  /// @return A vector of UsbDevice objects representing the connected USB
  /// devices
  static std::vector<std::unique_ptr<UsbDevice>> listAllUsbDevices();
};

} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_USB_DEVICE_H__