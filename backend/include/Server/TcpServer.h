#ifndef __STIMWALKER_SERVER_TCP_SERVER_H__
#define __STIMWALKER_SERVER_TCP_SERVER_H__

#include "stimwalkerConfig.h"

#include "Devices/Devices.h"
#include "Utils/CppMacros.h"
#include <asio.hpp>
#include <mutex>

namespace STIMWALKER_NAMESPACE::server {

enum TcpServerStatus { INITIALIZING, CONNECTED };

enum class TcpServerCommand : std::uint32_t {
  HANDSHAKE,
  CONNECT_DELSYS,
  CONNECT_MAGSTIM,
  DISCONNECT_DELSYS,
  DISCONNECT_MAGSTIM,
  START_RECORDING,
  STOP_RECORDING,
  PAUSE_RECORDING,
  RESUME_RECORDING,
  FAILED,
};

enum class TcpServerResponse : std::uint32_t { OK, NOK };

class TcpServer {
public:
  /// @brief Constructor
  /// @param port The port to listen to (default is 5000)
  TcpServer(int port = 5000);

  /// @brief Destructor
  ~TcpServer();
  TcpServer(const TcpServer &) = delete;

  // ----------------------- //
  // --- DEVICES METHODS --- //
  // ----------------------- //
public:
  /// @brief Start the server. This method is blocking. In order to run it
  /// asynchronously, run it in a separate thread
  void startServer();

  /// @brief Stop the server. This sends a message to the server that it should
  /// stop. After this command, [startServer] will return
  void stopServer();

  /// @brief Disconnect the current client
  void disconnectClient();

protected:
  /// @brief If the server is started
  DECLARE_PROTECTED_MEMBER(bool, IsStarted);

  /// @brief If a client is connected
  DECLARE_PROTECTED_MEMBER(bool, IsClientConnected);

  /// @brief Get the devices that are currently connected
  DECLARE_PROTECTED_MEMBER(devices::Devices, Devices);

protected:
  /// @brief The connected devices sorted by their names
  std::map<std::string, size_t> m_ConnectedDevices;

  // ----------------------------- //
  // --- COMMUNICATION METHODS --- //
  // ----------------------------- //
protected:
  /// @brief Handle the handshake command
  /// @param command The command to handle (this should be HANDSHAKE)
  /// @return True if the handshake is successful, false otherwise
  bool handleHandshake(TcpServerCommand command);

  /// @brief Handle a command
  /// @param command The command to handle
  /// @return True if the command is successful, false otherwise
  bool handleCommand(TcpServerCommand command);

  /// @brief Construct the packet to send to the client from a response
  /// @param response The response to send
  /// @return The packet to send
  std::array<char, 8> constructResponsePacket(TcpServerResponse response);

  /// @brief Parse a packet from the client to get the command
  /// @param buffer The buffer to parse
  /// @return The command sent by the client
  TcpServerCommand parseCommandPacket(const std::array<char, 8> &buffer);

  // -------------------------- //
  // --- TCP SERVER METHODS --- //
  // -------------------------- //
protected:
  /// @brief The port to listen to
  DECLARE_PROTECTED_MEMBER(int, Port);

  /// @brief The socket that is connected to the client
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                                 Socket);

  /// @brief The acceptor that listens to the port
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::acceptor>,
                                 Acceptor);

  /// @brief The current status of the server
  DECLARE_PROTECTED_MEMBER(TcpServerStatus, Status);

  /// @brief Connect the device based on the command
  /// @param command The command to connect the device
  /// @return True if the device is connected, false otherwise
  bool addDevice(TcpServerCommand command);
  virtual bool addDeviceToDevices(TcpServerCommand command);

private:
  /// @brief The asio context used for async methods of the server
  DECLARE_PRIVATE_MEMBER_NOGET(asio::io_context, Context);

  /// @brief The mutex to lock certain operations
  DECLARE_PRIVATE_MEMBER_NOGET(std::mutex, Mutex);

  /// @brief The current protocol version. This must match the client's version
  DECLARE_PRIVATE_MEMBER_NOGET(std::uint32_t, ProtocolVersion)
};

class TcpServerMock : public TcpServer {

public:
  /// @brief Constructor
  /// @param port The port to listen to (default is 5000)
  TcpServerMock(int port = 5000) : TcpServer(port) {};

  /// @brief Destructor
  ~TcpServerMock() = default;
  TcpServerMock(const TcpServerMock &) = delete;

protected:
  bool addDeviceToDevices(TcpServerCommand command) override;
};

} // namespace STIMWALKER_NAMESPACE::server

#endif // __STIMWALKER_SERVER_TCP_SERVER_H__