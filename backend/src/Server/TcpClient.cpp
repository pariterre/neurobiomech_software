#include "Server/TcpClient.h"

#include "Utils/Logger.h"

using asio::ip::tcp;
using namespace STIMWALKER_NAMESPACE;
using namespace STIMWALKER_NAMESPACE::server;

TcpClient::TcpClient(std::string host, int commandPort, int dataPort)
    : m_Host(host), m_CommandPort(commandPort), m_DataPort(dataPort),
      m_IsConnected(false), m_ProtocolVersion(1) {};

TcpClient::~TcpClient() {
  if (m_IsConnected) {
    disconnect();
  }
}

bool TcpClient::connect() {
  auto &logger = utils::Logger::getInstance();

  // Connect
  m_IsConnected = false;
  tcp::resolver resolver(m_Context);
  m_CommandSocket = std::make_unique<tcp::socket>(m_Context);
  asio::connect(*m_CommandSocket,
                resolver.resolve(m_Host, std::to_string(m_CommandPort)));
  m_DataSocket = std::make_unique<tcp::socket>(m_Context);
  asio::connect(*m_DataSocket,
                resolver.resolve(m_Host, std::to_string(m_DataPort)));
  m_IsConnected = true;

  // Send the handshake
  if (!sendCommandWithConfirmation(TcpServerCommand::HANDSHAKE)) {
    logger.fatal("Handshake failed");
    return false;
  }

  logger.info("Connected to server");
  return true;
}

bool TcpClient::disconnect() {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsConnected) {
    logger.warning("Client is not connected");
    return true;
  }

  m_CommandSocket->close();
  m_DataSocket->close();
  m_IsConnected = false;
  return true;
}

bool TcpClient::addDelsysDevice() {
  auto &logger = utils::Logger::getInstance();

  if (!sendCommandWithConfirmation(TcpServerCommand::CONNECT_DELSYS)) {
    logger.fatal("Failed to add Delsys device");
    return false;
  }

  logger.info("Delsys device added");
  return true;
}

bool TcpClient::addMagstimDevice() {
  auto &logger = utils::Logger::getInstance();

  if (!sendCommandWithConfirmation(TcpServerCommand::CONNECT_MAGSTIM)) {
    logger.fatal("Failed to add Magstim device");
    return false;
  }

  logger.info("Magstim device added");
  return true;
}

bool TcpClient::removeDelsysDevice() {
  auto &logger = utils::Logger::getInstance();

  if (!sendCommandWithConfirmation(TcpServerCommand::DISCONNECT_DELSYS)) {
    logger.fatal("Failed to remove Delsys device");
    return false;
  }

  logger.info("Delsys device removed");
  return true;
}

bool TcpClient::removeMagstimDevice() {
  auto &logger = utils::Logger::getInstance();

  if (!sendCommandWithConfirmation(TcpServerCommand::DISCONNECT_MAGSTIM)) {
    logger.fatal("Failed to remove Magstim device");
    return false;
  }

  logger.info("Magstim device removed");
  return true;
}

bool TcpClient::startRecording() {
  auto &logger = utils::Logger::getInstance();

  if (!sendCommandWithConfirmation(TcpServerCommand::START_RECORDING)) {
    logger.fatal("Failed to start recording");
    return false;
  }

  logger.info("Recording started");
  return true;
}

bool TcpClient::stopRecording() {
  auto &logger = utils::Logger::getInstance();

  if (!sendCommandWithConfirmation(TcpServerCommand::STOP_RECORDING)) {
    logger.fatal("Failed to stop recording");
    return false;
  }

  logger.info("Recording stopped");
  return true;
}

bool TcpClient::updateData() {
  auto &logger = utils::Logger::getInstance();
  if (!sendCommandWithConfirmation(TcpServerCommand::GET_DATA)) {
    logger.fatal("Failed to update the data");
    return false;
  }

  // TODO RENDU ICI!!
}

bool TcpClient::sendCommandWithConfirmation(TcpServerCommand command) {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsConnected) {
    logger.fatal("Client is not connected");
    return false;
  }

  asio::error_code error;
  size_t byteWritten = asio::write(
      *m_CommandSocket, asio::buffer(constructCommandPacket(command)), error);

  if (byteWritten != 8 || error) {
    logger.fatal("TCP write error: " + error.message());
    m_CommandSocket->close();
    m_DataSocket->close();
    m_IsConnected = false;
    return false;
  }

  auto response = waitForResponse();
  if (response != TcpServerResponse::OK) {
    logger.fatal("Failed to get confirmation for command: " +
                 std::to_string(static_cast<std::uint32_t>(command)));
    m_CommandSocket->close();
    m_DataSocket->close();
    m_IsConnected = false;
    return false;
  }

  return true;
}

TcpServerResponse TcpClient::waitForResponse() {
  auto buffer = std::array<char, 8>();
  asio::error_code error;
  size_t byteRead = asio::read(*m_CommandSocket, asio::buffer(buffer), error);
  if (byteRead != 8 || error) {
    auto &logger = utils::Logger::getInstance();
    logger.fatal("TCP read error: " + error.message());
    return TcpServerResponse::NOK;
  }

  return parseResponsePacket(buffer);
}

std::array<char, 8>
TcpClient::constructCommandPacket(TcpServerCommand command) {
  // Packets are exactly 8 bytes long, big-endian
  // - First 4 bytes are the version number
  // - Next 4 bytes are the command

  auto packet = std::array<char, 8>();
  packet.fill('\0');

  // Add the version number
  std::memcpy(packet.data(), &m_ProtocolVersion, sizeof(std::uint32_t));

  // Add the command
  std::memcpy(packet.data() + sizeof(std::uint32_t), &command,
              sizeof(TcpServerCommand));

  return packet;
}

TcpServerResponse
TcpClient::parseResponsePacket(const std::array<char, 8> &buffer) {
  // Packets are exactly 8 bytes long, big-endian
  // - First 4 bytes are the version number
  // - Next 4 bytes are the response

  // Check the version
  std::uint32_t version =
      *reinterpret_cast<const std::uint32_t *>(buffer.data());
  if (version != m_ProtocolVersion) {
    auto &logger = utils::Logger::getInstance();
    logger.fatal("Invalid version: " + std::to_string(version) +
                 ". Please "
                 "update the server to version " +
                 std::to_string(m_ProtocolVersion));
    return TcpServerResponse::NOK;
  }

  // Get the response
  return static_cast<TcpServerResponse>(
      *reinterpret_cast<const std::uint32_t *>(buffer.data() + 4));
}