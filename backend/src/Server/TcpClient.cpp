#include "Server/TcpClient.h"

#include "Utils/Logger.h"

#if defined(_WIN32) // Windows-specific handling
// Windows is little-endian by default
#define le32toh(x) (x)
#define htole32(x) (x)
#define le64toh(x) (x)
#define htole64(x) (x)

#elif defined(__APPLE__) // macOS
#include <libkern/OSByteOrder.h>
#define htole32(x) OSSwapHostToLittleInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#elif defined(__linux__) // Linux
#include <endian.h>      // Provides le32toh() and htole32()

#else
#error "Unsupported platform"
#endif

using asio::ip::tcp;
using namespace NEUROBIO_NAMESPACE;
using namespace NEUROBIO_NAMESPACE::server;

// Packets are at least 24 bytes long, little-endian
// - First 4 bytes are the version number
// - Next 4 bytes are the command the server responds to
// - Next 4 bytes are the message
// - Next 4 bytes are the data type (if any)
// - Remaining 8 mandatory bytes are the timestamp of the packet
//   (milliseconds since epoch)
// - If DATA_TYPE is not NONE, then the next 8 bytes are the data length to
// follow
//   (in bytes)
// - Finally, all the remaining bytes are the data themselves of length send
// by the previous 8 bytes

bool ServerResponse::checkVersionFromPacket(const std::vector<char> &buffer) {

  // Check the version
  uint32_t version;
  std::memcpy(&version, buffer.data(), sizeof(version));
  version = le32toh(version);

  if (version != COMMUNICATION_PROTOCOL_VERSION) {
    auto &logger = utils::Logger::getInstance();
    logger.fatal("CLIENT: Invalid version: " + std::to_string(version) +
                 ". Please "
                 "update the server to version " +
                 std::to_string(COMMUNICATION_PROTOCOL_VERSION));
    return false;
  }

  return true;
}

TcpServerCommand
ServerResponse::parseCommandFromPacket(const std::vector<char> &buffer) {

  if (!checkVersionFromPacket(buffer)) {
    return TcpServerCommand::NONE;
  }

  // Elements to skip to get to the message
  uint32_t version;

  // Get the response
  uint32_t command;
  std::memcpy(&command, buffer.data() + sizeof(version), sizeof(command));
  return static_cast<TcpServerCommand>(le32toh(command));
}

TcpServerMessage
ServerResponse::parseMessageFromPacket(const std::vector<char> &buffer) {

  if (!checkVersionFromPacket(buffer)) {
    return TcpServerMessage::NOK;
  }

  // Elements to skip to get to the message
  uint32_t version;
  uint32_t command;

  // Get the message
  uint32_t message;
  std::memcpy(&message, buffer.data() + sizeof(version) + sizeof(command),
              sizeof(message));
  return static_cast<TcpServerMessage>(le32toh(message));
}

TcpServerDataType
ServerResponse::parseDataTypeFromPacket(const std::vector<char> &buffer) {
  if (!checkVersionFromPacket(buffer)) {
    return TcpServerDataType::NONE;
  }

  // Elements to skip to get to the message
  uint32_t version;
  uint32_t command;
  uint32_t message;

  uint32_t dataType;
  std::memcpy(&dataType,
              buffer.data() + sizeof(version) + sizeof(command) +
                  sizeof(message),
              sizeof(dataType));
  return static_cast<TcpServerDataType>(le32toh(dataType));
}

ServerResponse::ServerResponse()
    : m_HasReceivedData(false), m_Command(TcpServerCommand::NONE),
      m_Message(TcpServerMessage::NOK), m_DataType(TcpServerDataType::NONE),
      m_Timestamp(
          std::chrono::system_clock::time_point(std::chrono::milliseconds(0))) {
}

ServerResponse::ServerResponse(tcp::socket &socket, std::shared_mutex &mutex)
    : m_HasReceivedData(false), m_Command(TcpServerCommand::NONE),
      m_Message(TcpServerMessage::NOK), m_DataType(TcpServerDataType::NONE),
      m_Timestamp(
          std::chrono::system_clock::time_point(std::chrono::milliseconds(0))) {

  std::vector<char> header = readHeaderFromSocket(socket);
  if (header.empty()) {
    return;
  }
  auto dataType = parseDataTypeFromPacket(header);
  auto data = readDataFromSocket(socket, dataType);

  std::unique_lock<std::shared_mutex> lock(mutex);
  m_HasReceivedData = true;
  m_Command = parseCommandFromPacket(header);
  m_Message = parseMessageFromPacket(header);
  m_DataType = dataType;
  m_Timestamp = parseTimeStampFromPacket(header);
  m_Data = std::move(data);
}

std::chrono::system_clock::time_point
ServerResponse::parseTimeStampFromPacket(const std::vector<char> &buffer) {
  if (!checkVersionFromPacket(buffer)) {
    return std::chrono::system_clock::time_point(std::chrono::milliseconds(0));
  }

  // Elements to skip to get to the message
  uint32_t version;
  uint32_t command;
  uint32_t message;
  uint32_t dataType;

  uint64_t timestamp;
  std::memcpy(&timestamp,
              buffer.data() + sizeof(version) + sizeof(command) +
                  sizeof(message) + sizeof(dataType),
              sizeof(timestamp));
  timestamp = le64toh(timestamp);

  return std::chrono::system_clock::time_point(
      std::chrono::milliseconds(timestamp));
}

std::vector<char> ServerResponse::readHeaderFromSocket(tcp::socket &socket) {
  std::vector<char> headerBuffer(BYTES_IN_SERVER_PACKET_HEADER);
  asio::error_code error;
  auto byteRead = asio::read(socket, asio::buffer(headerBuffer), error);
  if (byteRead != BYTES_IN_SERVER_PACKET_HEADER || error) {
    return {};
  }
  return headerBuffer;
}

std::vector<char> ServerResponse::readDataFromSocket(tcp::socket &socket,
                                                     TcpServerDataType type) {
  if (type == TcpServerDataType::NONE) {
    return {}; // No data to read
  }

  std::vector<char> dataBuffer(sizeof(uint64_t));
  asio::error_code error;
  auto byteRead = asio::read(socket, asio::buffer(dataBuffer), error);
  if (byteRead != 8 || error) {
    return {};
  }

  auto dataSize = le64toh(*reinterpret_cast<int64_t *>(dataBuffer.data()));
  if (dataSize <= 0) {
    return {};
  }

  dataBuffer.resize(dataSize);
  byteRead = asio::read(socket, asio::buffer(dataBuffer), error);
  if (byteRead != dataSize || error) {
    utils::Logger::getInstance().fatal(
        "CLIENT: Failed to read data from socket. Expected: " +
        std::to_string(dataSize) + ", got: " + std::to_string(byteRead));
    return {};
  }
  return dataBuffer;
}

TcpClient::TcpClient(std::string host, int commandPort, int messagePort,
                     int liveDataPort, int liveAnalysesPort)
    : m_Host(host), m_CommandPort(commandPort), m_MessagePort(messagePort),
      m_LiveDataPort(liveDataPort), m_LiveAnalysesPort(liveAnalysesPort),
      m_IsConnected(false) {};

TcpClient::~TcpClient() {
  if (m_IsConnected) {
    disconnect();
  }
}

bool TcpClient::connect(uint32_t stateId) {
  auto &logger = utils::Logger::getInstance();

  // Connect
  m_IsConnected = false;
  tcp::resolver resolver(m_Context);

  m_CommandSocket = std::make_unique<tcp::socket>(m_Context);
  auto commandHasReturned = std::make_shared<bool>(false);
  asio::async_connect(
      *m_CommandSocket, resolver.resolve(m_Host, std::to_string(m_CommandPort)),
      [this, stateId, commandHasReturned](const asio::error_code &ec,
                                          const tcp::endpoint &) {
        *commandHasReturned = true;
        if (ec) {
          return;
        }
        asio::write(*m_CommandSocket,
                    asio::buffer(constructCommandPacket(
                        static_cast<TcpServerCommand>(stateId))));
      });

  m_MessageSocket = std::make_unique<tcp::socket>(m_Context);
  auto messageHasReturned = std::make_shared<bool>(false);
  asio::async_connect(
      *m_MessageSocket, resolver.resolve(m_Host, std::to_string(m_MessagePort)),
      [this, stateId, messageHasReturned](const asio::error_code &ec,
                                          const tcp::endpoint &) {
        *messageHasReturned = true;
        if (ec) {
          return;
        }
        asio::write(*m_MessageSocket,
                    asio::buffer(constructCommandPacket(
                        static_cast<TcpServerCommand>(stateId))));
      });

  m_LiveDataSocket = std::make_unique<tcp::socket>(m_Context);
  auto liveDataHasReturned = std::make_shared<bool>(false);
  asio::async_connect(
      *m_LiveDataSocket,
      resolver.resolve(m_Host, std::to_string(m_LiveDataPort)),
      [this, stateId, liveDataHasReturned](const asio::error_code &ec,
                                           const tcp::endpoint &) {
        *liveDataHasReturned = true;
        if (ec) {
          return;
        }
        asio::write(*m_LiveDataSocket,
                    asio::buffer(constructCommandPacket(
                        static_cast<TcpServerCommand>(stateId))));
        startUpdatingLiveData();
      });

  m_LiveAnalysesSocket = std::make_unique<tcp::socket>(m_Context);
  auto liveAnalysesHasReturned = std::make_shared<bool>(false);
  asio::async_connect(
      *m_LiveAnalysesSocket,
      resolver.resolve(m_Host, std::to_string(m_LiveAnalysesPort)),
      [this, stateId, liveAnalysesHasReturned](const asio::error_code &ec,
                                               const tcp::endpoint &) {
        *liveAnalysesHasReturned = true;
        if (ec) {
          return;
        }
        asio::write(*m_LiveAnalysesSocket,
                    asio::buffer(constructCommandPacket(
                        static_cast<TcpServerCommand>(stateId))));
        startUpdatingLiveAnalyses();
      });

  while (!*commandHasReturned || !*messageHasReturned ||
         !*liveDataHasReturned || !*liveAnalysesHasReturned) {
    m_Context.run_one();
  }
  m_IsConnected = true;

  // Wait for the sockets to be connected
  m_ContextWorker = std::thread([this]() { m_Context.run(); });

  // Message socket should always listen to the server
  m_MessageWorker = std::thread([this]() {
    while (m_IsConnected) {
      // For now, we do nothing with this, but it can be used to react from a
      // change in states of the server
      m_PreviousMessage =
          ServerResponse(*m_MessageSocket, m_PreviousMessageMutex);
      m_HasPreviousMessage = m_PreviousMessage.getHasReceivedData();
    }
  });

  // Send the handshake
  auto response = sendCommand(TcpServerCommand::HANDSHAKE);
  if (response.getMessage() == TcpServerMessage::NOK) {
    logger.fatal("CLIENT: Handshake failed");
    disconnect();
    return false;
  }

  logger.info("CLIENT: Connected to server");
  return true;
}

bool TcpClient::disconnect() {
  m_IsConnected = false;

  closeSockets();
  if (m_LiveDataWorker.joinable()) {
    m_LiveDataWorker.join();
  }
  if (m_LiveAnalysesWorker.joinable()) {
    m_LiveAnalysesWorker.join();
  }
  m_MessageWorker.join();

  m_Context.stop();
  m_ContextWorker.join();
  return true;
}

bool TcpClient::addDelsysAnalogDevice() {
  auto &logger = utils::Logger::getInstance();

  auto response = sendCommand(TcpServerCommand::CONNECT_DELSYS_ANALOG);
  if (response.getMessage() == TcpServerMessage::NOK) {
    logger.fatal("CLIENT: Failed to add Delsys Analog device");
    return false;
  }

  logger.info("CLIENT: Delsys Analog device added");
  return true;
}

bool TcpClient::addDelsysEmgDevice() {
  auto &logger = utils::Logger::getInstance();

  auto response = sendCommand(TcpServerCommand::CONNECT_DELSYS_EMG);
  if (response.getMessage() == TcpServerMessage::NOK) {
    logger.fatal("CLIENT: Failed to add Delsys EMG device");
    return false;
  }

  logger.info("CLIENT: Delsys EMG device added");

  return true;
}

bool TcpClient::addMagstimDevice() {
  auto &logger = utils::Logger::getInstance();

  auto response = sendCommand(TcpServerCommand::CONNECT_MAGSTIM);
  if (response.getMessage() == TcpServerMessage::NOK) {
    logger.fatal("CLIENT: Failed to add Magstim device");
    return false;
  }

  logger.info("CLIENT: Magstim device added");
  return true;
}

bool TcpClient::removeDelsysAnalogDevice() {
  auto &logger = utils::Logger::getInstance();

  auto response = sendCommand(TcpServerCommand::DISCONNECT_DELSYS_ANALOG);
  if (response.getMessage() == TcpServerMessage::NOK) {
    logger.fatal("CLIENT: Failed to remove Delsys EMG device");
    return false;
  }

  logger.info("CLIENT: Delsys EMG device removed");
  return true;
}

bool TcpClient::removeDelsysEmgDevice() {
  auto &logger = utils::Logger::getInstance();

  auto response = sendCommand(TcpServerCommand::DISCONNECT_DELSYS_EMG);
  if (response.getMessage() == TcpServerMessage::NOK) {
    logger.fatal("CLIENT: Failed to remove Delsys EMG device");
    return false;
  }

  logger.info("CLIENT: Delsys EMG device removed");
  return true;
}

bool TcpClient::removeMagstimDevice() {
  auto &logger = utils::Logger::getInstance();

  auto response = sendCommand(TcpServerCommand::DISCONNECT_MAGSTIM);
  if (response.getMessage() == TcpServerMessage::NOK) {
    logger.fatal("CLIENT: Failed to remove Magstim device");
    return false;
  }

  logger.info("CLIENT: Magstim device removed");
  return true;
}

bool TcpClient::startRecording() {
  auto &logger = utils::Logger::getInstance();

  auto response = sendCommand(TcpServerCommand::START_RECORDING);
  if (response.getMessage() == TcpServerMessage::NOK) {
    logger.fatal("CLIENT: Failed to start recording");
    return false;
  }

  logger.info("CLIENT: Recording started");
  return true;
}

bool TcpClient::stopRecording() {
  auto &logger = utils::Logger::getInstance();

  auto response = sendCommand(TcpServerCommand::STOP_RECORDING);
  if (response.getMessage() == TcpServerMessage::NOK) {
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

bool TcpClient::addAnalyzer(const nlohmann::json &analyzer) {
  auto &logger = utils::Logger::getInstance();

  auto response = sendCommandWithData(TcpServerCommand::ADD_ANALYZER, analyzer);
  if (response.getMessage() == TcpServerMessage::NOK) {
    logger.fatal("CLIENT: Failed to add the analyzer");
    return false;
  }

  logger.info("CLIENT: Analyzer added");
  return true;
}

bool TcpClient::removeAnalyzer(const std::string &analyzerName) {
  auto &logger = utils::Logger::getInstance();

  auto response = sendCommandWithData(TcpServerCommand::REMOVE_ANALYZER,
                                      {{"analyzer", analyzerName}});
  if (response.getMessage() == TcpServerMessage::NOK) {
    logger.fatal("CLIENT: Failed to remove the analyzer");
    return false;
  }

  logger.info("CLIENT: Analyzer " + analyzerName + " removed");
  return true;
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

  auto mutex = std::shared_mutex();
  auto response = ServerResponse(*m_LiveDataSocket, mutex);
  if (!response.getHasReceivedData()) {
    // If no data received, just return
    return;
  }

  std::map<std::string, data::TimeSeries> data;
  try {
    data = devices::Devices::deserializeData(
        nlohmann::json::parse(response.getData()));
  } catch (...) {
    logger.fatal("CLIENT: Failed to parse the live trial data");
    return;
  }

  logger.debug("CLIENT: Live data received");
}

void TcpClient::startUpdatingLiveAnalyses() {
  m_LiveAnalysesWorker = std::thread([this]() {
    while (m_IsConnected) {
      updateLiveAnalyses();
    }
  });
}

void TcpClient::updateLiveAnalyses() {
  auto &logger = utils::Logger::getInstance();

  try {
    auto mutex = std::shared_mutex();
    auto response = ServerResponse(*m_LiveAnalysesSocket, mutex);
    if (!response.getHasReceivedData()) {
      // If no data received, just return
      return;
    }
    auto serialized = nlohmann::json::parse(response.getData());

    auto data = analyzer::Predictions(serialized);
  } catch (...) {
    logger.fatal("CLIENT: Failed to parse the last analyses");
    return;
  }

  logger.debug("CLIENT: Live analyze received");
}

ServerResponse TcpClient::sendCommand(TcpServerCommand command) {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsConnected) {
    logger.fatal("CLIENT: Client is not connected");
    return ServerResponse();
  }

  asio::error_code error;
  size_t byteWritten = asio::write(
      *m_CommandSocket, asio::buffer(constructCommandPacket(command)), error);

  if (byteWritten != BYTES_IN_CLIENT_PACKET_HEADER || error) {
    logger.fatal("CLIENT: TCP write error: " + error.message());
    disconnect();
    return ServerResponse();
  }

  ServerResponse response;
  do {
    auto mutex = std::shared_mutex();
    response = ServerResponse(*m_CommandSocket, mutex);
  } while (m_IsConnected && !response.getHasReceivedData());

  if (response.getMessage() == TcpServerMessage::NOK) {
    logger.warning("CLIENT: Failed to get confirmation for command: " +
                   std::to_string(static_cast<uint32_t>(command)));
  }
  return response;
}

ServerResponse TcpClient::sendCommandWithData(TcpServerCommand command,
                                              const nlohmann::json &data) {
  auto &logger = utils::Logger::getInstance();

  // First send the command as usual
  auto response = sendCommand(command);
  if (response.getMessage() == TcpServerMessage::NOK) {
    return response;
  }

  // If the command was successful, send the length of the data
  asio::error_code error;
  size_t byteWritten =
      asio::write(*m_MessageSocket,
                  asio::buffer(constructCommandPacket(
                      static_cast<TcpServerCommand>(data.dump().size()))),
                  error);

  if (byteWritten != BYTES_IN_CLIENT_PACKET_HEADER || error) {
    logger.fatal("CLIENT: TCP write error: " + error.message());
    disconnect();
    return ServerResponse();
  }

  // Send the data
  m_HasPreviousMessage = false;
  byteWritten = asio::write(*m_MessageSocket, asio::buffer(data.dump()), error);
  if (byteWritten != data.dump().size() || error) {
    logger.fatal("CLIENT: TCP write error: " + error.message());
    disconnect();
    return ServerResponse();
  }

  // Wait for the acknowledgment from the server
  while (true) {
    m_Context.run_one();
    std::shared_lock lock(m_PreviousMessageMutex);
    if (m_HasPreviousMessage) {
      m_HasPreviousMessage = false;
      if (m_PreviousMessage.getCommand() == command) {
        return m_PreviousMessage;
      }
    }
  }
}

std::vector<char> TcpClient::sendCommandWithResponse(TcpServerCommand command) {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsConnected) {
    logger.fatal("CLIENT: Client is not connected");
    return std::vector<char>();
  }

  m_HasPreviousMessage = false;
  asio::error_code error;
  size_t byteWritten = asio::write(
      *m_CommandSocket, asio::buffer(constructCommandPacket(command)), error);

  if (byteWritten != BYTES_IN_CLIENT_PACKET_HEADER || error) {
    logger.fatal("CLIENT: TCP write error: " + error.message());
    disconnect();
    return std::vector<char>();
  }

  // Wait for the response from the server
  while (true) {
    m_Context.run_one();
    std::shared_lock lock(m_PreviousMessageMutex);
    if (m_HasPreviousMessage) {
      m_HasPreviousMessage = false;
      if (m_PreviousMessage.getCommand() == command) {
        return m_PreviousMessage.getData();
      }
    }
  }
}

void TcpClient::closeSockets() {
  if (m_CommandSocket && m_CommandSocket->is_open()) {
    m_CommandSocket->close();
  }
  if (m_MessageSocket && m_MessageSocket->is_open()) {
    m_MessageSocket->close();
  }
  if (m_LiveDataSocket && m_LiveDataSocket->is_open()) {
    m_LiveDataSocket->close();
  }
  if (m_LiveAnalysesSocket && m_LiveAnalysesSocket->is_open()) {
    m_LiveAnalysesSocket->close();
  }
}

std::array<char, BYTES_IN_CLIENT_PACKET_HEADER>
TcpClient::constructCommandPacket(TcpServerCommand command) {
  // Packets are exactly 8 bytes long, little-endian
  // - First 4 bytes are the version number
  // - Next 4 bytes are the command

  auto packet = std::array<char, BYTES_IN_CLIENT_PACKET_HEADER>();
  packet.fill('\0');

  // Add the version number
  uint32_t versionLittleEndian = htole32(COMMUNICATION_PROTOCOL_VERSION);
  std::memcpy(packet.data(), &versionLittleEndian, sizeof(versionLittleEndian));

  // Add the command
  uint32_t commandLittleEndian = htole32(static_cast<uint32_t>(command));
  std::memcpy(packet.data() + sizeof(versionLittleEndian), &commandLittleEndian,
              sizeof(commandLittleEndian));

  return packet;
}
