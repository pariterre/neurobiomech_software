#include "Server/TcpClient.h"

#include "Utils/Logger.h"

#if defined(_WIN32) // Windows-specific handling
// Windows is little-endian by default
#define le32toh(x) (x)
#define htole32(x) (x)

#elif defined(__APPLE__) // macOS
#include <libkern/OSByteOrder.h>
#define htole32(x) OSSwapHostToLittleInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

#elif defined(__linux__) // Linux
#include <endian.h>      // Provides le32toh() and htole32()

#else
#error "Unsupported platform"
#endif

using asio::ip::tcp;
using namespace NEUROBIO_NAMESPACE;
using namespace NEUROBIO_NAMESPACE::server;

TcpClient::TcpClient(std::string host, int commandPort, int responsePort,
                     int liveDataPort, int liveAnalysesPort)
    : m_Host(host), m_CommandPort(commandPort), m_ResponsePort(responsePort),
      m_LiveDataPort(liveDataPort), m_LiveAnalysesPort(liveAnalysesPort),
      m_IsConnected(false),
      m_ProtocolVersion(COMMUNICATION_PROTOCOL_VERSION) {};

TcpClient::~TcpClient() {
  if (m_IsConnected) {
    disconnect();
  }
  if (m_LiveDataWorker.joinable()) {
    m_LiveDataWorker.join();
  }
  if (m_LiveAnalysesWorker.joinable()) {
    m_LiveAnalysesWorker.join();
  }
}

bool TcpClient::connect(std::uint32_t stateId) {
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

  m_ResponseSocket = std::make_unique<tcp::socket>(m_Context);
  auto responseHasReturned = std::make_shared<bool>(false);
  asio::async_connect(
      *m_ResponseSocket,
      resolver.resolve(m_Host, std::to_string(m_ResponsePort)),
      [this, stateId, responseHasReturned](const asio::error_code &ec,
                                           const tcp::endpoint &) {
        *responseHasReturned = true;
        if (ec) {
          return;
        }
        asio::write(*m_ResponseSocket,
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

  while (!*commandHasReturned || !*responseHasReturned ||
         !*liveDataHasReturned || !*liveAnalysesHasReturned) {
    m_Context.run_one();
  }
  m_IsConnected = true;

  // Wait for the sockets to be connected
  m_ContextWorker = std::thread([this]() { m_Context.run(); });

  // Response socket should always listen to the server
  m_ResponseWorker = std::thread([this]() {
    while (m_IsConnected) {
      // For now, we do nothing with this, but it can be used to react from a
      // change in states of the server
      m_PreviousAck = waitForMessage(*m_ResponseSocket);
      switch (m_PreviousAck) {
      case TcpServerResponse::OK:
      case TcpServerResponse::NOK:
      case TcpServerResponse::STATES_CHANGED:
        // Nothing more to do
        break;
      default:
        // If it is anything but the normal response, it is the size of a larger
        // message, so we wait for the next message
        m_PreviousResponse = waitForResponse(
            *m_ResponseSocket, static_cast<std::uint32_t>(m_PreviousAck));
      }
    }
  });

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

  closeSockets();
  if (m_LiveDataWorker.joinable()) {
    m_LiveDataWorker.join();
  }
  if (m_LiveAnalysesWorker.joinable()) {
    m_LiveAnalysesWorker.join();
  }
  m_ResponseWorker.join();

  m_Context.stop();
  m_ContextWorker.join();
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

bool TcpClient::addAnalyzer(const nlohmann::json &analyzer) {
  auto &logger = utils::Logger::getInstance();

  if (sendCommandWithData(TcpServerCommand::ADD_ANALYZER, analyzer) ==
      TcpServerResponse::NOK) {
    logger.fatal("CLIENT: Failed to add the analyzer");
    return false;
  }

  logger.info("CLIENT: Analyzer added");
  return true;
}

bool TcpClient::removeAnalyzer(const std::string &analyzerName) {
  auto &logger = utils::Logger::getInstance();

  if (sendCommandWithData(TcpServerCommand::REMOVE_ANALYZER,
                          {{"analyzer", analyzerName}}) ==
      TcpServerResponse::NOK) {
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

  auto ack = waitForMessage(*m_LiveDataSocket);
  auto dataBuffer =
      waitForResponse(*m_LiveDataSocket, static_cast<std::uint32_t>(ack));
  std::map<std::string, data::TimeSeries> data;
  try {
    data = devices::Devices::deserializeData(nlohmann::json::parse(dataBuffer));
  } catch (...) {
    logger.fatal("CLIENT: Failed to parse the last trial data");
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
    auto ack = waitForMessage(*m_LiveAnalysesSocket);
    auto dataBuffer =
        waitForResponse(*m_LiveAnalysesSocket, static_cast<std::uint32_t>(ack));
    auto serialized = nlohmann::json::parse(dataBuffer);

    auto data = analyzer::Predictions(serialized);
  } catch (...) {
    logger.fatal("CLIENT: Failed to parse the last trial data");
    return;
  }

  logger.debug("CLIENT: Live analyze received");
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

  if (byteWritten != BYTES_IN_CLIENT_PACKET_HEADER || error) {
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

TcpServerResponse TcpClient::sendCommandWithData(TcpServerCommand command,
                                                 const nlohmann::json &data) {
  auto &logger = utils::Logger::getInstance();

  // First send the command as usual
  auto response = sendCommand(command);
  if (response == TcpServerResponse::NOK) {
    return TcpServerResponse::NOK;
  }

  // If the command was successful, send the length of the data
  asio::error_code error;
  size_t byteWritten =
      asio::write(*m_ResponseSocket,
                  asio::buffer(constructCommandPacket(
                      static_cast<TcpServerCommand>(data.dump().size()))),
                  error);

  if (byteWritten != BYTES_IN_CLIENT_PACKET_HEADER || error) {
    logger.fatal("CLIENT: TCP write error: " + error.message());
    disconnect();
    return TcpServerResponse::NOK;
  }

  // Send the data
  byteWritten =
      asio::write(*m_ResponseSocket, asio::buffer(data.dump()), error);
  if (byteWritten != data.dump().size() || error) {
    logger.fatal("CLIENT: TCP write error: " + error.message());
    disconnect();
    return TcpServerResponse::NOK;
  }

  return waitForCommandAcknowledgment();
}

std::vector<char> TcpClient::sendCommandWithResponse(TcpServerCommand command) {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsConnected) {
    logger.fatal("CLIENT: Client is not connected");
    return std::vector<char>();
  }

  m_PreviousResponse.clear();

  asio::error_code error;
  size_t byteWritten = asio::write(
      *m_CommandSocket, asio::buffer(constructCommandPacket(command)), error);

  if (byteWritten != BYTES_IN_CLIENT_PACKET_HEADER || error) {
    logger.fatal("CLIENT: TCP write error: " + error.message());
    disconnect();
    return std::vector<char>();
  }

  auto ack = waitForCommandAcknowledgment();
  if (ack == TcpServerResponse::NOK) {
    logger.warning("CLIENT: Failed to get confirmation for command: " +
                   std::to_string(static_cast<std::uint32_t>(command)));
    return std::vector<char>();
  }

  while (m_PreviousResponse.empty()) {
    m_Context.run_one();
  }
  return m_PreviousResponse;
}

TcpServerResponse TcpClient::waitForCommandAcknowledgment() {
  auto buffer = std::array<char, BYTES_IN_SERVER_PACKET_HEADER>();
  asio::error_code error;
  size_t byteRead = asio::read(*m_CommandSocket, asio::buffer(buffer), error);
  if (byteRead != BYTES_IN_SERVER_PACKET_HEADER || error) {
    auto &logger = utils::Logger::getInstance();
    logger.fatal("CLIENT: TCP read error: " + error.message());
    return TcpServerResponse::NOK;
  }

  return parseAcknowledgmentFromPacket(buffer);
}

TcpServerResponse TcpClient::waitForMessage(asio::ip::tcp::socket &socket) {
  auto buffer = std::array<char, BYTES_IN_SERVER_PACKET_HEADER>();
  asio::error_code error;
  size_t byteRead = asio::read(socket, asio::buffer(buffer), error);
  if (byteRead != BYTES_IN_SERVER_PACKET_HEADER || error) {
    auto &logger = utils::Logger::getInstance();
    logger.fatal("CLIENT: TCP read error: " + error.message());
    return TcpServerResponse::NOK;
  }

  return parseAcknowledgmentFromPacket(buffer);
}

std::vector<char> TcpClient::waitForResponse(asio::ip::tcp::socket &socket,
                                             std::uint32_t expectedSize) {
  std::vector<char> dataBuffer(expectedSize);
  asio::error_code error;
  auto byteRead = asio::read(socket, asio::buffer(dataBuffer), error);
  if (byteRead != expectedSize || error) {
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
  std::uint32_t versionLittleEndian = htole32(m_ProtocolVersion);
  std::memcpy(packet.data(), &versionLittleEndian, sizeof(versionLittleEndian));

  // Add the command
  std::uint32_t commandLittleEndian =
      htole32(static_cast<std::uint32_t>(command));
  std::memcpy(packet.data() + sizeof(versionLittleEndian), &commandLittleEndian,
              sizeof(commandLittleEndian));

  return packet;
}

TcpServerResponse TcpClient::parseAcknowledgmentFromPacket(
    const std::array<char, BYTES_IN_SERVER_PACKET_HEADER> &buffer) {
  // Packets are exactly 16 bytes long, little-endian
  // - First 4 bytes are the version number
  // - The next 8 bytes are the timestamp of the packet (milliseconds since
  // epoch)
  // - Next 4 bytes are the response

  // Check the version
  std::uint32_t version;
  std::memcpy(&version, buffer.data(), sizeof(version));
  version = le32toh(version);

  if (version != m_ProtocolVersion) {
    auto &logger = utils::Logger::getInstance();
    logger.fatal("CLIENT: Invalid version: " + std::to_string(version) +
                 ". Please "
                 "update the server to version " +
                 std::to_string(m_ProtocolVersion));
    return TcpServerResponse::NOK;
  }

  // Skip the timestamp (8 bytes)
  std::uint64_t timestamp;

  // Get the response
  std::uint32_t response;
  std::memcpy(&response, buffer.data() + sizeof(version) + sizeof(timestamp),
              sizeof(response));
  response = le32toh(response);

  return static_cast<TcpServerResponse>(response);
}

std::chrono::system_clock::time_point TcpClient::parseTimeStampFromPacket(
    const std::array<char, BYTES_IN_SERVER_PACKET_HEADER> &buffer) {
  // Packets are exactly 16 bytes long, little-endian
  // - First 4 bytes are the version number
  // - The next 8 bytes are the timestamp of the packet (milliseconds since
  // epoch)
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
    return std::chrono::system_clock::time_point(std::chrono::milliseconds(0));
  }

  // Get the response
  auto timestamp = *reinterpret_cast<const std::uint64_t *>(buffer.data() + 4);
  return std::chrono::system_clock::time_point(
      std::chrono::milliseconds(timestamp));
}