#ifndef __STIMWALKER_DEVICES_TCP_DEVICE_H__
#define __STIMWALKER_DEVICES_TCP_DEVICE_H__

#include "Devices/Generic/AsyncDevice.h"

namespace STIMWALKER_NAMESPACE::devices {

/// @brief A class representing a Tcp device
/// @note This class is only available on Windows and Linux
class TcpDevice : public AsyncDevice {
public:
  /// @brief Constructor
  /// @param host The host name of the device
  /// @param port The port number of the device
  TcpDevice(const std::string &host, size_t port);
  TcpDevice(const TcpDevice &other) = delete;

  /// @brief Read data from the tcp device
  /// @param bufferSize The size of the buffer to read
  /// @return The data read from the device
  virtual std::vector<char> read(size_t bufferSize);

protected:
  /// Protected members with Get accessors

  /// @brief Get the host name of the device
  /// @return The host name of the device
  DECLARE_PROTECTED_MEMBER(std::string, Host)

  /// @brief Get the port number of the device
  /// @return The port number of the device
  DECLARE_PROTECTED_MEMBER(size_t, Port)

  /// Protected members without Get accessors

  /// @brief Get the async context of the device
  /// @return The async context of the device
  DECLARE_PROTECTED_MEMBER_NOGET(asio::io_context, TcpContext)

  /// @brief Get the tcp socket of the device
  /// @return The tcp socket of the device
  DECLARE_PROTECTED_MEMBER_NOGET(asio::ip::tcp::socket, TcpSocket)

  /// Methods
public:
  void disconnect() override;

protected:
  /// @brief Connect to the tcp device. This is expected to run on an
  /// async thread
  void handleConnect() override;
};

} // namespace STIMWALKER_NAMESPACE::devices
#endif // __STIMWALKER_DEVICES_TCP_DEVICE_H__