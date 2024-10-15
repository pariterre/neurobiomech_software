#include "Server/TcpClient.h"

#include "Utils/Logger.h"

using asio::ip::tcp;
using namespace STIMWALKER_NAMESPACE::server;

TcpClient::TcpClient(std::string host, int port)
    : m_Host(host), m_Port(port), m_IsConnected(false), m_CurrentVersion(1) {};

TcpClient::~TcpClient() {
  if (m_IsConnected) {
    disconnect();
  }
}

bool TcpClient::connect() {
  auto &logger = utils::Logger::getInstance();

  // Connect
  m_IsConnected = false;
  m_Socket = std::make_unique<tcp::socket>(m_Context);
  tcp::resolver resolver(m_Context);
  asio::connect(*m_Socket, resolver.resolve(m_Host, std::to_string(m_Port)));

  // Send the handshake
  if (!sendHandshake()) {
    logger.fatal("Handshake failed");
    m_Socket->close();
    return false;
  }

  // Read and handle the handshake response
  auto response = waitForResponse();
  if (response != TcpServerResponse::OK) {
    logger.fatal("Handshake failed");
    m_Socket->close();
    return false;
  }

  logger.info("Connected to server");
  m_IsConnected = true;
  return true;
}

bool TcpClient::disconnect() {
  m_IsConnected = false;
  return true;
}

TcpServerResponse TcpClient::waitForResponse() {
  auto buffer = std::array<char, 8>();
  asio::error_code error;
  size_t byteRead = asio::read(*m_Socket, asio::buffer(buffer), error);
  if (byteRead != 8 || error) {
    auto &logger = utils::Logger::getInstance();
    logger.fatal("TCP read error: " + error.message());
    return TcpServerResponse::NOK;
  }

  return parseResponsePacket(buffer);
}

bool TcpClient::sendHandshake() {
  auto &logger = utils::Logger::getInstance();

  asio::error_code error;
  size_t byteWritten = asio::write(
      *m_Socket, asio::buffer(constructPacket(TcpServerCommand::HANDSHAKE)),
      error);

  if (byteWritten != 8 || error) {
    logger.fatal("TCP write error: " + error.message());
    return false;
  }

  logger.info("Handshake sent to server");
  return true;
}

std::array<char, 8> TcpClient::constructPacket(TcpServerCommand command) {
  // Packets are exactly 8 bytes long, big-endian
  // - First 4 bytes are the version number
  // - Next 4 bytes are the command

  auto packet = std::array<char, 8>();
  packet.fill('\0');

  // Add the version number
  std::memcpy(packet.data(), &m_CurrentVersion, sizeof(std::uint32_t));

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
  if (version != m_CurrentVersion) {
    auto &logger = utils::Logger::getInstance();
    logger.fatal("Invalid version: " + std::to_string(version) +
                 ". Please "
                 "update the server to version " +
                 std::to_string(m_CurrentVersion));
    return TcpServerResponse::NOK;
  }

  // Get the response
  return static_cast<TcpServerResponse>(
      *reinterpret_cast<const std::uint32_t *>(buffer.data() + 4));
}