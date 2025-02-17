#ifndef __NEUROBIO_SERVER_TCP_SERVER_H__
#define __NEUROBIO_SERVER_TCP_SERVER_H__

#include "neurobioConfig.h"

#include "Analyzer/Analyzers.h"
#include "Devices/Devices.h"
#include "Utils/CppMacros.h"
#include <asio.hpp>
#include <mutex>

namespace NEUROBIO_NAMESPACE::server {

enum TcpServerStatus { INITIALIZING, CONNECTING, CONNECTED };

enum class TcpServerCommand : std::uint32_t {
  HANDSHAKE = 0,
  CONNECT_DELSYS_ANALOG = 10,
  CONNECT_DELSYS_EMG = 11,
  CONNECT_MAGSTIM = 12,
  ZERO_DELSYS_ANALOG = 40,
  ZERO_DELSYS_EMG = 41,
  DISCONNECT_DELSYS_ANALOG = 20,
  DISCONNECT_DELSYS_EMG = 21,
  DISCONNECT_MAGSTIM = 22,
  START_RECORDING = 30,
  STOP_RECORDING = 31,
  GET_LAST_TRIAL_DATA = 32,
  ADD_ANALYZER = 50,
  REMOVE_ANALYZER = 51,
  FAILED = 100,
};

enum class TcpServerResponse : std::uint32_t {
  NOK = 0,
  OK = 1,
};

class TcpServer {
public:
  /// @brief Constructor
  /// @param commandPort The port to communicate the commands (default is 5000)
  /// @param responsePort The port to communicate the response (default is 5001)
  /// @param liveDataPort The port to communicate the live data (default is
  /// 5002)
  /// @param liveAnalysesPort The port to communicate the live analyses (default
  /// is 5003)
  TcpServer(int commandPort = 5000, int responsePort = 5001,
            int liveDataPort = 5002, int liveAnalysesPort = 5003);

  /// @brief Destructor
  ~TcpServer();
  TcpServer(const TcpServer &) = delete;

  // ----------------------- //
  // --- DEVICES METHODS --- //
  // ----------------------- //
public:
  /// @brief Start the server. This method is non-blocking. The server will run
  /// in a separate thread
  void startServer();

  /// @brief Start the server. This method is blocking. The server will run in
  /// the current thread
  void startServerSync();

  /// @brief Stop the server. This sends a message to the server that it should
  /// stop. After this command, [startServer] will return
  void stopServer();

  /// @brief Disconnect the current client
  void disconnectClient();

  /// @brief If a client is connected
  bool isClientConnected() const;

protected:
  DECLARE_PROTECTED_MEMBER_NOGET(bool, IsClientConnecting);

  /// @brief Wait for a new connexion
  /// @return True if everything is okay, False if connexion failed
  bool waitForNewConnexion();

  /// @brief Wait until the socket is connected
  /// @param socketName The name of the socket to wait for to be connected
  /// @param socket The socket to wait for to be connected
  /// @param acceptor The acceptor to listen to
  /// @param canTimeout If the waiting can timeout
  /// @return True if the socket is connected, false otherwise
  bool
  waitUntilSocketIsConnected(const std::string &socketName,
                             std::unique_ptr<asio::ip::tcp::socket> &socket,
                             std::unique_ptr<asio::ip::tcp::acceptor> &acceptor,
                             bool canTimeout);

  /// @brief Cancel the new connexion
  void closeSockets();

  /// @brief Wait for a new command
  /// @return True if everything is okay, False if the server is shutting down
  void waitAndHandleNewCommand();

  /// @brief If the server is started
  DECLARE_PROTECTED_MEMBER(bool, IsServerRunning);

  /// @brief Get the devices that are currently connected
  DECLARE_PROTECTED_MEMBER(devices::Devices, Devices);

  /// @brief Get the data analyzers that are currently loaded
  DECLARE_PROTECTED_MEMBER(analyzer::Analyzers, Analyzers);

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

  /// @brief Handle extra information from a command
  /// @param error The error code to set if an error occurs
  /// @return The response to send by the client (raises an exception if an
  /// error occurs)
  nlohmann::json handleCommandExtraData(asio::error_code &error);

  /// @brief Construct the packet to send to the client from a response
  /// @param response The response to send
  /// @return The packet to send
  std::array<char, 16> constructResponsePacket(TcpServerResponse response);

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

  /// @brief The port to listen to communicate the responses
  DECLARE_PROTECTED_MEMBER(int, ResponsePort);

  /// @brief The port to listen to communicate the live data
  DECLARE_PROTECTED_MEMBER(int, LiveDataPort);

  /// @brief The port to listen to communicate the live analyses
  DECLARE_PROTECTED_MEMBER(int, LiveAnalysesPort);

  /// @brief The timeout period for the server
  DECLARE_PROTECTED_MEMBER(std::chrono::milliseconds, TimeoutPeriod);

  /// @brief The socket that is connected to the client for commands
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                                 CommandSocket);

  /// @brief The socket that is connected to the client for response
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                                 ResponseSocket);

  /// @brief The socket that is connected to the client for live data streaming
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                                 LiveDataSocket);

  /// @brief The socket that is connected to the client for live analyses
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::socket>,
                                 LiveAnalysesSocket);

  /// @brief The acceptor that listens to the command port
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::acceptor>,
                                 CommandAcceptor);

  /// @brief The acceptor that listens to the response port
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::acceptor>,
                                 ResponseAcceptor);

  /// @brief The acceptor that listens to the live data streaming port
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::acceptor>,
                                 LiveDataAcceptor);

  /// @brief The acceptor that listens to the live analyses port
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::acceptor>,
                                 LiveAnalysesAcceptor);

  /// @brief The current status of the server
  DECLARE_PROTECTED_MEMBER(TcpServerStatus, Status);

  /// @brief Add the requested device
  /// @param deviceName The name of the device to add
  /// @return True if the device is added, false otherwise
  bool addDevice(const std::string &deviceName);

  /// @brief Set the zero level of the requested device
  /// @param deviceName The name of the device to set the zero level
  /// @return True if the zero level is set, false otherwise
  bool setZeroLevel(const std::string &deviceName);

  /// @brief Make the device and add it to the devices. This method is called by
  /// [addDevice] when it is confirmed that it is possible to actually add the
  /// device (no check is expected to be done in this method). This is a virtual
  /// method that can be overridden by the mocker to simulate the addition of a
  /// device
  /// @param deviceName The name of the device to add
  virtual void makeAndAddDevice(const std::string &deviceName);

  /// @brief Remove the requested device
  /// @param deviceName The name of the device to remove
  /// @param restartStreaming If the data streaming should be restarted after
  /// removing the device
  /// @return True if the device is removed, false otherwise
  bool removeDevice(const std::string &deviceName,
                    bool restartStreaming = true);

  /// @brief Handle the sending of the live data to the client
  void handleSendLiveData();

  /// @brief Handle the analysis of the live data
  void handleSendAnalyzedLiveData();

private:
  /// @brief The asio contexts used for async methods of the server
  DECLARE_PRIVATE_MEMBER_NOGET(asio::io_context, Context);

  /// @brief The worker thread for the [startServerAsync] method
  DECLARE_PRIVATE_MEMBER_NOGET(std::thread, ServerWorker);

  /// @brief The mutex to lock certain operations
  DECLARE_PRIVATE_MEMBER_NOGET(std::mutex, Mutex);

  /// @brief The current protocol version. This must match the client's version
  DECLARE_PRIVATE_MEMBER_NOGET(std::uint32_t, ProtocolVersion)
};

class TcpServerMock : public TcpServer {

public:
  /// @brief Constructor
  /// @param commandPort The port to communicate the commands (default is 5000)
  /// @param responsePort The port to communicate the response (default is 5001)
  /// @param liveDataPort The port to communicate the live data (default is
  /// 5002)
  /// @param timeoutPeriod The timeout period for the server (default is 5000
  /// ms)
  TcpServerMock(
      int commandPort = 5000, int responsePort = 5001, int liveDataPort = 5002,
      std::chrono::milliseconds timeoutPeriod = std::chrono::milliseconds(5000))
      : TcpServer(commandPort, responsePort, liveDataPort) {
    m_TimeoutPeriod = timeoutPeriod;
  };

  /// @brief Set the timeout period for the server
  /// @param timeoutPeriod The timeout period for the server
  void setTimeoutPeriod(std::chrono::milliseconds timeoutPeriod) {
    m_TimeoutPeriod = timeoutPeriod;
  }

  /// @brief Destructor
  ~TcpServerMock() = default;
  TcpServerMock(const TcpServerMock &) = delete;

protected:
  void makeAndAddDevice(const std::string &deviceName) override;
};

} // namespace NEUROBIO_NAMESPACE::server

#endif // __NEUROBIO_SERVER_TCP_SERVER_H__