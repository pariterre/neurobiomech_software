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
  GET_DATA,
  FAILED,
};

enum class TcpServerResponse : std::uint32_t { OK, NOK };

class TcpServer {
public:
  /// @brief Constructor
  /// @param commandPort The port to communicate the commands (default is 5000)
  /// @param dataPort The port to communicate the data (default is 5001)
  TcpServer(int commandPort = 5000, int dataPort = 5001);

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
  /// @brief Wait for a new connexion
  void waitForNewConnexion();

  /// @brief Wait for a new command
  /// @return True if everything is okay, False if the server is shutting down
  void waitAndHandleNewCommand();

  /// @brief If the server is started
  DECLARE_PROTECTED_MEMBER(bool, IsStarted);

  DECLARE_PROTECTED_MEMBER(bool, IsShuttingDown);

  /// @brief If a client is connected
  DECLARE_PROTECTED_MEMBER(bool, IsClientConnected);

  /// @brief Get the devices that are currently connected
  DECLARE_PROTECTED_MEMBER(devices::Devices, Devices);

protected:
  /// @brief The id of the connected devices
  std::map<std::string, size_t> m_ConnectedDeviceIds;

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
  /// @brief The port to listen to communicate the commands
  DECLARE_PROTECTED_MEMBER(int, CommandPort);

  /// @brief The port to listen to communicate the data
  DECLARE_PROTECTED_MEMBER(int, DataPort);

  /// @brief The socket that is connected to the client for commands
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                                 CommandSocket);

  /// @brief The socket that is connected to the client for data
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                                 DataSocket);

  /// @brief The acceptor that listens to the command port
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::acceptor>,
                                 CommandAcceptor);

  /// @brief The acceptor that listens to the data port
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::acceptor>,
                                 DataAcceptor);

  /// @brief The current status of the server
  DECLARE_PROTECTED_MEMBER(TcpServerStatus, Status);

  /// @brief Add the requested device
  /// @param deviceName The name of the device to add
  /// @return True if the device is added, false otherwise
  bool addDevice(const std::string &deviceName);

  /// @brief Make the device and add it to the devices. This method is called by
  /// [addDevice] when it is confirmed that it is possible to actually add the
  /// device (no check is expected to be done in this method). This is a virtual
  /// method that can be overridden by the mocker to simulate the addition of a
  /// device
  /// @param deviceName The name of the device to add
  virtual void makeAndAddDevice(const std::string &deviceName);

  /// @brief Remove the requested device
  /// @param deviceName The name of the device to remove
  /// @return True if the device is removed, false otherwise
  bool removeDevice(const std::string &deviceName);

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
  /// @param commandPort The port to communicate the commands (default is 5000)
  /// @param dataPort The port to communicate the data (default is 5001)
  TcpServerMock(int commandPort = 5000, int dataPort = 5001)
      : TcpServer(commandPort, dataPort) {};

  /// @brief Destructor
  ~TcpServerMock() = default;
  TcpServerMock(const TcpServerMock &) = delete;

protected:
  void makeAndAddDevice(const std::string &deviceName) override;
};

} // namespace STIMWALKER_NAMESPACE::server

#endif // __STIMWALKER_SERVER_TCP_SERVER_H__