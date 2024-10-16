#include "Server/TcpServer.h"

#include "Utils/Logger.h"
#include <thread>

#include "Devices/Concrete/DelsysEmgDevice.h"
#include "Devices/Concrete/MagstimRapidDevice.h"
#include "Devices/Generic/Device.h"

using asio::ip::tcp;
using namespace STIMWALKER_NAMESPACE::server;

// Here are the names of the devices that can be connected (for internal use)
const std::string DEVICE_NAME_DELSYS = "DelsysEmgDevice";
const std::string DEVICE_NAME_MAGSTIM = "MagstimRapidDevice";

TcpServer::TcpServer(int commandPort, int dataPort)
    : m_IsStarted(false), m_IsClientConnected(false),
      m_CommandPort(commandPort), m_DataPort(dataPort), m_ProtocolVersion(1) {};

TcpServer::~TcpServer() {
  if (m_IsStarted) {
    stopServer();
  }
}

void TcpServer::startServer() {
  auto &logger = utils::Logger::getInstance();

  // Create the contexts and acceptors
  m_CommandAcceptor = std::make_unique<tcp::acceptor>(
      m_Context, tcp::endpoint(tcp::v4(), m_CommandPort));
  logger.info("Command server started on port " +
              std::to_string(m_CommandPort));
  m_DataAcceptor = std::make_unique<tcp::acceptor>(
      m_Context, tcp::endpoint(tcp::v4(), m_DataPort));
  logger.info("Data server started on port " + std::to_string(m_DataPort));

  m_IsStarted = true;
  while (m_IsStarted) {
    // Accept a new connection
    waitForNewConnexion();

    // Start the command worker to handle the commands
    auto commandWorker = std::thread([this]() {
      while (m_IsStarted && m_IsClientConnected) {
        waitAndHandleNewCommand();
      }
    });

    auto dataWorker = std::thread([this]() {
      while (m_IsStarted && m_IsClientConnected) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    });

    // Wait for the workers to finish
    commandWorker.join();
    dataWorker.join();
  }

  // If we get here, the server has been stopped
  logger.info("Server has shut down");
}

void TcpServer::stopServer() {
  auto &logger = utils::Logger::getInstance();

  // Make sure all the devices are properly disconnected
  if (m_IsClientConnected) {
    disconnectClient();
  }
  std::lock_guard<std::mutex> lock(m_Mutex);

  // Close the acceptor
  if (m_CommandAcceptor->is_open()) {
    m_CommandAcceptor->close();
  }
  if (m_DataAcceptor->is_open()) {
    m_DataAcceptor->close();
  }

  // Shutdown the server down
  m_IsStarted = false;
}

void TcpServer::disconnectClient() {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsClientConnected) {
    logger.warning("Client already disconnected");
    return;
  }
  std::lock_guard<std::mutex> lock(m_Mutex);

  // Make sure all the devices are properly disconnected
  m_Devices.clear();
  if (m_CommandSocket->is_open()) {
    m_CommandSocket->close();
  }
  if (m_DataSocket->is_open()) {
    m_DataSocket->close();
  }

  // Reset the status to initializing
  m_Status = TcpServerStatus::INITIALIZING;
  m_IsClientConnected = false;
  logger.info("Client disconnected");
}

void TcpServer::waitForNewConnexion() {
  auto &logger = utils::Logger::getInstance();

  m_Status = TcpServerStatus::INITIALIZING;
  m_CommandSocket = std::make_unique<tcp::socket>(m_Context);
  m_DataSocket = std::make_unique<tcp::socket>(m_Context);

  m_CommandAcceptor->accept(*m_CommandSocket);
  logger.info("Command socket connected to client, waiting for data socket");
  // TODO Check for a non-blocking accept
  m_DataAcceptor->accept(*m_DataSocket);
  logger.info("Data socket connected to client, waiting for handshake");
  m_IsClientConnected = true;
  m_CommandSocket->non_blocking(true);

  auto now = std::chrono::high_resolution_clock::now();
  while (m_Status == TcpServerStatus::INITIALIZING) {
    // Wait for the handshake
    waitAndHandleNewCommand();

    if (std::chrono::high_resolution_clock::now() - now >
        std::chrono::seconds(5)) {
      logger.fatal("Handshake timeout");
      disconnectClient();
      return;
    }
  }
}

void TcpServer::waitAndHandleNewCommand() {
  auto &logger = utils::Logger::getInstance();

  // Lock the mutex during the time the command is answered
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (!m_IsStarted || !m_IsClientConnected) {
    // Just a sanity check after the sleep and getting the lock
    return;
  }

  auto buffer = std::array<char, 8>();
  asio::error_code error;
  size_t byteRead = asio::read(*m_CommandSocket, asio::buffer(buffer), error);

  // Since command is non-blocking, we can just continue if there is no data
  if (error == asio::error::would_block) {
    return;
  }

  // If something went wrong, disconnect the client and stop everything
  if (byteRead == 0 || byteRead > 1024 || error) {
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
    logger.warning(deviceName + " already connected");
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
