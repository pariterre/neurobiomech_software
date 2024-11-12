#include "Devices/Generic/DelsysBaseDevice.h"

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "Data/DataPoint.h"
#include "Data/FixedTimeSeries.h"
#include "Devices/Exceptions.h"
#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

std::unique_ptr<STIMWALKER_NAMESPACE::data::TimeSeries>
timeSeriesGenerator(std::chrono::microseconds deltaTime) {
  return std::make_unique<STIMWALKER_NAMESPACE::data::FixedTimeSeries>(
      deltaTime);
};

DelsysBaseDevice::CommandTcpDevice::CommandTcpDevice(const std::string &host,
                                                     size_t port)
    : m_LastCommand(DelsysCommands::INITIALIZING),
      TcpDevice(host, port, std::chrono::milliseconds(100)) {}

std::string DelsysBaseDevice::CommandTcpDevice::deviceName() const {
  return "DelsysCommandTcpDevice";
}

std::vector<char> DelsysBaseDevice::CommandTcpDevice::read(size_t bufferSize) {
  if (m_LastCommand == DelsysCommands::STOP) {
    return std::vector<char>(bufferSize, '\0');
  }
  return TcpDevice::read(bufferSize);
}

DeviceResponses DelsysBaseDevice::CommandTcpDevice::parseAsyncSendCommand(
    const DeviceCommands &command, const std::any &data) {
  if (m_LastCommand == command) {
    return DeviceResponses::OK;
  }
  m_LastCommand = DelsysCommands(command.getValue());
  write(m_LastCommand.toString());
  std::vector<char> response = read(128);
  return std::strncmp(response.data(), "OK", 2) == 0 ? DeviceResponses::OK
                                                     : DeviceResponses::NOK;
}

DelsysBaseDevice::DataTcpDevice::DataTcpDevice(const std::string &host,
                                               size_t port)
    : TcpDevice(host, port, std::chrono::milliseconds(100)) {}

std::string DelsysBaseDevice::DataTcpDevice::deviceName() const {
  return "DelsysDataTcpDevice";
}

DeviceResponses DelsysBaseDevice::DataTcpDevice::parseAsyncSendCommand(
    const DeviceCommands &command, const std::any &data) {
  throw InvalidMethodException(
      "This method should not be called for DataTcpDevice");
}

DelsysBaseDevice::DelsysBaseDevice(size_t channelCount,
                                   std::chrono::microseconds deltaTime,
                                   size_t sampleCount, const std::string &host,
                                   size_t dataPort, size_t commandPort)
    : m_DeltaTime(deltaTime),
      m_CommandDevice(std::make_shared<CommandTcpDevice>(host, commandPort)),
      m_DataDevice(std::make_unique<DataTcpDevice>(host, dataPort)),
      m_BytesPerChannel(4), m_SampleCount(sampleCount),
      m_DataBuffer(
          std::vector<char>(channelCount * m_SampleCount * m_BytesPerChannel)),
      AsyncDevice(std::chrono::milliseconds(100)),
      AsyncDataCollector(
          channelCount, std::chrono::microseconds(1),
          [deltaTime]() { return timeSeriesGenerator(deltaTime); }) {
  m_IgnoreTooSlowWarning = true;
}

DelsysBaseDevice::DelsysBaseDevice(size_t channelCount,
                                   std::chrono::microseconds deltaTime,
                                   size_t sampleCount, size_t dataPort,
                                   const DelsysBaseDevice &other)
    : m_DeltaTime(deltaTime), m_CommandDevice(other.m_CommandDevice),
      m_DataDevice(std::make_unique<DataTcpDevice>(
          other.m_CommandDevice->getHost(), dataPort)),
      m_BytesPerChannel(4), m_SampleCount(sampleCount),
      m_DataBuffer(
          std::vector<char>(channelCount * m_SampleCount * m_BytesPerChannel)),
      AsyncDevice(std::chrono::milliseconds(100)),
      AsyncDataCollector(
          channelCount, std::chrono::microseconds(1),
          [deltaTime]() { return timeSeriesGenerator(deltaTime); }) {
  m_IgnoreTooSlowWarning = true;
}

DelsysBaseDevice::DelsysBaseDevice(
    std::unique_ptr<DelsysBaseDevice::DataTcpDevice> dataDevice,
    std::shared_ptr<DelsysBaseDevice::CommandTcpDevice> commandDevice,
    size_t channelCount, std::chrono::microseconds deltaTime,
    size_t sampleCount)
    : m_DeltaTime(deltaTime), m_CommandDevice(commandDevice),
      m_DataDevice(std::move(dataDevice)), m_BytesPerChannel(4),
      m_SampleCount(sampleCount),
      m_DataBuffer(
          std::vector<char>(channelCount * m_SampleCount * m_BytesPerChannel)),
      AsyncDevice(std::chrono::milliseconds(100)),
      AsyncDataCollector(
          channelCount, std::chrono::microseconds(1),
          [deltaTime]() { return timeSeriesGenerator(deltaTime); }) {
  m_IgnoreTooSlowWarning = true;
}

bool DelsysBaseDevice::handleConnect() {
  if (!m_CommandDevice->getIsConnected()) {
    // This is already connected if the delsys device is already connected to
    // another stream
    m_CommandDevice->connect();
    if (!m_CommandDevice->getIsConnected()) {
      utils::Logger::getInstance().fatal(
          "The command device is not connected, did you start Trigno?");
      return false;
    }
    m_CommandDevice->read(128); // Consume the welcome message
  }

  m_DataDevice->connect();
  if (!m_DataDevice->getIsConnected()) {
    utils::Logger::getInstance().fatal(
        "The data device is not connected, did you start Trigno?");
    m_CommandDevice->disconnect();
    return false;
  }

  return true;
}

bool DelsysBaseDevice::handleDisconnect() {
  if (m_IsStreamingData) {
    stopDataStreaming();
  }

  if (m_CommandDevice->getIsConnected()) {
    // This will happen if more than one devices are connected and one of them
    // disconnected the command device already
    m_CommandDevice->disconnect();
  }
  m_DataDevice->disconnect();

  return true;
}

bool DelsysBaseDevice::handleStartDataStreaming() {
  if (m_CommandDevice->send(DelsysCommands::START) != DeviceResponses::OK) {
    return false;
  }

  // Wait until the data starts streaming
  return m_DataDevice->read(m_DataBuffer);
}

bool DelsysBaseDevice::handleStopDataStreaming() {
  if (m_CommandDevice->send(DelsysCommands::STOP) != DeviceResponses::OK) {
    return false;
  }
  return true;
}

DeviceResponses
DelsysBaseDevice::parseAsyncSendCommand(const DeviceCommands &command,
                                        const std::any &data) {
  throw InvalidMethodException(
      "This method should not be called for Delsys devices");
}

void DelsysBaseDevice::dataCheck() {
  auto now = std::chrono::high_resolution_clock::now();
  m_DataDevice->read(m_DataBuffer);
  auto readingTime = std::chrono::high_resolution_clock::now();

  utils::Logger::getInstance().info(
      "Took " +
      std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                         readingTime - now)
                         .count()) +
      "ms to read the data");

  // Allocate space for all data in a single vector of floats
  std::vector<float> allData(m_SampleCount * m_DataChannelCount);

  // Perform a single memcpy to copy the entire data buffer as float
  std::memcpy(allData.data(), m_DataBuffer.data(),
              m_SampleCount * m_DataChannelCount * m_BytesPerChannel);

  // Now convert allData (float) to double precision
  std::vector<double> allDataAsDouble(allData.begin(), allData.end());

  // Now split the data into individual frames as doubles
  std::vector<std::vector<double>> dataPoints;
  dataPoints.reserve(m_SampleCount); // Pre-allocate space for the frames

  for (int i = 0; i < m_SampleCount; i++) {
    // Use iterators to create sub-vectors (views of the original data)
    std::vector<double> frame(allDataAsDouble.begin() + i * m_DataChannelCount,
                              allDataAsDouble.begin() +
                                  (i + 1) * m_DataChannelCount);

    // If the first frame is all zeros, assume no data were sent at all
    if (i == 0) {
      if (std::all_of(frame.begin(), frame.end(),
                      [](double value) { return value == 0.0; })) {
        return;
      }
    }
    dataPoints.push_back(std::move(frame));
  }

  addDataPoints(dataPoints);
}

void DelsysBaseDevice::handleNewData(const data::DataPoint &data) {
  // Do nothing
}

/// ------------ ///
/// MOCK SECTION ///
/// ------------ ///
using namespace DelsysBaseDeviceMock;
CommandTcpDeviceMock::CommandTcpDeviceMock(const std::string &host, size_t port)
    : m_LastCommand(DelsysCommandsMock::NONE), CommandTcpDevice(host, port) {}

bool CommandTcpDeviceMock::write(const std::string &data) {
  // Store the last command
  m_LastCommand = DelsysCommandsMock::fromString(data);
  return true;
}

bool CommandTcpDeviceMock::read(std::vector<char> &buffer) {
  // Prepare a response with bunch of \0 characters of length buffer.size()
  std::fill(buffer.begin(), buffer.end(), 0);

  switch (m_LastCommand.getValue()) {
  case DelsysCommandsMock::NONE: {
    // Write the welcome message
    std::string welcomeMessage =
        "Delsys Trigno System Digital Protocol Version 3.6.0 \r\n\r\n";
    std::copy(welcomeMessage.begin(), welcomeMessage.end(), buffer.begin());
    break;
  }
  case DelsysCommands::START:
  case DelsysCommands::STOP: {
    // Write the OK message
    std::string response = "OK\r\n\r\n";
    std::copy(response.begin(), response.end(), buffer.begin());
    break;
  }

  default: {
    throw InvalidMethodException("This command is not MOCK yet");
  }
  }

  return true;
}

bool CommandTcpDeviceMock::handleConnect() { return true; }

DataTcpDeviceMock::DataTcpDeviceMock(size_t channelCount,
                                     std::chrono::microseconds deltaTime,
                                     size_t sampleCount,
                                     const std::string &host, size_t port)
    : m_DataChannelCount(channelCount), m_DeltaTime(deltaTime),
      m_SampleCount(sampleCount), m_DataCounter(0), DataTcpDevice(host, port) {}

bool DataTcpDeviceMock::read(std::vector<char> &buffer) {
  size_t bytesPerChannel(4);

  // Wait for the next cycle of data
  auto newDataArrivesAt =
      m_StartTime + m_DeltaTime * m_SampleCount * m_DataCounter;
  std::this_thread::sleep_until(newDataArrivesAt);

  // Copy the 4-byte representation of the float into the byte array
  unsigned char dataAsChar[4];
  for (size_t i = 0; i < m_SampleCount; i++) {
    float value = static_cast<float>(
        std::sin(static_cast<float>(m_DataCounter * m_SampleCount + i) /
                 2000.0f * 2 * M_PI));
    std::memcpy(dataAsChar, &value, sizeof(float));
    for (size_t j = 0; j < m_DataChannelCount; j++)
      std::copy(dataAsChar, dataAsChar + 4,
                buffer.begin() + i * bytesPerChannel * m_DataChannelCount +
                    j * bytesPerChannel);
  }

  m_DataCounter++;
  return true;
}

bool DataTcpDeviceMock::handleConnect() {
  m_DataCounter = 0;
  m_StartTime = std::chrono::high_resolution_clock::now();
  return true;
}

DeviceResponses
DataTcpDeviceMock::parseAsyncSendCommand(const DeviceCommands &command,
                                         const std::any &data) {
  return DeviceResponses::OK;
}
