#ifndef __NEUROBIO_SERVER_TCP_CLIENT_H__
#define __NEUROBIO_SERVER_TCP_CLIENT_H__

#include "neurobioConfig.h"

#include "Data/TimeSeries.h"
#include "Server/TcpServer.h"
#include "Utils/CppMacros.h"
#include <asio.hpp>

namespace NEUROBIO_NAMESPACE::server {

class ServerResponse {
public:
  ServerResponse();
  ServerResponse(asio::ip::tcp::socket &socket, std::shared_mutex &mutex);
  virtual ~ServerResponse() = default;

protected:
  DECLARE_PROTECTED_MEMBER(bool, HasReceivedData);
  DECLARE_PROTECTED_MEMBER(TcpServerCommand, Command);
  DECLARE_PROTECTED_MEMBER(TcpServerMessage, Message);
  DECLARE_PROTECTED_MEMBER(TcpServerDataType, DataType);
  DECLARE_PROTECTED_MEMBER(std::chrono::system_clock::time_point, Timestamp);
  DECLARE_PROTECTED_MEMBER(std::vector<char>, Data);

protected:
  /// @brief Check the version from the packet
  /// @param buffer The buffer to check
  /// @return True if the version is correct, false otherwise
  bool checkVersionFromPacket(const std::vector<char> &buffer);

  /// @brief Parse the command the server is responding to from the packet
  /// @param buffer The buffer to parse
  /// @return The command the server is responding to
  TcpServerCommand parseCommandFromPacket(const std::vector<char> &buffer);

  /// @brief Parse a response packet from the server
  /// @param buffer The buffer to parse
  /// @return The response from the server
  TcpServerMessage parseMessageFromPacket(const std::vector<char> &buffer);

  /// @brief Parse the data type from the packet
  /// @param buffer The buffer to parse
  TcpServerDataType parseDataTypeFromPacket(const std::vector<char> &buffer);

  /// @brief Parse a response packet from the server
  /// @param buffer The buffer to parse
  /// @return The response from the server
  std::chrono::system_clock::time_point
  parseTimeStampFromPacket(const std::vector<char> &buffer);

  /// @brief Read the header from the socket
  /// @param socket The socket to read from
  /// @return The header read from the socket
  std::vector<char> readHeaderFromSocket(asio::ip::tcp::socket &socket);

  /// @brief Read and fill the data from the socket
  /// @param socket The socket to read from
  /// @param type The data type to read
  /// @return The data read from the socket
  std::vector<char> readDataFromSocket(asio::ip::tcp::socket &socket,
                                       TcpServerDataType type);
};

class TcpClient {
public:
  /// @brief Constructor
  /// @param host The host to connect to
  /// @param commandPort The port to communicate the commands (default is 5000)
  /// @param messagePort The port to communicate the messages (default is 5001)
  /// @param liveDataPort The port to communicate the live data (default is
  /// 5002)
  /// @param liveAnalysesPort The port to communicate the live analyses (default
  /// is 5003)
  TcpClient(std::string host = "localhost", int commandPort = 5000,
            int messagePort = 5001, int liveDataPort = 5002,
            int liveAnalysesPort = 5003);

  /// @brief Destructor
  ~TcpClient();
  TcpClient(const TcpClient &) = delete;

  /// @brief Connect to the server
  /// @param stateId The state ID to connect. This can be a random number
  /// between 0x10000000 and 0xFFFFFFFF, but it cannot be the same as another
  /// client (the server will reject the connection if the state ID is already
  /// in use)
  /// @return True if the connection is successful, false otherwise
  bool connect(uint32_t stateId);

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

  /// @brief Add an analyzer to the collection
  bool addAnalyzer(const nlohmann::json &analyzer);

  /// @brief Remove an analyzer from the collection
  bool removeAnalyzer(const std::string &analyzerName);

protected:
  /// @brief The data received from the server
  DECLARE_PROTECTED_MEMBER(data::TimeSeries, Data);

protected:
  /// @brief The host to connect to
  DECLARE_PROTECTED_MEMBER(std::string, Host);

  /// @brief The port to communicate the commands
  DECLARE_PROTECTED_MEMBER(int, CommandPort);

  /// @brief The port to communicate the messages
  DECLARE_PROTECTED_MEMBER(int, MessagePort);

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

  /// @brief Receive and update the live analyses
  void updateLiveAnalyses();

  /// @brief The Send a command to the server and wait for the confirmation
  /// @param command The command to send
  /// @return The acknowledgment from the server
  ServerResponse sendCommand(TcpServerCommand command);

  /// @brief The Send a command to the server and wait for the confirmation
  /// @param command The command to send
  /// @param data The data to send with the command
  /// @return The acknowledgment from the server
  ServerResponse sendCommandWithData(TcpServerCommand command,
                                     const nlohmann::json &data);
  /// @brief The Send a command to the server and wait for the confirmation
  /// @param command The command to send
  /// @param data The data to send with the command
  /// @return The acknowledgment from the server
  ServerResponse readDataSizeFromSocket(TcpServerCommand command,
                                        const nlohmann::json &data);

  /// @brief The Send a command to the server and wait for the confirmation
  /// @param command The command to send
  /// @return The response from the server
  std::vector<char> sendCommandWithResponse(TcpServerCommand command);

  /// @brief The last message received from the server
  DECLARE_PROTECTED_MEMBER_NOGET(ServerResponse, PreviousMessage);
  DECLARE_PROTECTED_MEMBER_NOGET(bool, HasPreviousMessage);

  /// @brief Close the sockets
  void closeSockets();

  /// @brief Construct a command packet to send to the server
  /// @param command The command to send
  /// @return The corresponding packet
  std::array<char, BYTES_IN_CLIENT_PACKET_HEADER>
  constructCommandPacket(TcpServerCommand command);

private:
  DECLARE_PROTECTED_MEMBER_NOGET(std::thread, ContextWorker);

  /// @brief The asio context used for async methods of the client
  DECLARE_PRIVATE_MEMBER_NOGET(asio::io_context, Context);

  /// @brief The socket that is connected to the server for commands
  DECLARE_PRIVATE_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                               CommandSocket);

  DECLARE_PROTECTED_MEMBER_NOGET(std::thread, MessageWorker);

  /// @brief The socket that is connected to the server for response messages
  DECLARE_PRIVATE_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                               MessageSocket);

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

private:
  DECLARE_PRIVATE_MEMBER_NOGET(std::shared_mutex, PreviousMessageMutex);
};

} // namespace NEUROBIO_NAMESPACE::server

#endif // __NEUROBIO_SERVER_TCP_SERVER_H__