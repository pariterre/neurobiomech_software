#ifndef __NEUROBIO_SERVER_TCP_CLIENT_H__
#define __NEUROBIO_SERVER_TCP_CLIENT_H__

#include "neurobioConfig.h"

#include "Data/TimeSeries.h"
#include "Server/TcpServer.h"
#include "Utils/CppMacros.h"
#include <asio.hpp>

namespace NEUROBIO_NAMESPACE::server {

class TcpClient {
public:
  /// @brief Constructor
  /// @param host The host to connect to
  /// @param commandPort The port to communicate the commands (default is 5000)
  /// @param responsePort The port to communicate the resonses (default is 5001)
  /// @param liveDataPort The port to communicate the live data (default is
  /// 5002)
  /// @param liveAnalysesPort The port to communicate the live analyses (default
  /// is 5003)
  TcpClient(std::string host = "localhost", int commandPort = 5000,
            int responsePort = 5001, int liveDataPort = 5002,
            int liveAnalysesPort = 5003);

  /// @brief Destructor
  ~TcpClient();
  TcpClient(const TcpClient &) = delete;

  /// @brief Connect to the server
  /// @return True if the connection is successful, false otherwise
  bool connect();

  /// @brief Disconnect from the server
  /// @return True if the disconnection is successful, false otherwise
  bool disconnect();

  /// @brief Add the Delsys Analog device
  /// @return True if the device is added, false otherwise
  bool addDelsysAnalogDevice();

  /// @brief Add the Delsys EMG device
  /// @return True if the device is added, false otherwise
  bool addDelsysEmgDevice();

  /// @brief Add the Magstim device
  /// @return True if the device is added, false otherwise
  bool addMagstimDevice();

  /// @brief Remove the Delsys Analog device
  /// @return True if the device is removed, false otherwise
  bool removeDelsysAnalogDevice();

  /// @brief Remove the Delsys EMG device
  /// @return True if the device is removed, false otherwise
  bool removeDelsysEmgDevice();

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

  /// @brief The port to communicate the live data
  DECLARE_PROTECTED_MEMBER(int, LiveDataPort);

  /// @brief The port to communicate the live analyses
  DECLARE_PROTECTED_MEMBER(int, LiveAnalysesPort);

  /// @brief If the client is connected to the server
  DECLARE_PROTECTED_MEMBER(bool, IsConnected);

  /// @brief Main loop for the live data streaming
  void startUpdatingLiveData();

  /// @brief Receive and update the live data
  void updateLiveData();

  /// @brief Main loop for the live analyses streaming
  void startUpdatingLiveAnalyses();

  /// @brief Add an analyzer to the collection
  void addAnalyzer(const nlohmann::json &analyzer);

  /// @brief Remove an analyzer from the collection
  void removeAnalyzer(const std::string &analyzerName);

  /// @brief Receive and update the live analyses
  void updateLiveAnalyses();

  /// @brief The Send a command to the server and wait for the confirmation
  /// @param command The command to send
  /// @return The acknowledgment from the server
  TcpServerResponse sendCommand(TcpServerCommand command);

  /// @brief The Send a command to the server and wait for the confirmation
  /// @param command The command to send
  /// @param data The data to send with the command
  /// @return The acknowledgment from the server
  TcpServerResponse sendCommandWithData(TcpServerCommand command,
                                        const nlohmann::json &data);

  /// @brief The Send a command to the server and wait for the confirmation
  /// @param command The command to send
  /// @return The response from the server
  std::vector<char> sendCommandWithResponse(TcpServerCommand command);

  /// @brief Wait for acknowledgment from the server (invoked by [sendCommand])
  /// @return The acknowledgment from the server
  TcpServerResponse waitForCommandAcknowledgment();

  /// @brief Wait for a response from the server
  /// @param socket The socket to wait for the response
  /// @return The response from the server
  std::vector<char> waitForResponse(asio::ip::tcp::socket &socket);

  /// @brief Close the sockets
  void closeSockets();

  /// @brief Construct a command packet to send to the server
  /// @param command The command to send
  /// @return The corresponding packet
  std::array<char, 8> constructCommandPacket(TcpServerCommand command);

  /// @brief Parse a response packet from the server
  /// @param buffer The buffer to parse
  /// @return The response from the server
  TcpServerResponse
  parseAcknowledgmentFromPacket(const std::array<char, 16> &buffer);

  /// @brief Parse a response packet from the server
  /// @param buffer The buffer to parse
  /// @return The response from the server
  std::chrono::system_clock::time_point
  parseTimeStampFromPacket(const std::array<char, 16> &buffer);

private:
  /// @brief The asio context used for async methods of the client
  DECLARE_PRIVATE_MEMBER_NOGET(asio::io_context, Context);

  /// @brief The socket that is connected to the server for commands
  DECLARE_PRIVATE_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                               CommandSocket);

  /// @brief The socket that is connected to the server for responses
  DECLARE_PRIVATE_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                               ResponseSocket);

  /// @brief The socket that is connected to the server for live data
  DECLARE_PRIVATE_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                               LiveDataSocket);

  /// @brief The worker thread for the live data streaming
  DECLARE_PRIVATE_MEMBER_NOGET(std::thread, LiveDataWorker);

  /// @brief The socket that is connected to the server for live analyses
  DECLARE_PRIVATE_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                               LiveAnalysesSocket);

  /// @brief The worker thread for the live analyses streaming
  DECLARE_PRIVATE_MEMBER_NOGET(std::thread, LiveAnalysesWorker);

  /// @brief The protocol version of the communication with the server
  DECLARE_PRIVATE_MEMBER_NOGET(std::uint32_t, ProtocolVersion)
};

} // namespace NEUROBIO_NAMESPACE::server

#endif // __NEUROBIO_SERVER_TCP_SERVER_H__