#include "Server/TcpServer.h"

#include "Utils/Logger.h"
#include <asio/steady_timer.hpp>
#include <thread>

#include "Analyzer/Analyzer.h"
#include "Analyzer/Analyzers.h"
#include "Analyzer/Exceptions.h"
#include "Devices/Concrete/DelsysAnalogDevice.h"
#include "Devices/Concrete/DelsysEmgDevice.h"
#include "Devices/Concrete/MagstimRapidDevice.h"
#include "Devices/Generic/DelsysBaseDevice.h"
#include "Devices/Generic/Device.h"

#if defined(_WIN32) // Windows-specific byte swap functions
// Windows is little-endian by default
#define htole32(x) (x)
#define htole64(x) (x)
#define le32toh(x) (x)

#elif defined(__APPLE__) // macOS
#include <libkern/OSByteOrder.h>
#define htole32(x) OSSwapHostToLittleInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)
#define htole64(x) OSSwapHostToLittleInt64(x)

#elif defined(__linux__) // Linux
#include <endian.h>      // Provides htole32(), htole64() and le32toh()

#else
#error "Unsupported platform"
#endif

using namespace NEUROBIO_NAMESPACE::server;

TcpServerCommand parseCommandPacket(
    const std::array<char, BYTES_IN_CLIENT_PACKET_HEADER> &buffer) {
  // Packets are exactly 8 bytes long, litte endian
  // - First 4 bytes are the version number
  // - Next 4 bytes are the command

  // Check the version
  std::uint32_t version;
  // Safely copy memory and convert from little-endian
  std::memcpy(&version, buffer.data(), sizeof(version));
  version = le32toh(version); // Convert to native endianness

  if (version != COMMUNICATION_PROTOCOL_VERSION) {
    auto &logger = NEUROBIO_NAMESPACE::utils::Logger::getInstance();
    logger.fatal("Invalid version: " + std::to_string(version) +
                 ". Please "
                 "update the client to version " +
                 std::to_string(COMMUNICATION_PROTOCOL_VERSION));
    return TcpServerCommand::FAILED;
  }

  // Get the command
  std::uint32_t command;
  std::memcpy(&command, buffer.data() + sizeof(version), sizeof(command));
  command = le32toh(command); // Convert to native endianness

  return static_cast<TcpServerCommand>(command);
}

template <typename T>
std::vector<char>
constructResponsePacket(TcpServerCommand command, TcpServerMessage message,
                        TcpServerDataType dataType, uint64_t dataSize, T data) {
  // Packets are exactly 24 bytes long, litte endian if dataType is NONE,
  // otherwise a mandatory 8 bytes is added alongside the actual data
  // - First 4 bytes are the version number
  // - Next 4 bytes of the command it is responding to. If it does not respond
  // to any command, it is set to NONE (0xFFFFFFFF)
  // - Next 4 bytes are the message from the server
  // - Next 4 bytes are the data type (if any, otherwise 0xFFFFFFFF) that is
  // being sent to the client
  // - The Next 8 bytes are the timestamp of the message
  // - Next 8 bytes are skiped if dataType is NONE, otherwise they are the size
  // of the extra data that will be sent to the client. bytes of extra data that
  // will be sent to the client.
  // - The rest of the packet is the extra data, if any of exactly dataSize
  // bytes

  if (dataType == TcpServerDataType::NONE && dataSize > 0) {
    throw std::invalid_argument(
        "Cannot send data when dataType is NONE and dataSize is greater than "
        "0");
  }

  auto packet = dataType == TcpServerDataType::NONE
                    ? std::vector<char>(BYTES_IN_SERVER_PACKET_HEADER, '\0')
                    : std::vector<char>(
                          BYTES_IN_SERVER_PACKET_HEADER + 8 + dataSize, '\0');

  // Add the version number in uint32_t format (litte endian)
  std::uint32_t versionLittleEndian = htole32(COMMUNICATION_PROTOCOL_VERSION);
  std::memcpy(packet.data(), &versionLittleEndian, sizeof(versionLittleEndian));

  // Add the command in uint32_t format (litte endian)
  std::uint32_t commandLittleEndian =
      htole32(static_cast<std::uint32_t>(command));
  std::memcpy(packet.data() + sizeof(versionLittleEndian), &commandLittleEndian,
              sizeof(commandLittleEndian));

  // Add the response in uint32_t format (litte endian)
  std::uint32_t responseLittleEndian =
      htole32(static_cast<std::uint32_t>(message));
  std::memcpy(packet.data() + sizeof(versionLittleEndian) +
                  sizeof(commandLittleEndian),
              &responseLittleEndian, sizeof(responseLittleEndian));

  // Add the data type in uint32_t format (litte endian)
  std::uint32_t dataTypeLittleEndian =
      htole32(static_cast<std::uint32_t>(dataType));
  std::memcpy(packet.data() + sizeof(versionLittleEndian) +
                  sizeof(commandLittleEndian) + sizeof(responseLittleEndian),
              &dataTypeLittleEndian, sizeof(dataTypeLittleEndian));

  // Add the timestamps in uint64_t format (litte endian)
  std::uint64_t timestamp =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  std::uint64_t timestampLittleEndian = htole64(timestamp);
  std::memcpy(packet.data() + sizeof(versionLittleEndian) +
                  sizeof(commandLittleEndian) + sizeof(responseLittleEndian) +
                  sizeof(dataTypeLittleEndian),
              &timestampLittleEndian, sizeof(timestampLittleEndian));

  // Add the size of the extra data in uint64_t format (litte endian)
  if (dataType != TcpServerDataType::NONE) {

    std::uint64_t dataSizeLittleEndian = htole64(dataSize);
    std::memcpy(packet.data() + sizeof(versionLittleEndian) +
                    sizeof(commandLittleEndian) + sizeof(responseLittleEndian) +
                    sizeof(timestampLittleEndian) +
                    sizeof(dataTypeLittleEndian),
                &dataSizeLittleEndian, sizeof(dataSizeLittleEndian));

    // If there is extra data, add it to the packet
    if (dataSize > 0) {
      std::memcpy(
          packet.data() + sizeof(versionLittleEndian) +
              sizeof(commandLittleEndian) + sizeof(responseLittleEndian) +
              sizeof(timestampLittleEndian) + sizeof(dataTypeLittleEndian) +
              sizeof(dataSizeLittleEndian),
          data.data(), dataSize);
    }
  }

  return packet;
}

std::vector<char> constructResponsePacket(TcpServerCommand command,
                                          TcpServerMessage message) {
  return constructResponsePacket(command, message, TcpServerDataType::NONE, 0,
                                 std::vector<char>());
}

// Here are the names of the devices that can be connected (for internal use)
const std::string DEVICE_NAME_DELSYS_EMG = "DelsysEmgDevice";
const std::string DEVICE_NAME_DELSYS_ANALOG = "DelsysAnalogDevice";
const std::string DEVICE_NAME_MAGSTIM = "MagstimRapidDevice";

ClientSession::ClientSession(
    std::shared_ptr<asio::io_context> context, std::uint32_t id,
    std::function<bool(TcpServerCommand command, const ClientSession &client)>
        handleHandshake,
    std::function<bool(TcpServerCommand command, const ClientSession &client)>
        handleCommand,
    std::function<void(const ClientSession &client)> onDisconnect,
    std::chrono::milliseconds timeoutPeriod)
    : m_TimeoutPeriod(timeoutPeriod), m_Context(context), m_Id(id),
      m_IsHandshakeDone(false), m_HasDisconnected(false),
      m_HandleHandshake(handleHandshake), m_HandleCommand(handleCommand),
      m_OnDisconnect(onDisconnect) {}

ClientSession::~ClientSession() { disconnect(); }

void ClientSession::connectCommandSocket(
    std::shared_ptr<asio::ip::tcp::socket> socket) {
  if (m_HasDisconnected)
    return;
  m_CommandSocket = socket;
  tryStartSessionLoop();
}

void ClientSession::connectResponseSocket(
    std::shared_ptr<asio::ip::tcp::socket> socket) {
  if (m_HasDisconnected)
    return;
  m_ResponseSocket = socket;
  tryStartSessionLoop();
}

void ClientSession::connectLiveDataSocket(
    std::shared_ptr<asio::ip::tcp::socket> socket) {
  if (m_HasDisconnected)
    return;
  m_LiveDataSocket = socket;
  tryStartSessionLoop();
}

void ClientSession::connectLiveAnalysesSocket(
    std::shared_ptr<asio::ip::tcp::socket> socket) {
  if (m_HasDisconnected)
    return;
  m_LiveAnalysesSocket = socket;
  tryStartSessionLoop();
}

bool ClientSession::isConnected() const {
  return !m_HasDisconnected && m_CommandSocket && m_CommandSocket->is_open() &&
         m_ResponseSocket && m_ResponseSocket->is_open() && m_LiveDataSocket &&
         m_LiveDataSocket->is_open() && m_LiveAnalysesSocket &&
         m_LiveAnalysesSocket->is_open();
}

void ClientSession::disconnect() {
  if (m_HasDisconnected) {
    return; // Already disconnected
  }
  m_HasDisconnected = true;

  auto closeSocket = [](std::shared_ptr<asio::ip::tcp::socket> &s) {
    if (s && s->is_open()) {
      asio::error_code ec;
      s->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
      s->close(ec);
    }
  };

  m_IsHandshakeDone = false;
  closeSocket(m_CommandSocket);
  closeSocket(m_ResponseSocket);
  closeSocket(m_LiveDataSocket);
  closeSocket(m_LiveAnalysesSocket);

  utils::Logger::getInstance().info("Session " + std::to_string(m_Id) +
                                    " disconnected and cleaned up.");
  m_OnDisconnect(*this);
}

void ClientSession::startTimerForTimeout() {
  m_ConnexionTimer = std::make_shared<asio::steady_timer>(*m_Context);
  m_ConnexionTimer->expires_after(m_TimeoutPeriod);
  m_ConnexionTimer->async_wait([this](const asio::error_code &ec) {
    if (ec) {
      // Timer was cancelled or failed (meaning the connexion was
      // successful)
      return;
    }
    // If we reach here, the timer expired, meaning the client did not
    // connect all sockets in time, so we disconnect them
    auto &logger = utils::Logger::getInstance();
    logger.warning("Client session " + std::to_string(m_Id) +
                   " did not connect all sockets in time, disconnecting.");
    disconnect();
  });
}

void ClientSession::tryStartSessionLoop() {
  if (!isConnected()) {
    // Once a client tried to connect once, they have a fixed amount of time to
    // connect all sockets, otherwise we disconnect them
    if (!m_ConnexionTimer) {
      startTimerForTimeout();
    }
    return;
  }

  // All sockets connected — spawn logic
  m_ConnexionTimer.reset();
  auto &logger = utils::Logger::getInstance();
  logger.info("Starting client session with ID: " + std::to_string(m_Id));

  // Start reading from the sockets
  commandSocketLoop();
}

void ClientSession::commandSocketLoop() {
  if (m_HasDisconnected || !m_CommandSocket || !m_CommandSocket->is_open()) {
    // Terminate the loop if disconnected or socket is not open
    return;
  }

  auto buffer =
      std::make_shared<std::array<char, BYTES_IN_CLIENT_PACKET_HEADER>>();
  m_CommandSocket->async_read_some(
      asio::buffer(*buffer),
      [this, buffer](const asio::error_code &ec, size_t byteRead) {
        auto &logger = utils::Logger::getInstance();
        if (ec) {
          // If anything went wrong, disconnect the client
          disconnect();
          return;
        }

        // Parse the packet
        TcpServerCommand command = parseCommandPacket(*buffer);

        // Handle the command based on the current status
        if (!(m_IsHandshakeDone ? m_HandleCommand(command, *this)
                                : m_HandleHandshake(command, *this))) {
          // If anything went wrong, disconnect the client
          disconnect();
        }
        m_IsHandshakeDone = true;

        // If we get here, the command was successful and we can continue
        // listening for the next command
        commandSocketLoop();
      });
}

TcpServer::TcpServer(int commandPort, int responsePort, int liveDataPort,
                     int liveAnalysesPort)
    : m_CommandPort(commandPort), m_ResponsePort(responsePort),
      m_LiveDataPort(liveDataPort), m_LiveAnalysesPort(liveAnalysesPort),
      m_TimeoutPeriod(std::chrono::milliseconds(5000)),
      m_Status(TcpServerStatus::OFF),
      m_Context(std::make_shared<asio::io_context>()),
      m_LiveDataContext(std::make_shared<asio::io_context>()),
      m_LiveAnalysesContext(std::make_shared<asio::io_context>()) {
  m_LiveDataTimer = std::make_shared<asio::steady_timer>(
      *m_LiveDataContext, std::chrono::milliseconds(100));
  m_LiveAnalysesTimer = std::make_shared<asio::steady_timer>(
      *m_LiveAnalysesContext, std::chrono::milliseconds(25));
}

TcpServer::~TcpServer() { stopServer(); }

void TcpServer::startServer() {
  m_ServerWorker = std::thread([this]() { startServerSync(); });
  // Wait for the server to be ready
  while (m_Status != TcpServerStatus::READY) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return;
}

void TcpServer::startServerSync() {
  auto &logger = utils::Logger::getInstance();

  m_Status = TcpServerStatus::PREPARING;
  startAcceptors();
  startAcceptingSocketConnexions();
  m_Status = TcpServerStatus::READY;

  liveDataLoop();
  // Start the loop timers
  std::thread liveDataWorkerThread([this]() {
    m_LiveDataContext->run();
    utils::Logger::getInstance().info("Live data has terminated");
  });

  liveAnalysesLoop();
  std::thread liveAnalysesWorkerThread([this]() {
    m_LiveAnalysesContext->run();
    utils::Logger::getInstance().info("Live analyses has terminated");
  });

  m_Context->run();
  liveAnalysesWorkerThread.join();
  liveDataWorkerThread.join();
  logger.info("TCP Server is terminating");
}

void TcpServer::startAcceptingSocketConnexions() {
  acceptSocketConnexion(m_CommandAcceptor,
                        &TcpServer::handleCommandSocketConnexion);
  acceptSocketConnexion(m_ResponseAcceptor,
                        &TcpServer::handleResponseSocketConnexion);
  acceptSocketConnexion(m_LiveDataAcceptor, &TcpServer::handleLiveDataSocket);
  acceptSocketConnexion(m_LiveAnalysesAcceptor,
                        &TcpServer::handleLiveAnalysesSocket);
}

void TcpServer::acceptSocketConnexion(
    std::unique_ptr<asio::ip::tcp::acceptor> &acceptor,
    void (TcpServer::*handler)(std::shared_ptr<asio::ip::tcp::socket>)) {

  auto socket = std::make_shared<asio::ip::tcp::socket>(*m_Context);
  acceptor->async_accept(
      *socket, [this, socket, handler, &acceptor](const asio::error_code &ec) {
        if (!ec) {
          (this->*handler)(socket);
        }

        // Continue accepting
        acceptSocketConnexion(acceptor, handler);
      });
}

void TcpServer::handleCommandSocketConnexion(
    std::shared_ptr<asio::ip::tcp::socket> socket) {
  auto sessionId = readSessionIdFromSocket(socket);
  if (sessionId == 0xFFFFFFFF) {
    return;
  }

  auto session = getOrCreateSession(sessionId);
  session->connectCommandSocket(socket);
}

void TcpServer::handleResponseSocketConnexion(
    std::shared_ptr<asio::ip::tcp::socket> socket) {
  auto sessionId = readSessionIdFromSocket(socket);
  if (sessionId == 0xFFFFFFFF) {
    return;
  }

  auto session = getOrCreateSession(sessionId);
  session->connectResponseSocket(socket);
}

void TcpServer::handleLiveDataSocket(
    std::shared_ptr<asio::ip::tcp::socket> socket) {
  auto sessionId = readSessionIdFromSocket(socket);
  if (sessionId == 0xFFFFFFFF) {
    return;
  }

  auto session = getOrCreateSession(sessionId);
  session->connectLiveDataSocket(socket);
}

void TcpServer::handleLiveAnalysesSocket(
    std::shared_ptr<asio::ip::tcp::socket> socket) {
  auto sessionId = readSessionIdFromSocket(socket);
  if (sessionId == 0xFFFFFFFF) {
    return;
  }

  auto session = getOrCreateSession(sessionId);
  session->connectLiveAnalysesSocket(socket);
}

std::shared_ptr<ClientSession> TcpServer::getOrCreateSession(std::uint32_t id) {
  std::unique_lock lock(m_SessionMutex); // Exclusive lock
  auto &session = m_Sessions[id];
  if (!session) {
    session = std::make_shared<ClientSession>(
        m_Context, id,
        [this](TcpServerCommand command, const ClientSession &client) {
          return handleHandshake(command, client);
        },
        [this](TcpServerCommand command, const ClientSession &client) {
          return handleCommand(command, client);
        },
        [this](const ClientSession &client) {
          return handleClientHasDisconnected(client);
        },
        m_TimeoutPeriod);
  }
  return session;
}

std::uint32_t TcpServer::readSessionIdFromSocket(
    std::shared_ptr<asio::ip::tcp::socket> socket) {
  uint32_t id(0xFFFFFFFF); // Default invalid ID
  bool hasValue(false);

  // Start a timer so a user that would not send the session ID
  // will be disconnected after a while
  asio::steady_timer timer(*m_Context, m_TimeoutPeriod);
  timer.async_wait([&hasValue](const asio::error_code &ec) {
    if (ec)
      return;        // Timer was canceled
    hasValue = true; // Timeout occurred
  });

  auto buffer = std::array<char, BYTES_IN_CLIENT_PACKET_HEADER>();
  asio::async_read(
      *socket, asio::buffer(buffer),
      [this, &hasValue, &id, &timer, &buffer](const asio::error_code &ec,
                                              std::size_t byteRead) {
        if (ec || byteRead != BYTES_IN_CLIENT_PACKET_HEADER || hasValue) {
          // If anything went wrong, we will return an error value
          hasValue = true;
          return;
        }
        timer.cancel(); // Cancel the timer since we successfully read the ID

        id = static_cast<std::uint32_t>(parseCommandPacket(buffer));
        hasValue = true;
      });

  // Wait for the timer or the async read to finish
  while (!hasValue) {
    m_Context->run_one();
  }

  if (id < 0x10000000) {
    id = 0xFFFFFFFF; // Invalid ID, return error value
  }

  std::unique_lock lock(m_SessionMutex);
  auto &session = m_Sessions[id];
  if (session && session->isConnected()) {
    // If the session is already connected so the state is invalid
    auto &logger = utils::Logger::getInstance();
    logger.warning("Client with ID " + std::to_string(id) +
                   " is already connected, please choose a different ID.");
    id = 0xFFFFFFFF; // Return error value
  }

  if (id == 0xFFFFFFFF) {
    auto &logger = utils::Logger::getInstance();
    socket->close();
  }

  return id;
}

void TcpServer::handleClientHasDisconnected(const ClientSession &session) {
  // If the server is already off, no need to handle disconnection
  if (m_Status == TcpServerStatus::OFF) {
    return;
  }

  // Exclusive lock
  std::unique_lock lock(m_SessionMutex);
  auto it = m_Sessions.find(session.getId());
  if (it == m_Sessions.end())
    return;
  m_Sessions.erase(it);
}

void TcpServer::stopServer() {
  if (m_Status == TcpServerStatus::OFF) {
    return;
  }
  m_Status = TcpServerStatus::OFF;

  // Stop any running contexts
  m_Context->stop();
  m_LiveDataContext->stop();
  m_LiveAnalysesContext->stop();

  // Shutdown the server down
  auto &logger = utils::Logger::getInstance();
  logger.info("Stopping the server...");

  // Terminating the timers
  m_LiveDataTimer->cancel();
  m_LiveAnalysesTimer->cancel();

  // Make sure all the devices are properly disconnected
  logger.info("Disconnecting all devices");
  for (auto &name : m_Devices.getDeviceNames()) {
    removeDevice(name, false);
  }

  // Clear the analyzers
  logger.info("Clearing all the analyzers");
  m_Analyzers.clear();

  // Disconnect all the clients
  {
    logger.info("Disconnecting all the clients");
    std::unique_lock lock(m_SessionMutex);
    for (auto &sessionPair : m_Sessions) {
      auto &session = sessionPair.second;
      if (!session || !session->isConnected()) {
        continue; // Skip disconnected sessions
      }
      session->disconnect();
    }
    m_Sessions.clear();
  }

  // Cancel all the acceptors
  logger.info("Canceling all the acceptors");
  cancelAcceptors();

  // Wait for the server to stop
  if (m_ServerWorker.joinable()) {
    m_ServerWorker.join();
  }
  logger.info("Server has shut down");
}

bool TcpServer::isClientConnected(const std::uint32_t &id) const {
  return m_Sessions.find(id) != m_Sessions.end();
}

void TcpServer::startAcceptors() {
  auto &logger = utils::Logger::getInstance();

  // Create the contexts and acceptors
  m_CommandAcceptor = std::make_unique<asio::ip::tcp::acceptor>(
      *m_Context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_CommandPort));
  logger.info("TCP Command server started on port " +
              std::to_string(m_CommandPort));

  m_ResponseAcceptor = std::make_unique<asio::ip::tcp::acceptor>(
      *m_Context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_ResponsePort));
  logger.info("TCP Response server started on port " +
              std::to_string(m_ResponsePort));

  m_LiveDataAcceptor = std::make_unique<asio::ip::tcp::acceptor>(
      *m_Context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_LiveDataPort));
  logger.info("TCP Live Data server started on port " +
              std::to_string(m_LiveDataPort));

  m_LiveAnalysesAcceptor = std::make_unique<asio::ip::tcp::acceptor>(
      *m_Context,
      asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_LiveAnalysesPort));
  logger.info("TCP Live Analyses server started on port " +
              std::to_string(m_LiveAnalysesPort));
}

void TcpServer::cancelAcceptors() {
  if (m_CommandAcceptor && m_CommandAcceptor->is_open()) {
    m_CommandAcceptor->cancel();
  }

  if (m_ResponseAcceptor && m_ResponseAcceptor->is_open()) {
    m_ResponseAcceptor->cancel();
  }

  if (m_LiveDataAcceptor && m_LiveDataAcceptor->is_open()) {
    m_LiveDataAcceptor->cancel();
  }

  if (m_LiveAnalysesAcceptor && m_LiveAnalysesAcceptor->is_open()) {
    m_LiveAnalysesAcceptor->cancel();
  }
}

bool TcpServer::handleHandshake(TcpServerCommand command,
                                const ClientSession &session) {
  if (m_Status == TcpServerStatus::OFF) {
    // If the server is off, we cannot handle any commands
    return false;
  }

  auto &logger = utils::Logger::getInstance();
  asio::error_code error;

  // The only valid command during initialization is the handshake
  auto isAccepted = command == TcpServerCommand::HANDSHAKE;
  size_t byteWritten = asio::write(
      *session.getCommandSocket(),
      asio::buffer(constructResponsePacket(
          command, isAccepted ? TcpServerMessage::OK : TcpServerMessage::NOK)),
      error);
  if (!isAccepted || byteWritten != BYTES_IN_SERVER_PACKET_HEADER || error) {
    if (!isAccepted) {
      logger.fatal("Invalid command during initialization: " +
                   std::to_string(static_cast<std::uint32_t>(command)));
    } else {
      logger.fatal("TCP write error: " + error.message());
    }
    return false;
  }

  // Set the status to running
  logger.info("Handshake from client " + std::to_string(session.getId()) +
              " successful, server is now connected.");

  return true;
}

bool TcpServer::handleCommand(TcpServerCommand command,
                              const ClientSession &session) {
  if (m_Status == TcpServerStatus::OFF) {
    // If the server is off, we cannot handle any commands
    return false;
  }

  auto &logger = utils::Logger::getInstance();
  asio::error_code error;

  // Handle the command
  TcpServerMessage response = TcpServerMessage::OK;
  bool shouldNotifyClients = false;
  switch (command) {
  case TcpServerCommand::GET_STATES: {
    auto states = nlohmann::json();

    // Connected devices status
    auto connectedDevices = nlohmann::json();
    for (auto &id : m_Devices.getDeviceIds()) {
      const auto &device = m_Devices[id];
      auto value = nlohmann::json();
      value["is_connected"] = device.getIsConnected();
      value["is_collecting"] = m_Devices.hasDataCollector(id);
      value["is_recording"] = false;
      if (value["is_collecting"]) {
        value["is_recording"] = m_Devices.getDataCollector(id).getIsRecording();
      }
      connectedDevices[device.deviceName()] = value;
    }
    states["connected_devices"] = connectedDevices;

    // Connected analyzers status
    auto connectedAnalyzers = nlohmann::json();
    for (const auto &id : m_Analyzers.getAnalyzerIds()) {
      const auto &analyzer = m_Analyzers[id];
      auto value = nlohmann::json();
      value["configuration"] = analyzer.getSerializedConfiguration();
      connectedAnalyzers[analyzer.getName()] = value;
    }
    states["connected_analyzers"] = connectedAnalyzers;

    auto dump = states.dump();
    asio::write(*session.getResponseSocket(),
                asio::buffer(constructResponsePacket(
                    command, TcpServerMessage::SENDING_DATA,
                    TcpServerDataType::STATES, dump.size(), dump)),
                error);
    if (error) {
      logger.fatal("TCP write error: " + error.message());
      response = TcpServerMessage::NOK;
    }
    break;
  }
  case TcpServerCommand::CONNECT_DELSYS_ANALOG:
    response = addDevice(DEVICE_NAME_DELSYS_ANALOG) ? TcpServerMessage::OK
                                                    : TcpServerMessage::NOK;
    shouldNotifyClients = true;
    break;

  case TcpServerCommand::CONNECT_DELSYS_EMG:
    response = addDevice(DEVICE_NAME_DELSYS_EMG) ? TcpServerMessage::OK
                                                 : TcpServerMessage::NOK;
    shouldNotifyClients = true;
    break;

  case TcpServerCommand::CONNECT_MAGSTIM:
    response = addDevice(DEVICE_NAME_MAGSTIM) ? TcpServerMessage::OK
                                              : TcpServerMessage::NOK;
    shouldNotifyClients = true;
    break;

  case TcpServerCommand::ZERO_DELSYS_ANALOG:
    response = m_Devices.zeroLevelDevice(DEVICE_NAME_DELSYS_ANALOG)
                   ? TcpServerMessage::OK
                   : TcpServerMessage::NOK;
    shouldNotifyClients = true;
    break;

  case TcpServerCommand::ZERO_DELSYS_EMG:
    response = m_Devices.zeroLevelDevice(DEVICE_NAME_DELSYS_EMG)
                   ? TcpServerMessage::OK
                   : TcpServerMessage::NOK;
    shouldNotifyClients = true;
    break;

  case TcpServerCommand::DISCONNECT_DELSYS_ANALOG:
    response = removeDevice(DEVICE_NAME_DELSYS_ANALOG) ? TcpServerMessage::OK
                                                       : TcpServerMessage::NOK;
    shouldNotifyClients = true;
    break;

  case TcpServerCommand::DISCONNECT_DELSYS_EMG:
    response = removeDevice(DEVICE_NAME_DELSYS_EMG) ? TcpServerMessage::OK
                                                    : TcpServerMessage::NOK;
    shouldNotifyClients = true;
    break;

  case TcpServerCommand::DISCONNECT_MAGSTIM:
    response = removeDevice(DEVICE_NAME_MAGSTIM) ? TcpServerMessage::OK
                                                 : TcpServerMessage::NOK;
    shouldNotifyClients = true;
    break;

  case TcpServerCommand::START_RECORDING:
    response = m_Devices.startRecording() ? TcpServerMessage::OK
                                          : TcpServerMessage::NOK;
    shouldNotifyClients = true;
    break;

  case TcpServerCommand::STOP_RECORDING:
    response = m_Devices.stopRecording() ? TcpServerMessage::OK
                                         : TcpServerMessage::NOK;
    shouldNotifyClients = true;
    break;

  case TcpServerCommand::GET_LAST_TRIAL_DATA: {
    auto data = m_Devices.getLastTrialDataSerialized();

    auto dump = data.dump();
    asio::write(*session.getResponseSocket(),
                asio::buffer(constructResponsePacket(
                    command, TcpServerMessage::SENDING_DATA,
                    TcpServerDataType::FULL_TRIAL, dump.size(), dump)),
                error);
  } break;

  case TcpServerCommand::ADD_ANALYZER: {
    try {
      auto data = handleExtraData(command, error, session);
      m_Analyzers.add(data);
    } catch (const std::exception &e) {
      logger.fatal("Failed to get extra info: " + std::string(e.what()));
      response = TcpServerMessage::NOK;
    } catch (...) {
      logger.fatal("Failed to get extra info");
      response = TcpServerMessage::NOK;
    }
    shouldNotifyClients = true;
  } break;

  case TcpServerCommand::REMOVE_ANALYZER: {
    try {
      std::string analyzerName =
          handleExtraData(command, error, session)["analyzer"];
      m_Analyzers.remove(analyzerName);
    } catch (const std::exception &e) {
      logger.fatal("Failed to get extra info: " + std::string(e.what()));
      response = TcpServerMessage::NOK;
    }
    shouldNotifyClients = true;
  } break;

  default:
    logger.fatal("Invalid command: " +
                 std::to_string(static_cast<std::uint32_t>(command)));
    response = TcpServerMessage::NOK;
    break;
  }

  // Respond to the command
  size_t byteWritten = asio::write(
      *session.getCommandSocket(),
      asio::buffer(constructResponsePacket(command, response)), error);
  if (byteWritten != BYTES_IN_SERVER_PACKET_HEADER || error) {
    logger.fatal("TCP write error: " + error.message());
    return false;
  }
  if (shouldNotifyClients) {
    notifyClientsOfStateChange(command);
  }

  return true;
}

void TcpServer::notifyClientsOfStateChange(TcpServerCommand command) {
  auto packet =
      constructResponsePacket(command, TcpServerMessage::STATES_CHANGED);

  // TODO Write doc

  std::shared_lock lock(m_SessionMutex);
  for (const auto &sessionPair : m_Sessions) {
    const auto &session = sessionPair.second;
    if (!session || !session->isConnected()) {
      continue;
    }

    asio::write(*session->getResponseSocket(), asio::buffer(packet));
  }
}

nlohmann::json TcpServer::handleExtraData(TcpServerCommand command,
                                          asio::error_code &error,
                                          const ClientSession &session) {
  auto &logger = utils::Logger::getInstance();

  // Send an acknowledgment to the client we are ready to receive the data
  size_t byteWritten =
      asio::write(*session.getCommandSocket(),
                  asio::buffer(constructResponsePacket(
                      command, TcpServerMessage::LISTENING_EXTRA_DATA)),
                  error);

  // Receive the size of the data
  auto buffer = std::array<char, BYTES_IN_CLIENT_PACKET_HEADER>();
  size_t byteRead =
      asio::read(*session.getResponseSocket(), asio::buffer(buffer), error);
  if (byteRead != BYTES_IN_CLIENT_PACKET_HEADER || error) {
    logger.fatal("TCP read error: " + error.message());
    throw std::runtime_error("Failed to read the size of the data");
  }

  // Parse the size of the data
  std::uint32_t dataSize =
      static_cast<std::uint32_t>(parseCommandPacket(buffer));
  auto dataBuffer = std::vector<char>(dataSize);
  byteRead =
      asio::read(*session.getResponseSocket(), asio::buffer(dataBuffer), error);
  if (byteRead != dataSize || error) {
    logger.fatal("TCP read error: " + error.message());
    throw std::runtime_error("Failed to read the data");
  }

  return nlohmann::json::parse(dataBuffer.begin(), dataBuffer.end());
}

bool TcpServer::addDevice(const std::string &deviceName) {
  auto &logger = utils::Logger::getInstance();

  // Check if [m_ConnectedDeviceIds] contains the device
  if (m_ConnectedDeviceIds.find(deviceName) != m_ConnectedDeviceIds.end()) {
    logger.warning("Cannot add the " + deviceName +
                   " devise as it is already connected");
    return false;
  }

  // Stop the data streaming when changing the devices
  m_Devices.stopDataStreaming();
  makeAndAddDevice(deviceName);
  if (!m_Devices.connect()) {
    removeDevice(deviceName);
    return false;
  }
  m_Devices.startDataStreaming();

  return true;
}

bool TcpServer::setZeroLevel(const std::string &deviceName) {
  auto &logger = utils::Logger::getInstance();

  // Check if [m_ConnectedDeviceIds] contains the device
  if (m_ConnectedDeviceIds.find(deviceName) == m_ConnectedDeviceIds.end()) {
    logger.warning(deviceName + " not connected");
    return false;
  }

  m_Devices.zeroLevelDevice(deviceName);
  return true;
}

void TcpServer::makeAndAddDevice(const std::string &deviceName) {
  auto &logger = utils::Logger::getInstance();

  if (deviceName == DEVICE_NAME_DELSYS_ANALOG) {
    bool isInitialized = false;
    for (auto &id : m_Devices.getDeviceIds()) {
      if (dynamic_cast<const devices::DelsysBaseDevice *>(&m_Devices[id])) {
        isInitialized = true;
        m_ConnectedDeviceIds[DEVICE_NAME_DELSYS_ANALOG] =
            m_Devices.add(std::make_unique<devices::DelsysAnalogDevice>(
                static_cast<const devices::DelsysBaseDevice &>(m_Devices[id])));
        break;
      }
    }
    if (!isInitialized) {
      m_ConnectedDeviceIds[DEVICE_NAME_DELSYS_ANALOG] =
          m_Devices.add(std::make_unique<devices::DelsysAnalogDevice>());
    }

  } else if (deviceName == DEVICE_NAME_DELSYS_EMG) {
    bool isInitialized = false;
    for (auto &id : m_Devices.getDeviceIds()) {
      if (dynamic_cast<const devices::DelsysBaseDevice *>(&m_Devices[id])) {
        isInitialized = true;
        m_ConnectedDeviceIds[DEVICE_NAME_DELSYS_EMG] =
            m_Devices.add(std::make_unique<devices::DelsysEmgDevice>(
                static_cast<const devices::DelsysBaseDevice &>(m_Devices[id])));
        break;
      }
    }
    if (!isInitialized) {
      m_ConnectedDeviceIds[DEVICE_NAME_DELSYS_EMG] =
          m_Devices.add(std::make_unique<devices::DelsysEmgDevice>());
    }

  } else if (deviceName == DEVICE_NAME_MAGSTIM) {
    m_ConnectedDeviceIds[DEVICE_NAME_MAGSTIM] =
        m_Devices.add(devices::MagstimRapidDevice::findMagstimDevice());

  } else {
    logger.fatal("Invalid device name: " + deviceName);
    throw std::runtime_error("Invalid device name: " + deviceName);
  }
}

bool TcpServer::removeDevice(const std::string &deviceName,
                             bool restartStreaming) {
  auto &logger = utils::Logger::getInstance();

  // Check if [m_ConnectedDeviceIds] contains the device
  if (m_ConnectedDeviceIds.find(deviceName) == m_ConnectedDeviceIds.end()) {
    logger.warning(deviceName + " not connected");
    return false;
  }

  // Stop the data streaming when changing the devices
  m_Devices.stopDataStreaming();
  m_Devices.remove(m_ConnectedDeviceIds[deviceName]);
  m_ConnectedDeviceIds.erase(deviceName);
  if (restartStreaming) {
    m_Devices.startDataStreaming();
  }

  return true;
}

void TcpServer::liveDataLoop() {
  m_LiveDataTimer->expires_at(std::chrono::steady_clock::now() +
                              std::chrono::milliseconds(100));

  m_LiveDataTimer->async_wait([this](const asio::error_code &ec) {
    auto &logger = utils::Logger::getInstance();
    if (ec) {
      logger.info("Live data loop stopped");
      return;
    }
    logger.debug("Sending live data to client");

    auto data = m_Devices.getLiveDataSerialized();
    if (data.size() == 0) {
      // Reschedule the next execution
      liveDataLoop();
      return;
    }
    auto dataDump = data.dump();

    auto packet = constructResponsePacket(
        TcpServerCommand::NONE, TcpServerMessage::SENDING_DATA,
        TcpServerDataType::LIVE_DATA, dataDump.size(), dataDump);

    // Send the data to all the clients
    asio::error_code error;
    std::shared_lock lock(m_SessionMutex);
    for (auto &sessionPair : m_Sessions) {
      try {
        const auto &session = sessionPair.second;
        if (!session || !session->isConnected()) {
          continue;
        }

        auto &socket = session->getLiveDataSocket();
        asio::write(*socket, asio::buffer(packet), error);
      } catch (const std::exception &) {
        // Do nothing and hope for the best
      }
    }
    logger.debug("Sent live data of size: " + std::to_string(dataDump.size()) +
                 " to " + std::to_string(m_Sessions.size()) + " clients");

    // Reschedule the next execution
    liveDataLoop();
  });
}

void TcpServer::liveAnalysesLoop() {
  m_LiveAnalysesTimer->expires_at(std::chrono::steady_clock::now() +
                                  std::chrono::milliseconds(25));
  m_LiveAnalysesTimer->async_wait([this](const asio::error_code &ec) {
    auto &logger = utils::Logger::getInstance();
    if (ec) {
      logger.info("Live analyses loop stopped");
      return;
    }

    if (m_Analyzers.size() == 0) {
      // No analyzers to run, reschedule the next execution
      liveAnalysesLoop();
      return;
    }

    std::map<std::string, data::TimeSeries> data = m_Devices.getLiveData();
    if (data.size() == 0) {
      // No data to analyze, reschedule the next execution
      liveAnalysesLoop();
      return;
    }

    // If we get here, we have data to analyze, so we do analyze them
    logger.debug("Analyzing live data");
    analyzer::Predictions predictions;
    try {
      predictions = m_Analyzers.predict(data);
    } catch (const analyzer::TimeWentBackwardException &) {
      // This can happen quite a lot if intervals is faster than the one
      // from data collector, so we just ignore it
      liveAnalysesLoop();
      return;
    } catch (...) {
      // Some other can get wrong when the user starts the analyzer before
      // starting the data collector, so we log the error and reschedule the
      // next execution
      logger.fatal("Failed to analyze live data");
      liveAnalysesLoop();
      return;
    }

    // Serialize the predictions
    auto dataDump = predictions.serialize().dump();
    auto packet = constructResponsePacket(
        TcpServerCommand::NONE, TcpServerMessage::SENDING_DATA,
        TcpServerDataType::LIVE_ANALYSES, dataDump.size(), dataDump);

    asio::error_code error;
    std::shared_lock lock(m_SessionMutex);
    for (auto &sessionPair : m_Sessions) {
      try {
        auto &session = sessionPair.second;
        if (!session || !session->isConnected()) {
          continue; // Skip disconnected sessions
        }

        auto &socket = session->getLiveAnalysesSocket();
        if (!socket || !socket->is_open()) {
          // Skip if the socket is not connected. This should not happen, but
          // because of a race condition, it did happen once, so we
          // handle it anyway
          continue;
        }
        asio::write(*socket, asio::buffer(packet), error);
      } catch (const std::exception &) {
        // Do nothing and hope for the best
      }
    }
    logger.debug("Live analyses data size: " + std::to_string(dataDump.size()) +
                 " sent to " + std::to_string(m_Sessions.size()) + " clients");
    liveAnalysesLoop();
  });
}

void TcpServerMock::makeAndAddDevice(const std::string &deviceName) {
  auto &logger = utils::Logger::getInstance();

  if (deviceName == DEVICE_NAME_DELSYS_ANALOG) {
    bool isInitialized = false;
    for (auto &id : m_Devices.getDeviceIds()) {
      if (dynamic_cast<const devices::DelsysBaseDevice *>(&m_Devices[id])) {
        isInitialized = true;
        m_ConnectedDeviceIds[DEVICE_NAME_DELSYS_ANALOG] =
            m_Devices.add(std::make_unique<devices::DelsysAnalogDeviceMock>(
                static_cast<const devices::DelsysBaseDevice &>(m_Devices[id])));
        break;
      }
    }
    if (!isInitialized) {
      m_ConnectedDeviceIds[DEVICE_NAME_DELSYS_ANALOG] =
          m_Devices.add(std::make_unique<devices::DelsysAnalogDeviceMock>());
    }

  } else if (deviceName == DEVICE_NAME_DELSYS_EMG) {
    bool isInitialized = false;
    for (auto &id : m_Devices.getDeviceIds()) {
      if (dynamic_cast<const devices::DelsysBaseDevice *>(&m_Devices[id])) {
        isInitialized = true;
        m_ConnectedDeviceIds[DEVICE_NAME_DELSYS_EMG] =
            m_Devices.add(std::make_unique<devices::DelsysEmgDeviceMock>(
                static_cast<const devices::DelsysBaseDevice &>(m_Devices[id])));
        break;
      }
    }
    if (!isInitialized) {
      m_ConnectedDeviceIds[DEVICE_NAME_DELSYS_EMG] =
          m_Devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
    }

  } else if (deviceName == DEVICE_NAME_MAGSTIM) {
    m_ConnectedDeviceIds[DEVICE_NAME_MAGSTIM] =
        m_Devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());

  } else {
    logger.fatal("Invalid device name: " + deviceName);
    throw std::runtime_error("Invalid device name: " + deviceName);
  }
}