#include "Server/TcpClient.h"

#include "Utils/Logger.h"

using asio::ip::tcp;
using namespace STIMWALKER_NAMESPACE;
using namespace STIMWALKER_NAMESPACE::server;

TcpClient::TcpClient(std::string host, int commandPort, int responsePort,
                     int liveDataPort)
    : m_Host(host), m_CommandPort(commandPort), m_ResponsePort(responsePort),
      m_LiveDataPort(liveDataPort), m_IsConnected(false),
      m_ProtocolVersion(1) {};

TcpClient::~TcpClient() {
  if (m_IsConnected) {
    disconnect();
  }
  if (m_LiveDataWorker.joinable()) {
    m_LiveDataWorker.join();
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

  m_ResponseSocket = std::make_unique<tcp::socket>(m_Context);
  asio::connect(*m_ResponseSocket,
                resolver.resolve(m_Host, std::to_string(m_ResponsePort)));

  m_LiveDataSocket = std::make_unique<tcp::socket>(m_Context);
  asio::connect(*m_LiveDataSocket,
                resolver.resolve(m_Host, std::to_string(m_LiveDataPort)));
  startUpdatingLiveData();

  m_IsConnected = true;

  // Send the handshake
  if (sendCommand(TcpServerCommand::HANDSHAKE) == TcpServerResponse::NOK) {
    logger.fatal("CLIENT: Handshake failed");
    disconnect();
    return false;
  }

  logger.info("CLIENT: Connected to server");
  return true;
}

bool TcpClient::disconnect() {
  m_IsConnected = false;
  if (m_LiveDataWorker.joinable()) {
    m_LiveDataWorker.join();
  }
  closeSockets();
  return true;
}

bool TcpClient::addDelsysAnalogDevice() {
  auto &logger = utils::Logger::getInstance();

  if (sendCommand(TcpServerCommand::CONNECT_DELSYS_ANALOG) ==
      TcpServerResponse::NOK) {
    logger.fatal("CLIENT: Failed to add Delsys Analog device");
    return false;
  }

  logger.info("CLIENT: Delsys Analog device added");
  return true;
}

bool TcpClient::addDelsysEmgDevice() {
  auto &logger = utils::Logger::getInstance();

  if (sendCommand(TcpServerCommand::CONNECT_DELSYS_EMG) ==
      TcpServerResponse::NOK) {
    logger.fatal("CLIENT: Failed to add Delsys EMG device");
    return false;
  }

  logger.info("CLIENT: Delsys EMG device added");
  return true;
}

bool TcpClient::addMagstimDevice() {
  auto &logger = utils::Logger::getInstance();

  if (sendCommand(TcpServerCommand::CONNECT_MAGSTIM) ==
      TcpServerResponse::NOK) {
    logger.fatal("CLIENT: Failed to add Magstim device");
    return false;
  }

  logger.info("CLIENT: Magstim device added");
  return true;
}

bool TcpClient::removeDelsysAnalogDevice() {
  auto &logger = utils::Logger::getInstance();

  if (sendCommand(TcpServerCommand::DISCONNECT_DELSYS_ANALOG) ==
      TcpServerResponse::NOK) {
    logger.fatal("CLIENT: Failed to remove Delsys EMG device");
    return false;
  }

  logger.info("CLIENT: Delsys EMG device removed");
  return true;
}

bool TcpClient::removeDelsysEmgDevice() {
  auto &logger = utils::Logger::getInstance();

  if (sendCommand(TcpServerCommand::DISCONNECT_DELSYS_EMG) ==
      TcpServerResponse::NOK) {
    logger.fatal("CLIENT: Failed to remove Delsys EMG device");
    return false;
  }

  logger.info("CLIENT: Delsys EMG device removed");
  return true;
}

bool TcpClient::removeMagstimDevice() {
  auto &logger = utils::Logger::getInstance();

  if (sendCommand(TcpServerCommand::DISCONNECT_MAGSTIM) ==
      TcpServerResponse::NOK) {
    logger.fatal("CLIENT: Failed to remove Magstim device");
    return false;
  }

  logger.info("CLIENT: Magstim device removed");
  return true;
}

bool TcpClient::startRecording() {
  auto &logger = utils::Logger::getInstance();

  if (sendCommand(TcpServerCommand::START_RECORDING) ==
      TcpServerResponse::NOK) {
    logger.fatal("CLIENT: Failed to start recording");
    return false;
  }

  logger.info("CLIENT: Recording started");
  return true;
}

bool TcpClient::stopRecording() {
  auto &logger = utils::Logger::getInstance();

  if (sendCommand(TcpServerCommand::STOP_RECORDING) == TcpServerResponse::NOK) {
    logger.fatal("CLIENT: Failed to stop recording");
    return false;
  }

  logger.info("CLIENT: Recording stopped");
  return true;
}

std::map<std::string, data::TimeSeries> TcpClient::getLastTrialData() {
  auto &logger = utils::Logger::getInstance();
  logger.info("CLIENT: Fetching the last trial data");

  std::vector<char> dataBuffer =
      sendCommandWithResponse(TcpServerCommand::GET_LAST_TRIAL_DATA);

  // Parse the data
  std::map<std::string, data::TimeSeries> data;
  try {
    data = devices::Devices::deserializeData(nlohmann::json::parse(dataBuffer));
  } catch (...) {
    logger.fatal("CLIENT: Failed to parse the last trial data");
    return std::map<std::string, data::TimeSeries>();
  }

  logger.info("CLIENT: Last trial data acquired");
  return data;
}

void TcpClient::startUpdatingLiveData() {
  m_LiveDataWorker = std::thread([this]() {
    while (m_IsConnected) {
      updateLiveData();
    }
  });
}

void TcpClient::updateLiveData() {
  auto &logger = utils::Logger::getInstance();

  auto dataBuffer = waitForResponse(*m_LiveDataSocket);
  std::map<std::string, data::TimeSeries> data;
  try {
    data = devices::Devices::deserializeData(nlohmann::json::parse(dataBuffer));
  } catch (...) {
    logger.fatal("CLIENT: Failed to parse the last trial data");
    return;
  }

  logger.debug("CLIENT: Live data received");
}

TcpServerResponse TcpClient::sendCommand(TcpServerCommand command) {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsConnected) {
    logger.fatal("CLIENT: Client is not connected");
    return TcpServerResponse::NOK;
  }

  asio::error_code error;
  size_t byteWritten = asio::write(
      *m_CommandSocket, asio::buffer(constructCommandPacket(command)), error);

  if (byteWritten != 8 || error) {
    logger.fatal("CLIENT: TCP write error: " + error.message());
    disconnect();
    return TcpServerResponse::NOK;
  }

  auto response = waitForCommandAcknowledgment();
  if (response == TcpServerResponse::NOK) {
    logger.warning("CLIENT: Failed to get confirmation for command: " +
                   std::to_string(static_cast<std::uint32_t>(command)));
  }
  return response;
}

std::vector<char> TcpClient::sendCommandWithResponse(TcpServerCommand command) {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsConnected) {
    logger.fatal("CLIENT: Client is not connected");
    return std::vector<char>();
  }

  asio::error_code error;
  size_t byteWritten = asio::write(
      *m_CommandSocket, asio::buffer(constructCommandPacket(command)), error);

  if (byteWritten != 8 || error) {
    logger.fatal("CLIENT: TCP write error: " + error.message());
    disconnect();
    return std::vector<char>();
  }

  auto data = waitForResponse(*m_ResponseSocket);
  auto response = waitForCommandAcknowledgment();
  if (response == TcpServerResponse::NOK) {
    logger.warning("CLIENT: Failed to get confirmation for command: " +
                   std::to_string(static_cast<std::uint32_t>(command)));
    return std::vector<char>();
  }
  return data;
}

TcpServerResponse TcpClient::waitForCommandAcknowledgment() {
  auto buffer = std::array<char, 8>();
  asio::error_code error;
  size_t byteRead = asio::read(*m_CommandSocket, asio::buffer(buffer), error);
  if (byteRead != 8 || error) {
    auto &logger = utils::Logger::getInstance();
    logger.fatal("CLIENT: TCP read error: " + error.message());
    return TcpServerResponse::NOK;
  }

  return parseAcknowledgmentPacket(buffer);
}

std::vector<char> TcpClient::waitForResponse(asio::ip::tcp::socket &socket) {
  auto buffer = std::array<char, 8>();
  asio::error_code error;
  size_t byteRead = asio::read(socket, asio::buffer(buffer), error);
  if (byteRead != 8 || error) {
    auto &logger = utils::Logger::getInstance();
    logger.fatal("CLIENT: TCP read error: " + error.message());
    return std::vector<char>();
  }

  std::uint32_t totalByteCount =
      static_cast<std::uint32_t>(parseAcknowledgmentPacket(buffer));
  std::vector<char> dataBuffer(totalByteCount);
  byteRead = asio::read(socket, asio::buffer(dataBuffer), error);
  if (byteRead != totalByteCount || error) {
    utils::Logger::getInstance().fatal(
        "CLIENT: Failed to fetch the last trial data");
    return std::vector<char>();
  }
  return dataBuffer;
}

void TcpClient::closeSockets() {
  if (m_CommandSocket && m_CommandSocket->is_open()) {
    m_CommandSocket->close();
  }
  if (m_ResponseSocket && m_ResponseSocket->is_open()) {
    m_ResponseSocket->close();
  }
  if (m_LiveDataSocket && m_LiveDataSocket->is_open()) {
    m_LiveDataSocket->close();
  }
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
TcpClient::parseAcknowledgmentPacket(const std::array<char, 8> &buffer) {
  // Packets are exactly 8 bytes long, big-endian
  // - First 4 bytes are the version number
  // - Next 4 bytes are the response

  // Check the version
  std::uint32_t version =
      *reinterpret_cast<const std::uint32_t *>(buffer.data());
  if (version != m_ProtocolVersion) {
    auto &logger = utils::Logger::getInstance();
    logger.fatal("CLIENT: Invalid version: " + std::to_string(version) +
                 ". Please "
                 "update the server to version " +
                 std::to_string(m_ProtocolVersion));
    return TcpServerResponse::NOK;
  }

  // Get the response
  return *reinterpret_cast<const TcpServerResponse *>(buffer.data() + 4);
}