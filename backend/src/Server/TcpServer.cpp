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

TcpServer::TcpServer(int port)
    : m_IsStarted(false), m_IsClientConnected(false), m_Port(port),
      m_ProtocolVersion(1) {};

TcpServer::~TcpServer() {
  if (m_IsStarted) {
    stopServer();
  }
}

void TcpServer::startServer() {
  auto &logger = utils::Logger::getInstance();

  // Create the context and acceptor
  m_Acceptor = std::make_unique<tcp::acceptor>(
      m_Context, tcp::endpoint(tcp::v4(), m_Port));
  logger.info("Server started on port " + std::to_string(m_Port));

  // Accept a connection
  m_IsStarted = true;
  while (m_IsStarted) {
    m_Status = TcpServerStatus::INITIALIZING;
    m_Socket = std::make_unique<tcp::socket>(m_Context);

    m_Acceptor->accept(*m_Socket);
    logger.info("Server connected to client");
    m_IsClientConnected = true;
    m_Socket->non_blocking(true);

    auto buffer = std::array<char, 8>();
    asio::error_code error;
    while (true) {
      // Lock the mutex during the time the command is answered
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      if (!m_IsStarted || !m_IsClientConnected) {
        break;
      }
      std::lock_guard<std::mutex> lock(m_Mutex);

      size_t byteRead = asio::read(*m_Socket, asio::buffer(buffer), error);
      if (error == asio::error::would_block) {
        // No data available at the moment
        continue;
      }

      // If there is any error, log it and break out of the loop
      if (byteRead == 0 || byteRead > 1024 || error) {
        logger.fatal("TCP read error: " + error.message());
        break;
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
      }

      if (!isSuccessful) {
        break;
      }
    }

    // If we get here, the server should continue running but the client has
    // disconnected, so shut the devices, close the socket to make sure and wait
    // for a new connection
    disconnectClient();
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
  if (m_Acceptor->is_open()) {
    m_Acceptor->close();
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

  // Wait a bit so if the connexion was already closed, we don't close it again

  if (m_Socket->is_open()) {
    m_Socket->close();
  }

  // Set the status to initializing
  m_Status = TcpServerStatus::INITIALIZING;
  m_IsClientConnected = false;
  logger.info("Client disconnected");
}

bool TcpServer::handleHandshake(TcpServerCommand command) {
  auto &logger = utils::Logger::getInstance();
  asio::error_code error;

  // The only valid command during initialization is the handshake
  if (command != TcpServerCommand::HANDSHAKE) {
    logger.fatal("Invalid command during initialization: " +
                 std::to_string(static_cast<std::uint32_t>(command)));

    asio::write(*m_Socket,
                asio::buffer(constructResponsePacket(TcpServerResponse::NOK)),
                error);
    return false;
  }

  // Respond OK to the handshake
  size_t byteWritten = asio::write(
      *m_Socket, asio::buffer(constructResponsePacket(TcpServerResponse::OK)),
      error);
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

  case TcpServerCommand::START_DATA_STREAMING:
    m_Devices.startDataStreaming();
    break;

  case TcpServerCommand::STOP_DATA_STREAMING:
    m_Devices.stopDataStreaming();
    break;

  case TcpServerCommand::PAUSE_RECORDING:
    logger.info("Pausing recording");
    m_Devices.pauseRecording();
    break;

  case TcpServerCommand::RESUME_RECORDING:
    logger.info("Resuming recording");
    m_Devices.resumeRecording();
    break;

  default:
    logger.fatal("Invalid command: " +
                 std::to_string(static_cast<std::uint32_t>(command)));
    response = TcpServerResponse::NOK;
    break;
  }

  // Respond OK to the command
  size_t byteWritten = asio::write(
      *m_Socket, asio::buffer(constructResponsePacket(response)), error);
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

  makeAndAddDevice(deviceName);
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

  m_Devices.remove(m_ConnectedDeviceIds[deviceName]);
  m_ConnectedDeviceIds.erase(deviceName);
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
