#include "Server/TcpServer.h"

#include "Utils/Logger.h"
#include <asio/steady_timer.hpp>
#include <thread>

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

std::array<char, BYTES_IN_SERVER_PACKET_HEADER>
constructResponsePacket(TcpServerResponse response) {
  // Packets are exactly 16 bytes long, litte endian
  // - First 4 bytes are the version number
  // - The next 8 bytes are the timestamp of the packet (milliseconds since
  // epoch)
  // - Next 4 bytes are the actual response

  auto packet = std::array<char, BYTES_IN_SERVER_PACKET_HEADER>();
  packet.fill('\0');

  // Add the version number in uint32_t format (litte endian)
  std::uint32_t versionLittleEndian = htole32(COMMUNICATION_PROTOCOL_VERSION);
  std::memcpy(packet.data(), &versionLittleEndian, sizeof(versionLittleEndian));

  // Add the timestamps in uint64_t format (litte endian)
  std::uint64_t timestamp =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  std::uint64_t timestampLittleEndian = htole64(timestamp);
  std::memcpy(packet.data() + sizeof(versionLittleEndian),
              &timestampLittleEndian, sizeof(timestampLittleEndian));

  // Add the response in uint32_t format (litte endian)
  std::uint32_t responseLittleEndian =
      htole32(static_cast<std::uint32_t>(response));
  std::memcpy(packet.data() + sizeof(versionLittleEndian) +
                  sizeof(timestampLittleEndian),
              &responseLittleEndian, sizeof(responseLittleEndian));

  return packet;
}

// Here are the names of the devices that can be connected (for internal use)
const std::string DEVICE_NAME_DELSYS_EMG = "DelsysEmgDevice";
const std::string DEVICE_NAME_DELSYS_ANALOG = "DelsysAnalogDevice";
const std::string DEVICE_NAME_MAGSTIM = "MagstimRapidDevice";

ClientSession::ClientSession(
    std::shared_ptr<asio::io_context> context, std::string id,
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

  utils::Logger::getInstance().info("Session " + m_Id +
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
    logger.warning("Client session " + m_Id +
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
  logger.info("Starting client session with ID: " + m_Id);

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
      m_Context(std::make_shared<asio::io_context>()) {
  m_LiveDataTimer = std::make_shared<asio::steady_timer>(
      *m_Context, std::chrono::milliseconds(100));
  m_LiveAnalysesTimer = std::make_shared<asio::steady_timer>(
      *m_Context, std::chrono::milliseconds(25));
  liveDataLoop();
  liveAnalysesLoop();
}

TcpServer::~TcpServer() { stopServer(); }

void TcpServer::startServer() {
  m_ServerWorker = std::thread([this]() { startServerSync(); });
}

void TcpServer::startServerSync() {
  auto &logger = utils::Logger::getInstance();

  startAcceptors();

  m_Status = TcpServerStatus::CONNECTING;
  startAcceptingSocketConnexions();
  m_Context->run();
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
  std::string sessionId = readSessionId(socket);

  auto session = getOrCreateSession(sessionId);
  session->connectCommandSocket(socket);
}

void TcpServer::handleResponseSocketConnexion(
    std::shared_ptr<asio::ip::tcp::socket> socket) {
  std::string sessionId = readSessionId(socket);

  auto session = getOrCreateSession(sessionId);
  session->connectResponseSocket(socket);
}

void TcpServer::handleLiveDataSocket(
    std::shared_ptr<asio::ip::tcp::socket> socket) {
  std::string sessionId = readSessionId(socket);

  auto session = getOrCreateSession(sessionId);
  session->connectLiveDataSocket(socket);
}

void TcpServer::handleLiveAnalysesSocket(
    std::shared_ptr<asio::ip::tcp::socket> socket) {
  std::string sessionId = readSessionId(socket);

  auto session = getOrCreateSession(sessionId);
  session->connectLiveAnalysesSocket(socket);
}

std::shared_ptr<ClientSession>
TcpServer::getOrCreateSession(const std::string &id) {
  std::lock_guard<std::mutex> lock(m_SessionMutex);
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
          handleClientHasDisconnected(client);
        },
        m_TimeoutPeriod);
  }
  return session;
}

std::string
TcpServer::readSessionId(std::shared_ptr<asio::ip::tcp::socket> socket) {
  return "coucou";
}

void TcpServer::handleClientHasDisconnected(const ClientSession &session) {
  std::lock_guard<std::mutex> lock(m_SessionMutex);
  auto it = m_Sessions.find(session.getId());
  if (it == m_Sessions.end())
    return;
  m_Sessions.erase(it);
}

void TcpServer::stopServer() {
  if (m_Context->stopped()) {
    return;
  }

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
  logger.info("Disconnecting all the clients");
  for (auto &sessionPair : m_Sessions) {
    sessionPair.second->disconnect();
  }

  // Cancel all the acceptors
  logger.info("Canceling all the acceptors");
  cancelAcceptors();

  // Stop any running context
  m_Context->stop();

  // Wait for the server to stop
  if (m_ServerWorker.joinable()) {
    m_ServerWorker.join();
  }
  logger.info("Server has shut down");
}

bool TcpServer::isClientConnected(const std::string &id) const {
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
  auto &logger = utils::Logger::getInstance();
  asio::error_code error;

  // The only valid command during initialization is the handshake
  if (command != TcpServerCommand::HANDSHAKE) {
    logger.fatal("Invalid command during initialization: " +
                 std::to_string(static_cast<std::uint32_t>(command)));

    asio::write(*session.getCommandSocket(),
                asio::buffer(constructResponsePacket(TcpServerResponse::NOK)),
                error);
    return false;
  }

  // Respond OK to the handshake
  size_t byteWritten = asio::write(
      *session.getCommandSocket(),
      asio::buffer(constructResponsePacket(TcpServerResponse::OK)), error);
  if (byteWritten != BYTES_IN_SERVER_PACKET_HEADER || error) {
    logger.fatal("TCP write error: " + error.message());
    return false;
  }

  // Set the status to running
  m_Status = TcpServerStatus::CONNECTED;
  logger.info("Handshake from client is valid");

  return true;
}

bool TcpServer::handleCommand(TcpServerCommand command,
                              const ClientSession &session) {
  auto &logger = utils::Logger::getInstance();
  asio::error_code error;

  // Handle the command
  TcpServerResponse response;
  switch (command) {
  case TcpServerCommand::CONNECT_DELSYS_ANALOG:
    response = addDevice(DEVICE_NAME_DELSYS_ANALOG) ? TcpServerResponse::OK
                                                    : TcpServerResponse::NOK;
    break;

  case TcpServerCommand::CONNECT_DELSYS_EMG:
    response = addDevice(DEVICE_NAME_DELSYS_EMG) ? TcpServerResponse::OK
                                                 : TcpServerResponse::NOK;
    break;

  case TcpServerCommand::CONNECT_MAGSTIM:
    response = addDevice(DEVICE_NAME_MAGSTIM) ? TcpServerResponse::OK
                                              : TcpServerResponse::NOK;
    break;

  case TcpServerCommand::ZERO_DELSYS_ANALOG:
    response = m_Devices.zeroLevelDevice(DEVICE_NAME_DELSYS_ANALOG)
                   ? TcpServerResponse::OK
                   : TcpServerResponse::NOK;
    break;

  case TcpServerCommand::ZERO_DELSYS_EMG:
    response = m_Devices.zeroLevelDevice(DEVICE_NAME_DELSYS_EMG)
                   ? TcpServerResponse::OK
                   : TcpServerResponse::NOK;
    break;

  case TcpServerCommand::DISCONNECT_DELSYS_ANALOG:
    response = removeDevice(DEVICE_NAME_DELSYS_ANALOG) ? TcpServerResponse::OK
                                                       : TcpServerResponse::NOK;
    break;

  case TcpServerCommand::DISCONNECT_DELSYS_EMG:
    response = removeDevice(DEVICE_NAME_DELSYS_EMG) ? TcpServerResponse::OK
                                                    : TcpServerResponse::NOK;
    break;

  case TcpServerCommand::DISCONNECT_MAGSTIM:
    response = removeDevice(DEVICE_NAME_MAGSTIM) ? TcpServerResponse::OK
                                                 : TcpServerResponse::NOK;
    break;

  case TcpServerCommand::START_RECORDING:
    response = m_Devices.startRecording() ? TcpServerResponse::OK
                                          : TcpServerResponse::NOK;
    break;

  case TcpServerCommand::STOP_RECORDING:
    response = m_Devices.stopRecording() ? TcpServerResponse::OK
                                         : TcpServerResponse::NOK;
    break;

  case TcpServerCommand::GET_LAST_TRIAL_DATA: {
    auto data = m_Devices.getLastTrialDataSerialized();
    auto dataDump = data.dump();
    asio::write(*session.getResponseSocket(),
                asio::buffer(constructResponsePacket(
                    static_cast<TcpServerResponse>(dataDump.size()))),
                error);
    auto written = asio::write(*session.getResponseSocket(),
                               asio::buffer(dataDump), error);
    logger.info("Data size: " + std::to_string(written));
    response = TcpServerResponse::OK;
  } break;

  case TcpServerCommand::ADD_ANALYZER: {
    try {
      auto data = handleExtraData(error, session);
      m_Analyzers.add(data);
      response = TcpServerResponse::OK;
    } catch (const std::exception &e) {
      logger.fatal("Failed to get extra info: " + std::string(e.what()));
      response = TcpServerResponse::NOK;
      break;
    } catch (...) {
      logger.fatal("Failed to get extra info");
      response = TcpServerResponse::NOK;
      break;
    }
  } break;

  case TcpServerCommand::REMOVE_ANALYZER: {
    try {
      std::string analyzerName = handleExtraData(error, session)["analyzer"];
      m_Analyzers.remove(analyzerName);
      response = TcpServerResponse::OK;
    } catch (const std::exception &e) {
      logger.fatal("Failed to get extra info: " + std::string(e.what()));
      response = TcpServerResponse::NOK;
      break;
    }
  } break;

  default:
    logger.fatal("Invalid command: " +
                 std::to_string(static_cast<std::uint32_t>(command)));
    response = TcpServerResponse::NOK;
    break;
  }

  // Respond OK to the command
  size_t byteWritten =
      asio::write(*session.getCommandSocket(),
                  asio::buffer(constructResponsePacket(response)), error);

  // TODO Add events to notify all the clients the something has happened
  if (byteWritten != BYTES_IN_SERVER_PACKET_HEADER || error) {
    logger.fatal("TCP write error: " + error.message());
    return false;
  }

  return true;
}

nlohmann::json TcpServer::handleExtraData(asio::error_code &error,
                                          const ClientSession &session) {
  auto &logger = utils::Logger::getInstance();

  // Send an acknowledgment to the client we are ready to receive the data
  size_t byteWritten = asio::write(
      *session.getCommandSocket(),
      asio::buffer(constructResponsePacket(TcpServerResponse::OK)), error);

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
    auto dataSize = asio::buffer(constructResponsePacket(
        static_cast<TcpServerResponse>(dataDump.size())));
    auto dataToSend = asio::buffer(dataDump);

    // Send the data to all the clients
    asio::error_code error;
    for (auto &session : m_Sessions) {
      try {
        auto &socket = session.second->getLiveDataSocket();
        asio::write(*socket, dataSize, error);
        asio::write(*socket, dataToSend, error);
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
    auto dataSize = asio::buffer(constructResponsePacket(
        static_cast<TcpServerResponse>(dataDump.size())));
    auto dataToSend = asio::buffer(dataDump);

    asio::error_code error;
    for (auto &session : m_Sessions) {
      try {
        auto &socket = session.second->getLiveAnalysesSocket();
        asio::write(*socket, dataSize, error);
        asio::write(*socket, dataToSend, error);
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