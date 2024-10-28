#ifndef __STIMWALKER_SERVER_TCP_CLIENT_H__
#define __STIMWALKER_SERVER_TCP_CLIENT_H__

#include "stimwalkerConfig.h"

#include "Data/TimeSeries.h"
#include "Server/TcpServer.h"
#include "Utils/CppMacros.h"
#include <asio.hpp>

namespace STIMWALKER_NAMESPACE::server {

class TcpClient {
public:
  /// @brief Constructor
  /// @param host The host to connect to
  /// @param commandPort The port to communicate the commands (default is 5000)
  /// @param responsePort The port to communicate the resonses (default is 5001)
  TcpClient(std::string host = "localhost", int commandPort = 5000,
            int responsePort = 5001);

  /// @brief Destructor
  ~TcpClient();
  TcpClient(const TcpClient &) = delete;

  /// @brief Connect to the server
  /// @return True if the connection is successful, false otherwise
  bool connect();

  /// @brief Disconnect from the server
  /// @return True if the disconnection is successful, false otherwise
  bool disconnect();

  /// @brief Add the Delsys device
  /// @return True if the device is added, false otherwise
  bool addDelsysDevice();

  /// @brief Add the Magstim device
  /// @return True if the device is added, false otherwise
  bool addMagstimDevice();

  /// @brief Remove the Delsys device
  /// @return True if the device is removed, false otherwise
  bool removeDelsysDevice();

  /// @brief Remove the Magstim device
  /// @return True if the device is removed, false otherwise
  bool removeMagstimDevice();

  /// @brief Start recording data
  /// @return True if the recording is started, false otherwise
  bool startRecording();

  /// @brief Stop recording data
  /// @return True if the recording is stopped, false otherwise
  bool stopRecording();

  /// @brief Get the data from the previously recorded trial on the server
  /// @return True if the data is received, false otherwise
  std::map<std::string, data::TimeSeries> getLastTrialData();

protected:
  /// @brief The data received from the server
  DECLARE_PROTECTED_MEMBER(data::TimeSeries, Data);

protected:
  /// @brief The host to connect to
  DECLARE_PROTECTED_MEMBER(std::string, Host);

  /// @brief The port to communicate the commands
  DECLARE_PROTECTED_MEMBER(int, CommandPort);

  /// @brief The port to communicate the responses
  DECLARE_PROTECTED_MEMBER(int, ResponsePort);

  /// @brief If the client is connected to the server
  DECLARE_PROTECTED_MEMBER(bool, IsConnected);

  /// @brief The Send a command to the server and wait for the confirmation
  /// @param command The command to send
  TcpServerResponse sendCommand(TcpServerCommand command);

  /// @brief Wait for a response from the server (invoked by [sendCommand])
  TcpServerResponse waitForResponse();

  /// @brief Construct a command packet to send to the server
  /// @param command The command to send
  /// @return The corresponding packet
  std::array<char, 8> constructCommandPacket(TcpServerCommand command);

  /// @brief Parse a response packet from the server
  /// @param buffer The buffer to parse
  /// @return The response from the server
  TcpServerResponse parseResponsePacket(const std::array<char, 8> &buffer);

private:
  /// @brief The asio context used for async methods of the client
  DECLARE_PRIVATE_MEMBER_NOGET(asio::io_context, Context);

  /// @brief The socket that is connected to the server for commands
  DECLARE_PRIVATE_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                               CommandSocket);

  /// @brief The socket that is connected to the server for responses
  DECLARE_PRIVATE_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                               ResponseSocket);

  /// @brief The protocol version of the communication with the server
  DECLARE_PRIVATE_MEMBER_NOGET(std::uint32_t, ProtocolVersion)
};

} // namespace STIMWALKER_NAMESPACE::server

#endif // __STIMWALKER_SERVER_TCP_SERVER_H__