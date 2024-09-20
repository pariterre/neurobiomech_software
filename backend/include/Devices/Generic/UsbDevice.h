#ifndef __STIMWALKER_DEVICES_USB_DEVICE_H__
#define __STIMWALKER_DEVICES_USB_DEVICE_H__

#include "stimwalkerConfig.h"

#include <asio.hpp>
#include <iostream>
#include <vector>

#include "UsbExceptions.h"
#include "Utils/CppMacros.h"

// https://github.com/nicolasmcnair/magpy/blob/master/magpy/magstim.py#L129
// https://github.com/nigelrogasch/MAGIC/blob/master/magstim.m#L301

namespace STIMWALKER_NAMESPACE::devices {

class UsbCommands {
public:
  static constexpr int PRINT = 0;

  UsbCommands() = delete;
  UsbCommands(int value) : m_Value(value) {}

  virtual std::string toString() const {
    switch (m_Value) {
    case PRINT:
      return "PRINT";
    default:
      return "UNKNOWN";
    }
  }

protected:
  DECLARE_PROTECTED_MEMBER(int, Value);
};

class UsbResponses {
public:
  static constexpr int OK = 0;
  static constexpr int ERROR = 1;
  static constexpr int COMMAND_NOT_FOUND = 2;

  // Constructor from int
  UsbResponses(int value) : m_Value(value) {}

  // Use default move semantics
  UsbResponses(UsbResponses &&other) noexcept = default;
  UsbResponses &operator=(UsbResponses &&other) noexcept = default;

  // Use default copy semantics
  UsbResponses(const UsbResponses &other) = default;
  UsbResponses &operator=(const UsbResponses &other) = default;

protected:
  DECLARE_PROTECTED_MEMBER(int, Value);
};

/// @brief A class representing a USB device
/// @details This class provides a way to list all USB devices connected to
/// the system and get their information
/// @note This class is only available on Windows and Linux
class UsbDevice {
public:
  /// Constructors
public:
  /// @brief Constructor
  /// @param port The port name of the device
  /// @param vid The vendor ID of the device
  /// @param pid The product ID of the device
  UsbDevice(const std::string &port, const std::string &vid,
            const std::string &pid);

  /// @brief Copy constructor
  /// @param other The other UsbDevice object to copy
  UsbDevice(const UsbDevice &other);

  /// @brief Factory method to create a UsbDevice object from a vendor ID and
  /// product ID. Throws an exception if the device is not found
  /// @param vid The vendor ID of the device
  /// @param pid The product ID of the device
  /// @return A UsbDevice object with the specified vendor ID and product ID.
  /// Throws an exception if the device is not found
  static UsbDevice fromVidAndPid(const std::string &vid,
                                 const std::string &pid);

protected:
  /// Protected members with Get accessors
  /// @brief Get the port name of the device
  /// @return The port name of the device
  DECLARE_PROTECTED_MEMBER(std::string, Port)

  /// @brief Get the vendor ID of the device
  /// @return The vendor ID of the device
  DECLARE_PROTECTED_MEMBER(std::string, Vid)

  /// @brief Get the product ID of the device
  /// @return The product ID of the device
  DECLARE_PROTECTED_MEMBER(std::string, Pid)

  /// Protected members without Get accessors

  /// @brief Get the serial port of the device
  /// @return The serial port of the device
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::serial_port>, SerialPort)

  /// @brief Get the async context of the device
  /// @return The async context of the device
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::io_context>,
                                 SerialPortContext)

  /// @brief Get the async context of the command loop
  /// @return The async context of the command loop
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::io_context>, Context)

  /// @brief Get the mutex
  /// @return The mutex
  DECLARE_PROTECTED_MEMBER_NOGET(std::mutex, Mutex)

  /// @brief Worker thread to keep the device alive
  DECLARE_PROTECTED_MEMBER_NOGET(std::thread, Worker)

  /// Methods
public:
  /// @brief Connect the device
  void connect();

  /// @brief Disconnect the device
  void disconnect();

  /// @brief Send a command to the device
  /// @param command The command to send to the device
  /// @param data The data to send to the device
  /// @param ignoreResponse True to ignore the response, false otherwise
  UsbResponses send(const UsbCommands &command, const std::any &data,
                    bool ignoreResponse = false);
  UsbResponses send(const UsbCommands &command, const char *data,
                    bool ignoreResponse = false) {
    return send(command, std::string(data), ignoreResponse);
  }

protected:
  /// @brief Connect to the ubs device. This is expected to run on an async
  /// thread
  virtual void _initialize();

  /// @brief Parse a command received from the user and send to the device
  /// @param command The command to parse
  /// @param data The data to parse
  virtual UsbResponses _parseCommand(const UsbCommands &command,
                                     const std::any &data);

  /// @brief Connect to the serial port
  virtual void _connectSerialPort();

  /// @brief Set the "RTS" mode of the communication. [isFast] to true is
  /// faster but less reliable.
  /// @param isFast True to enable fast mode, false to disable it
  /// @note This methods emulates the useRTS signal from Python
  virtual void _setFastCommunication(bool isFast);

  /// Static helper methods
public:
  /// @brief Static method to list all USB devices connected to the system
  /// @return A vector of UsbDevice objects representing the connected USB
  /// devices
  static std::vector<UsbDevice> listAllUsbDevices();

  /// @brief Equality operator
  /// @param other The other UsbDevice object to compare with
  /// @return True if the two objects are equal, false otherwise
  bool operator==(const UsbDevice &other) const;
};

} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_USB_DEVICE_H__