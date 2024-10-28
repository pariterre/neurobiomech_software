#include "Server/TcpServer.h"

#include "Utils/Logger.h"
#include <asio/steady_timer.hpp>
#include <thread>

#include "Devices/Concrete/DelsysEmgDevice.h"
#include "Devices/Concrete/MagstimRapidDevice.h"
#include "Devices/Generic/Device.h"

using namespace STIMWALKER_NAMESPACE::server;

// Here are the names of the devices that can be connected (for internal use)
const std::string DEVICE_NAME_DELSYS = "DelsysEmgDevice";
const std::string DEVICE_NAME_MAGSTIM = "MagstimRapidDevice";

TcpServer::TcpServer(int commandPort, int responsePort)
    : m_IsServerRunning(false), m_CommandPort(commandPort),
      m_ResponsePort(responsePort),
      m_TimeoutPeriod(std::chrono::milliseconds(5000)), m_ProtocolVersion(1) {};

TcpServer::~TcpServer() { stopServer(); }

void TcpServer::startServer() {
  m_ServerWorker = std::thread([this]() { startServerSync(); });
}

void TcpServer::startServerSync() {
  auto &logger = utils::Logger::getInstance();

  // Create the contexts and acceptors
  m_CommandAcceptor = std::make_unique<asio::ip::tcp::acceptor>(
      m_CommandContext,
      asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_CommandPort));
  logger.info("Command server started on port " +
              std::to_string(m_CommandPort));
  m_ResponseAcceptor = std::make_unique<asio::ip::tcp::acceptor>(
      m_ResponseContext,
      asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_ResponsePort));
  logger.info("Response server started on port " +
              std::to_string(m_ResponsePort));

  m_IsServerRunning = true;
  while (m_IsServerRunning) {
    // Accept a new connection
    waitForNewConnexion();
    if (!isClientConnected()) {
      // If it failed to connect, restart the process
      continue;
    }

    // Start the command worker to handle the commands
    auto commandWorker = std::thread([this]() {
      while (m_IsServerRunning && isClientConnected()) {
        waitAndHandleNewCommand();
      }
    });

    auto responseWorker = std::thread([this]() {
      while (m_IsServerRunning && isClientConnected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    });

    // Wait for the workers to finish
    commandWorker.join();
    responseWorker.join();
  }
}

void TcpServer::stopServer() {
  auto &logger = utils::Logger::getInstance();

  // Give a bit of time so it things went too fast the startServerSync is
  // actually ready
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // Shutdown the server down
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_IsServerRunning = false;
  }

  // Make sure all the devices are properly disconnected
  if (isClientConnected()) {
    disconnectClient();
  }

  // When closing the server too soon, they can be open even if client was not
  // connected
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_CommandSocket && m_CommandSocket->is_open()) {
      m_CommandSocket->close();
    }
    if (m_ResponseSocket && m_ResponseSocket->is_open()) {
      m_ResponseSocket->close();
    }

    if (m_CommandAcceptor) {
      m_CommandAcceptor->close();
    }
    if (m_ResponseAcceptor) {
      m_ResponseAcceptor->close();
    }
  }
  // Stop any running context
  m_CommandContext.stop();

  // Wait for the server to stop
  if (m_ServerWorker.joinable()) {
    m_ServerWorker.join();
  }
  logger.info("Server has shut down");
}

void TcpServer::disconnectClient() {
  auto &logger = utils::Logger::getInstance();

  if (isClientConnected()) {
    logger.info("Disconnecting client");
  }

  // Make sure all the devices are properly disconnected
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Devices.clear();
  if (m_CommandSocket && m_CommandSocket->is_open()) {
    m_CommandSocket->close();
  }
  if (m_ResponseSocket && m_ResponseSocket->is_open()) {
    m_ResponseSocket->close();
  }

  // Reset the status to initializing
  m_Status = TcpServerStatus::INITIALIZING;
}

bool TcpServer::isClientConnected() const {
  return m_CommandSocket && m_CommandSocket->is_open() && m_ResponseSocket &&
         m_ResponseSocket->is_open();
}

void TcpServer::waitForNewConnexion() {
  auto &logger = utils::Logger::getInstance();
  logger.info("Waiting for a new connexion");

  m_Status = TcpServerStatus::INITIALIZING;

  // Wait for the command socket to connect
  auto commandTimer =
      asio::steady_timer(m_CommandContext, std::chrono::milliseconds(10));
  m_CommandSocket = std::make_unique<asio::ip::tcp::socket>(m_CommandContext);
  m_CommandAcceptor->async_accept(*m_CommandSocket,
                                  [](const asio::error_code &) {});
  // Wait for a connexion while periodically checking if the server is still
  // running
  while (!m_CommandSocket->is_open()) {
    commandTimer.async_wait(
        [this](const asio::error_code &) { m_CommandContext.stop(); });
    m_CommandContext.run();
    m_CommandContext.restart();

    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_IsServerRunning) {
      if (m_CommandAcceptor && m_CommandAcceptor->is_open()) {
        m_CommandAcceptor->cancel();
      }
      logger.info("Stopping listening to ports as server is shutting down");
      return;
    }
  }
  logger.info("Command socket connected to client, waiting for a connexion to "
              "the response socket");

  // Wait for the response socket to connect
  auto responseSocketTimer =
      asio::steady_timer(m_ResponseContext, std::chrono::milliseconds(10));
  m_ResponseSocket = std::make_unique<asio::ip::tcp::socket>(m_ResponseContext);
  m_ResponseAcceptor->async_accept(*m_ResponseSocket, [](asio::error_code) {});
  auto now = std::chrono::high_resolution_clock::now();
  while (!m_ResponseSocket->is_open()) {
    responseSocketTimer.async_wait(
        [this](const asio::error_code &) { m_ResponseContext.stop(); });
    m_ResponseContext.run();
    m_ResponseContext.restart();

    if (!m_IsServerRunning ||
        std::chrono::high_resolution_clock::now() - now > m_TimeoutPeriod) {
      std::lock_guard<std::mutex> lock(m_Mutex);
      if (m_CommandSocket && m_CommandSocket->is_open()) {
        m_CommandSocket->close();
      }
      if (m_ResponseAcceptor && m_ResponseAcceptor->is_open()) {
        m_ResponseAcceptor->cancel();
      }
      if (!m_IsServerRunning) {
        logger.info("Stopping listening to ports as server is shutting down");
      } else if (std::chrono::high_resolution_clock::now() - now >
                 m_TimeoutPeriod) {
        logger.fatal("Response socket connection timeout (" +
                     std::to_string(m_TimeoutPeriod.count()) +
                     " ms), disconnecting client");
      }
      return;
    }
  }

  {
    logger.info(
        "Response socket connected to client, waiting for official handshake");
    m_CommandSocket->non_blocking(true);
  }

  now = std::chrono::high_resolution_clock::now();
  while (m_Status == TcpServerStatus::INITIALIZING) {
    // Wait for the handshake
    waitAndHandleNewCommand();

    // Since the command is non-blocking, we can just continue if there is no
    // data
    if (!m_IsServerRunning || !isClientConnected() ||
        std::chrono::high_resolution_clock::now() - now > m_TimeoutPeriod) {
      logger.fatal("Handshake timeout (" +
                   std::to_string(m_TimeoutPeriod.count()) +
                   " ms), disconnecting client");
      disconnectClient();
      return;
    }
  }
}

void TcpServer::waitAndHandleNewCommand() {
  auto &logger = utils::Logger::getInstance();

  // Lock the mutex during the time the command is answered
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_IsServerRunning || !isClientConnected()) {
      // Just a sanity check after the sleep and getting the lock
      return;
    }
  }

  auto buffer = std::array<char, 8>();
  asio::error_code error;
  size_t byteRead = asio::read(*m_CommandSocket, asio::buffer(buffer), error);

  if (error == asio::error::eof) {
    logger.info("Client disconnected");
    disconnectClient();
    return;
  }

  // Since command is non-blocking, we can just continue if there is no data
  if (!m_IsServerRunning || byteRead == 0 ||
      error == asio::error::would_block) {
    return;
  }

  // If something went wrong, disconnect the client and stop everything
  if (byteRead > 1024 || error) {
    logger.fatal("TCP read error: " + error.message());
    disconnectClient();
    return;
  }

  // Parse the packet
  TcpServerCommand command = parseCommandPacket(buffer);

  // Handle the command based on the current status
  bool isSuccessful = false;
  switch (m_Status) {
  case (TcpServerStatus::INITIALIZING):
    isSuccessful = handleHandshake(command);
    break;
  case (TcpServerStatus::CONNECTED):
    isSuccessful = handleCommand(command);
    break;
  default:
    logger.fatal("Invalid server status: " +
                 std::to_string(static_cast<std::uint32_t>(m_Status)));
    isSuccessful = false;
    break;
  }

  // If anything went wrong, disconnect the client
  if (!isSuccessful) {
    disconnectClient();
  }

  // If we get here, the command was successful and we can continue
  return;
}

bool TcpServer::handleHandshake(TcpServerCommand command) {
  auto &logger = utils::Logger::getInstance();
  asio::error_code error;

  // The only valid command during initialization is the handshake
  if (command != TcpServerCommand::HANDSHAKE) {
    logger.fatal("Invalid command during initialization: " +
                 std::to_string(static_cast<std::uint32_t>(command)));

    asio::write(*m_CommandSocket,
                asio::buffer(constructResponsePacket(TcpServerResponse::NOK)),
                error);
    return false;
  }

  // Respond OK to the handshake
  size_t byteWritten = asio::write(
      *m_CommandSocket,
      asio::buffer(constructResponsePacket(TcpServerResponse::OK)), error);
  if (byteWritten != 8 || error) {
    logger.fatal("TCP write error: " + error.message());
    return false;
  }

  // Set the status to running
  m_Status = TcpServerStatus::CONNECTED;
  logger.info("Handshake from client is valid");

  return true;
}

bool TcpServer::handleCommand(TcpServerCommand command) {
  auto &logger = utils::Logger::getInstance();
  asio::error_code error;

  // Handle the command
  TcpServerResponse response;
  switch (command) {
  case TcpServerCommand::CONNECT_DELSYS:
    response = addDevice(DEVICE_NAME_DELSYS) ? TcpServerResponse::OK
                                             : TcpServerResponse::NOK;
    break;
  case TcpServerCommand::CONNECT_MAGSTIM:
    response = addDevice(DEVICE_NAME_MAGSTIM) ? TcpServerResponse::OK
                                              : TcpServerResponse::NOK;
    break;

  case TcpServerCommand::DISCONNECT_DELSYS:
    response = removeDevice(DEVICE_NAME_DELSYS) ? TcpServerResponse::OK
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
    response = static_cast<TcpServerResponse>(dataDump.size());
    asio::write(*m_ResponseSocket,
                asio::buffer(constructResponsePacket(response)), error);
    auto written =
        asio::write(*m_ResponseSocket, asio::buffer(dataDump), error);
    logger.info("Data size: " + std::to_string(written));
    response = TcpServerResponse::OK;
  } break;

  default:
    logger.fatal("Invalid command: " +
                 std::to_string(static_cast<std::uint32_t>(command)));
    response = TcpServerResponse::NOK;
    break;
  }

  // Respond OK to the command
  size_t byteWritten = asio::write(
      *m_CommandSocket, asio::buffer(constructResponsePacket(response)), error);
  if (byteWritten != 8 || error) {
    logger.fatal("TCP write error: " + error.message());
    return false;
  }

  return true;
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
  m_Devices.connect();
  m_Devices.startDataStreaming();

  return true;
}

void TcpServer::makeAndAddDevice(const std::string &deviceName) {
  auto &logger = utils::Logger::getInstance();

  if (deviceName == DEVICE_NAME_DELSYS) {
    m_ConnectedDeviceIds[DEVICE_NAME_DELSYS] =
        m_Devices.add(std::make_unique<devices::DelsysEmgDevice>());
  } else if (deviceName == DEVICE_NAME_MAGSTIM) {
    m_ConnectedDeviceIds[DEVICE_NAME_MAGSTIM] =
        m_Devices.add(devices::MagstimRapidDevice::findMagstimDevice());
  } else {
    logger.fatal("Invalid device name: " + deviceName);
    throw std::runtime_error("Invalid device name: " + deviceName);
  }
}

bool TcpServer::removeDevice(const std::string &deviceName) {
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
  m_Devices.startDataStreaming();

  return true;
}

TcpServerCommand
TcpServer::parseCommandPacket(const std::array<char, 8> &buffer) {
  // Packets are exactly 8 bytes long, big-endian
  // - First 4 bytes are the version number
  // - Next 4 bytes are the command

  // Check the version
  std::uint32_t version =
      *reinterpret_cast<const std::uint32_t *>(buffer.data());
  if (version != m_ProtocolVersion) {
    auto &logger = utils::Logger::getInstance();
    logger.fatal("Invalid version: " + std::to_string(version) +
                 ". Please "
                 "update the client to version " +
                 std::to_string(m_ProtocolVersion));
    return TcpServerCommand::FAILED;
  }

  // Get the command
  return static_cast<TcpServerCommand>(
      *reinterpret_cast<const std::uint32_t *>(buffer.data() + 4));
}

std::array<char, 8>
TcpServer::constructResponsePacket(TcpServerResponse response) {
  // Packets are exactly 8 bytes long, big-endian
  // - First 4 bytes are the version number
  // - Next 4 bytes are the response

  auto packet = std::array<char, 8>();
  packet.fill('\0');

  // Add the version number
  std::memcpy(packet.data(), &m_ProtocolVersion, sizeof(std::uint32_t));

  // Add the response
  std::memcpy(packet.data() + sizeof(std::uint32_t), &response,
              sizeof(TcpServerResponse));

  return packet;
}

void TcpServerMock::makeAndAddDevice(const std::string &deviceName) {
  auto &logger = utils::Logger::getInstance();

  if (deviceName == DEVICE_NAME_DELSYS) {
    m_ConnectedDeviceIds[DEVICE_NAME_DELSYS] =
        m_Devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());
  } else if (deviceName == DEVICE_NAME_MAGSTIM) {
    m_ConnectedDeviceIds[DEVICE_NAME_MAGSTIM] =
        m_Devices.add(devices::MagstimRapidDeviceMock::findMagstimDevice());
  } else {
    logger.fatal("Invalid device name: " + deviceName);
    throw std::runtime_error("Invalid device name: " + deviceName);
  }
}
