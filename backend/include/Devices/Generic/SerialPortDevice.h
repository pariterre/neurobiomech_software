#ifndef __STIMWALKER_DEVICES_SERIAL_PORT_DEVICE_H__
#define __STIMWALKER_DEVICES_SERIAL_PORT_DEVICE_H__

#include "Devices/Generic/AsyncDevice.h"

namespace STIMWALKER_NAMESPACE::devices {

/// @brief A class representing a Serial port device
/// @note This class is only available on Windows and Linux
class SerialPortDevice : public AsyncDevice {

public:
  /// @brief Constructor
  /// @param port The port name of the device
  SerialPortDevice(const std::string &port);

  /// @brief Copy constructor
  /// @param other The other SerialPortResponses object to copy
  SerialPortDevice(const SerialPortDevice &other);

protected:
  /// Protected members with Get accessors
  /// @brief Get the port name of the device
  /// @return The port name of the device
  DECLARE_PROTECTED_MEMBER(std::string, Port)

  /// Protected members without Get accessors

  /// @brief Get the serial port of the device
  /// @return The serial port of the device
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::serial_port>, SerialPort)

  /// @brief Get the async context of the device
  /// @return The async context of the device
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::io_context>,
                                 SerialPortContext)

  /// Methods
public:
  void disconnect() override;

protected:
  /// @brief Connect to the serial port device
  virtual void handleConnect() override;

  /// @brief Connect to the serial port
  virtual void connectSerialPort();

  /// @brief Set the "RTS" mode of the communication. [isFast] to true is
  /// faster but less reliable.
  /// @param isFast True to enable fast mode, false to disable it
  /// @note This methods emulates the useRTS signal from Python
  virtual void setFastCommunication(bool isFast);

  /// Static helper methods
public:
  /// @brief Equality operator
  /// @param other The other SerialPortDevice object to compare with
  /// @return True if the two objects are equal, false otherwise
  bool operator==(const SerialPortDevice &other) const;
};

} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_SERIAL_PORT_DEVICE_H__