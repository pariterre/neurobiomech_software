#ifndef __NEUROBIO_SERVER_TCP_SERVER_H__
#define __NEUROBIO_SERVER_TCP_SERVER_H__

#include "neurobioConfig.h"

#include "Analyzer/Analyzers.h"
#include "Devices/Devices.h"
#include "Utils/CppMacros.h"
#include <asio.hpp>
#include <shared_mutex>

namespace NEUROBIO_NAMESPACE::server {

enum TcpServerStatus { OFF, PREPARING, READY };

static const uint32_t COMMUNICATION_PROTOCOL_VERSION = 2;
static const size_t BYTES_IN_CLIENT_PACKET_HEADER = 8;
static const size_t BYTES_IN_SERVER_PACKET_HEADER = 24;

enum class TcpServerCommand : uint32_t {
  HANDSHAKE = 0,
  GET_STATES = 1,
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
  NONE = 0xFFFFFFFF,
};

enum class TcpServerMessage : uint32_t {
  OK = 0,
  NOK = 1,
  LISTENING_EXTRA_DATA = 2,
  SENDING_DATA = 10,
  STATES_CHANGED = 20,
};

enum class TcpServerDataType : uint32_t {
  STATES = 0,
  FULL_TRIAL = 1,
  LIVE_DATA = 10,
  LIVE_ANALYSES = 11,
  NONE = 0xFFFFFFFF,
};

class ClientSession {
public:
  ClientSession(
      std::shared_ptr<asio::io_context> context, uint32_t id,
      std::function<bool(TcpServerCommand command, const ClientSession &client)>
          handleHandshake,
      std::function<bool(TcpServerCommand command, const ClientSession &client)>
          handleCommand,
      std::function<void(const ClientSession &client)> onDisconnect,
      std::chrono::milliseconds timeoutPeriod);

  ~ClientSession();

protected:
  /// @brief The timer used to send the live data to the client
  DECLARE_PROTECTED_MEMBER_NOGET(std::shared_ptr<asio::steady_timer>,
                                 ConnexionTimer);

  /// @brief The timeout period for the server
  DECLARE_PROTECTED_MEMBER(std::chrono::milliseconds, TimeoutPeriod);

public:
  /// @brief Connect the command socket to the given socket
  /// @param socket The socket to connect the command socket to
  void connectCommandSocket(std::shared_ptr<asio::ip::tcp::socket> socket);

  /// @brief Connect the message socket to the given socket
  /// @param socket The socket to connect the message socket to
  void connectMessageSocket(std::shared_ptr<asio::ip::tcp::socket> socket);

  /// @brief Connect the live data socket to the given socket
  /// @param socket The socket to connect the live data socket to
  void connectLiveDataSocket(std::shared_ptr<asio::ip::tcp::socket> socket);

  /// @brief Connect the live analyses socket to the given socket
  /// @param socket The socket to connect the live analyses socket to
  void connectLiveAnalysesSocket(std::shared_ptr<asio::ip::tcp::socket> socket);

  /// @brief Returns if the session is connected
  /// @return True if the session is connected, false otherwise
  bool isConnected() const;

  /// @brief Disconnects the session
  void disconnect();

protected:
  /// @brief The asio contexts used for async methods of the server
  DECLARE_PROTECTED_MEMBER_NOGET(std::shared_ptr<asio::io_context>, Context);

  /// @brief The sessions id of the client
  DECLARE_PROTECTED_MEMBER(uint32_t, Id);

  /// @brief Whether the handshake has been completed
  DECLARE_PROTECTED_MEMBER(bool, IsHandshakeDone);

  /// @brief The internal variable to prevent from disconnecting more
  /// than once
  DECLARE_PRIVATE_MEMBER_NOGET(bool, HasDisconnected);

  /// @brief The command socket used to communicate with the client
  DECLARE_PROTECTED_MEMBER(std::shared_ptr<asio::ip::tcp::socket>,
                           CommandSocket);
  /// @brief The message socket used to communicate with the client
  DECLARE_PROTECTED_MEMBER(std::shared_ptr<asio::ip::tcp::socket>,
                           MessageSocket);
  /// @brief The live data socket used to communicate with the client
  DECLARE_PROTECTED_MEMBER(std::shared_ptr<asio::ip::tcp::socket>,
                           LiveDataSocket);
  /// @brief The live analyses socket used to communicate with the client
  DECLARE_PROTECTED_MEMBER(std::shared_ptr<asio::ip::tcp::socket>,
                           LiveAnalysesSocket);

  /// @brief The function to call when a handshake is received
  /// @param command The command received
  /// @param id The session ID
  DECLARE_PROTECTED_MEMBER_NOGET(
      std::function<bool(TcpServerCommand command,
                         const ClientSession &client)>,
      HandleHandshake);

  /// @brief The function to call when a command is received
  /// @param command The command received
  /// @param id The session ID
  DECLARE_PROTECTED_MEMBER_NOGET(
      std::function<bool(TcpServerCommand command,
                         const ClientSession &client)>,
      HandleCommand);

  /// @brief The callback to call when the client disconnects
  DECLARE_PROTECTED_MEMBER_NOGET(
      std::function<void(const ClientSession &client)>, OnDisconnect);

  /// @brief Start the timer for the timeout
  void startTimerForTimeout();

  /// @brief Try to start the listening session. It only succeed if all the
  /// sockets are connected
  void tryStartSessionLoop();

  /// @brief The session loop that handles the command socket
  void commandSocketLoop();
};

class TcpServer {
public:
  /// @brief Constructor
  /// @param commandPort The port to communicate the commands (default is 5000)
  /// @param messagePort The port to communicate the messages (default is 5001)
  /// @param liveDataPort The port to communicate the live data (default is
  /// 5002)
  /// @param liveAnalysesPort The port to communicate the live analyses (default
  /// is 5003)
  TcpServer(int commandPort = 5000, int messagePort = 5001,
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

  /// @brief Stop the server. This sends a message to the server that it
  /// should stop. After this command, [startServer] will return
  void stopServer();

public:
  /// @brief Check if a session is connected
  /// @param id The id of the session to check
  bool isClientConnected(const uint32_t &id) const;

protected:
  /// @brief Start accepting connexions on all the ports
  void startAcceptors();

  /// @brief Stop accepting connexions
  void cancelAcceptors();

  /// @brief Get or create a session for the given id
  /// @param id The id of the session to get or create
  /// @return The session for the given id
  std::shared_ptr<ClientSession> getOrCreateSession(uint32_t id);

  /// @brief Read the session id from the socket
  /// @param socket The socket to read the session id from
  /// @return The session id read from the socket
  uint32_t
  readSessionIdFromSocket(std::shared_ptr<asio::ip::tcp::socket> socket);

  /// @brief The mutex used to protect the sessions
  DECLARE_PROTECTED_MEMBER_NOGET(std::shared_mutex, SessionMutex);

  /// @brief The sessions that are currently connected to the server
  std::unordered_map<uint32_t, std::shared_ptr<ClientSession>> m_Sessions;

  /// @brief Start accepting connexions on all the ports
  void startAcceptingSocketConnexions();

  /// @brief Accept a new socket connexion (generic)
  void acceptSocketConnexion(
      std::unique_ptr<asio::ip::tcp::acceptor> &acceptor,
      void (TcpServer::*handler)(std::shared_ptr<asio::ip::tcp::socket>));

  /// @brief Handle a command socket connexion
  /// @param socket The socket that has answered the connexion
  void
  handleCommandSocketConnexion(std::shared_ptr<asio::ip::tcp::socket> socket);

  /// @brief Handle a message socket connexion
  /// @param socket The socket that has answered the connexion
  void
  handleMessageSocketConnexion(std::shared_ptr<asio::ip::tcp::socket> socket);

  /// @brief Handle a live data socket connexion
  /// @param socket The socket that has answered the connexion
  void handleLiveDataSocket(std::shared_ptr<asio::ip::tcp::socket> socket);

  /// @brief Handle a live analyses socket connexion
  /// @param socket The socket that has answered the connexion
  void handleLiveAnalysesSocket(std::shared_ptr<asio::ip::tcp::socket> socket);

  /// @brief Handle a client that has disconnected
  /// @param session The client session that has disconnected
  void handleClientHasDisconnected(const ClientSession &session);

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
  /// @param session The client session that sent the command
  /// @return True if the handshake is successful, false otherwise
  bool handleHandshake(TcpServerCommand command, const ClientSession &session);

  /// @brief Handle a command
  /// @param command The command to handle
  /// @param session The client session that sent the command
  /// @return True if the command is successful, false otherwise
  bool handleCommand(TcpServerCommand command, const ClientSession &session);

  /// @brief Send clients that the internal states has changed
  /// @param command The command that triggered the state change
  void notifyClientsOfStateChange(TcpServerCommand command);

  /// @brief Handle extra information from a command
  /// @param command The command that sent the extra data
  /// @param error The error code to set if an error occurs
  /// @param session The client session that sent the command
  /// @return The response to send by the client (raises an exception if an
  /// error occurs)
  nlohmann::json handleExtraData(TcpServerCommand command,
                                 asio::error_code &error,
                                 const ClientSession &session);

  // -------------------------- //
  // --- TCP SERVER METHODS --- //
  // -------------------------- //
protected:
  /// @brief The port to listen to communicate the commands
  DECLARE_PROTECTED_MEMBER(int, CommandPort);

  /// @brief The port to listen to communicate the messages
  DECLARE_PROTECTED_MEMBER(int, MessagePort);

  /// @brief The port to listen to communicate the live data
  DECLARE_PROTECTED_MEMBER(int, LiveDataPort);

  /// @brief The port to listen to communicate the live analyses
  DECLARE_PROTECTED_MEMBER(int, LiveAnalysesPort);

  /// @brief The timeout period for the server
  DECLARE_PROTECTED_MEMBER(std::chrono::milliseconds, TimeoutPeriod);

  /// @brief The acceptor that listens to the command port
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::acceptor>,
                                 CommandAcceptor);

  /// @brief The acceptor that listens to the message port
  DECLARE_PROTECTED_MEMBER_NOGET(std::unique_ptr<asio::ip::tcp::acceptor>,
                                 MessageAcceptor);

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

  /// @brief The timer used to send the live data to the client
  DECLARE_PROTECTED_MEMBER_NOGET(std::shared_ptr<asio::steady_timer>,
                                 LiveDataTimer);

  /// @brief The session loop that handles the live data socket
  void liveDataLoop();

  /// @brief The timer used to send the live data to the client
  DECLARE_PROTECTED_MEMBER_NOGET(std::shared_ptr<asio::steady_timer>,
                                 LiveAnalysesTimer);

  /// @brief Handle the analysis of the live data
  void liveAnalysesLoop();

private:
  /// @brief The asio contexts used for async methods of the server
  DECLARE_PRIVATE_MEMBER_NOGET(std::shared_ptr<asio::io_context>, Context);

  /// @brief The asio contexts used for live data streaming
  DECLARE_PRIVATE_MEMBER_NOGET(std::shared_ptr<asio::io_context>,
                               LiveDataContext);

  /// @brief The asio contexts used for live analyses streaming
  DECLARE_PRIVATE_MEMBER_NOGET(std::shared_ptr<asio::io_context>,
                               LiveAnalysesContext);

  /// @brief The worker thread for the [startServerAsync] method
  DECLARE_PRIVATE_MEMBER_NOGET(std::thread, ServerWorker);
};

class TcpServerMock : public TcpServer {

public:
  /// @brief Constructor
  /// @param commandPort The port to communicate the commands (default is 5000)
  /// @param messagePort The port to communicate the messages (default is 5001)
  /// @param liveDataPort The port to communicate the live data (default is
  /// 5002)
  /// @param liveAnalysesPort The port to communicate the live analyses (default
  /// is 5003)
  /// @param timeoutPeriod The timeout period for the server (default is 5000
  /// ms)
  TcpServerMock(
      int commandPort = 5000, int messagePort = 5001, int liveDataPort = 5002,
      int liveAnalysesPort = 5003,
      std::chrono::milliseconds timeoutPeriod = std::chrono::milliseconds(5000))
      : TcpServer(commandPort, messagePort, liveDataPort, liveAnalysesPort) {
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