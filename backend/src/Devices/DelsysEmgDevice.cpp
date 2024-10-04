#include "Devices/DelsysEmgDevice.h"

#include <cstring>
#include <iostream>
#include <stdexcept>

#include "Devices/Data/DataPoint.h"
#include "Devices/Generic/Exceptions.h"
#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

DelsysEmgDevice::CommandTcpDevice::CommandTcpDevice(const std::string &host,
                                                    size_t port)
    : TcpDevice(host, port) {}

DeviceResponses DelsysEmgDevice::CommandTcpDevice::parseSendCommand(
    const DeviceCommands &command, const std::any &data) {
  auto commandAsDelsys = DelsysCommands(command.getValue());
  write(commandAsDelsys.toString());
  std::vector<char> response = read(128);
  return std::strncmp(response.data(), "OK", 2) == 0 ? DeviceResponses::OK
                                                     : DeviceResponses::NOK;
}

DelsysEmgDevice::DataTcpDevice::DataTcpDevice(const std::string &host,
                                              size_t port)
    : TcpDevice(host, port) {}

DeviceResponses
DelsysEmgDevice::DataTcpDevice::parseSendCommand(const DeviceCommands &command,
                                                 const std::any &data) {
  throw DeviceShouldNotUseSendException(
      "This method should not be called for DataTcpDevice");
}

DelsysEmgDevice::DelsysEmgDevice(const std::string &host, size_t commandPort,
                                 size_t dataPort)
    : m_CommandDevice(std::make_unique<CommandTcpDevice>(host, commandPort)),
      m_DataDevice(std::make_unique<DataTcpDevice>(host, dataPort)),
      m_BytesPerChannel(4), m_ChannelCount(16), m_SampleCount(27),
      m_DataBuffer(std::vector<char>(m_ChannelCount * m_SampleCount *
                                     m_BytesPerChannel)),
      AsyncDevice(), DataCollector(16, 2000) {}

DelsysEmgDevice::DelsysEmgDevice(
    std::unique_ptr<DelsysEmgDevice::CommandTcpDevice> commandDevice,
    std::unique_ptr<DelsysEmgDevice::DataTcpDevice> dataDevice,
    const std::string &host, size_t commandPort, size_t dataPort)
    : m_CommandDevice(std::move(commandDevice)),
      m_DataDevice(std::move(dataDevice)), m_BytesPerChannel(4),
      m_ChannelCount(16), m_SampleCount(27),
      m_DataBuffer(std::vector<char>(m_ChannelCount * m_SampleCount *
                                     m_BytesPerChannel)),
      AsyncDevice(), DataCollector(16, 2000) {}

DelsysEmgDevice::~DelsysEmgDevice() {
  if (m_IsRecording) {
    stopRecording();
  }

  if (m_IsConnected) {
    disconnect();
  }
}

void DelsysEmgDevice::disconnect() {
  if (!m_IsConnected) {
    throw DeviceIsNotConnectedException("The device is not connected");
  }

  if (m_IsRecording) {
    stopRecording();
  }

  AsyncDevice::disconnect();
}

void DelsysEmgDevice::handleConnect() {
  if (m_IsConnected) {
    throw DeviceIsConnectedException("The device is already connected");
  }

  try {
    m_CommandDevice->connect();
    m_CommandDevice->read(128); // Consume the welcome message
    m_DataDevice->connect();
    m_IsConnected = true;
  } catch (DeviceConnexionFailedException &e) {
    utils::Logger::getInstance().fatal(
        "The command device is not connected, did you start Trigno?");
    throw e;
  }
}

void DelsysEmgDevice::handleStartRecording() {
  if (m_CommandDevice->send(DelsysCommands::START) != DeviceResponses::OK) {
    throw DeviceFailedToStartRecordingException("Command failed: START");
  }
}

void DelsysEmgDevice::handleStopRecording() {
  if (m_CommandDevice->send(DelsysCommands::STOP) != DeviceResponses::OK) {
    throw DeviceFailedToStopRecordingException("Command failed: STOP");
  }
}

DeviceResponses DelsysEmgDevice::parseSendCommand(const DeviceCommands &command,
                                                  const std::any &data) {
  throw DeviceShouldNotUseSendException(
      "This method should not be called for DelsysEmgDevice");
}

data::DataPoint DelsysEmgDevice::readData() {

  // // Wait for some time
  // std::vector<std::vector<double>> allData;
  // while (allData.size() < 1000) {
  //   m_DataDevice.read(m_DataBuffer);

  //   std::vector<double> data(m_ChannelCount * m_SampleCount);
  //   std::memcpy(data.data(), m_DataBuffer.data(),
  //               m_ChannelCount * m_SampleCount * m_BytesPerChannel);
  //   for (int i = 0; i < m_SampleCount; i++) {
  //     std::vector<double> dataAsDouble(m_ChannelCount);
  //     std::transform(data.begin() + i * m_ChannelCount,
  //                    data.begin() + (i + 1) * m_ChannelCount,
  //                    dataAsDouble.begin(),
  //                    [](int x) { return static_cast<double>(x); });
  //     allData.push_back(dataAsDouble);
  //   }
  // }

  return data::DataPoint(std::vector<double>(16, 0.0));
}

/// ------------ ///
/// MOCK SECTION ///
/// ------------ ///

DelsysEmgDeviceMock::CommandTcpDeviceMock::CommandTcpDeviceMock(
    const std::string &host, size_t port)
    : CommandTcpDevice(host, port) {}

void DelsysEmgDeviceMock::CommandTcpDeviceMock::read(
    std::vector<char> &buffer) {
  // Write the welcome message followed by two new lines and fill the rest with
  // a bunch of \0
  std::string welcomeMessage =
      "Delsys Trigno System Digital Protocol Version 3.6.0 \r\n\r\n";
  std::copy(welcomeMessage.begin(), welcomeMessage.end(), buffer.begin());
  std::fill(buffer.begin() + welcomeMessage.size(), buffer.end(), 0);
}

void DelsysEmgDeviceMock::CommandTcpDeviceMock::handleConnect() {
  m_IsConnected = true;
}

DeviceResponses DelsysEmgDeviceMock::CommandTcpDeviceMock::parseSendCommand(
    const DeviceCommands &command, const std::any &data) {
  m_IsConnected = true;
  return DeviceResponses::OK;
}

DelsysEmgDeviceMock::DataTcpDeviceMock::DataTcpDeviceMock(
    const std::string &host, size_t port)
    : DataTcpDevice(host, port) {}

void DelsysEmgDeviceMock::DataTcpDeviceMock::read(std::vector<char> &buffer) {
  // Write the value 1 at each element of the buffer
  std::fill(buffer.begin(), buffer.end(), 1);
}

void DelsysEmgDeviceMock::DataTcpDeviceMock::handleConnect() {
  m_IsConnected = true;
}

DeviceResponses DelsysEmgDeviceMock::DataTcpDeviceMock::parseSendCommand(
    const DeviceCommands &command, const std::any &data) {
  return DeviceResponses::OK;
}

DelsysEmgDeviceMock::DelsysEmgDeviceMock(const std::string &host,
                                         size_t commandPort, size_t dataPort)
    : DelsysEmgDevice(
          std::make_unique<DelsysEmgDeviceMock::CommandTcpDeviceMock>(
              host, commandPort),
          std::make_unique<DelsysEmgDeviceMock::DataTcpDeviceMock>(host,
                                                                   dataPort),
          host, commandPort, dataPort) {}
