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
    : m_CommandDevice(CommandTcpDevice(host, commandPort)),
      m_DataDevice(DataTcpDevice(host, dataPort)), m_BytesPerChannel(4),
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

void DelsysEmgDevice::startRecording() {
  if (m_IsRecording) {
    throw DeviceIsRecordingException("The device is already recording");
  }

  if (m_CommandDevice.send(DelsysCommands::START) != DeviceResponses::OK) {
    throw DeviceFailedToStartRecordingException("Command failed: START");
  }
  m_IsRecording = true;
}

void DelsysEmgDevice::stopRecording() {
  if (!m_IsRecording) {
    throw DeviceIsNotRecordingException("The device is not recording");
  }

  if (m_CommandDevice.send(DelsysCommands::STOP) != DeviceResponses::OK) {
    throw DeviceFailedToStopRecordingException("Command failed: STOP");
  }
  m_IsRecording = false;
}

void DelsysEmgDevice::handleConnect() {
  if (m_IsConnected) {
    throw DeviceIsConnectedException("The device is already connected");
  }

  try {
    m_CommandDevice.connect();
    m_CommandDevice.read(128); // Consume the welcome message
    m_DataDevice.connect();
    m_IsConnected = true;
  } catch (DeviceConnexionFailedException &e) {
    utils::Logger::getInstance().fatal(
        "The command device is not connected, did you start Trigno?");
    throw e;
  }
}

DeviceResponses DelsysEmgDevice::parseSendCommand(const DeviceCommands &command,
                                                  const std::any &data) {
  throw DeviceShouldNotUseSendException(
      "This method should not be called for DelsysEmgDevice");
}

data::DataPoint DelsysEmgDevice::readData() {

  // Wait for some time
  std::vector<std::vector<double>> allData;
  while (allData.size() < 1000) {
    m_DataDevice.read(m_DataBuffer);

    std::vector<double> data(m_ChannelCount * m_SampleCount);
    std::memcpy(data.data(), m_DataBuffer.data(),
                m_ChannelCount * m_SampleCount * m_BytesPerChannel);
    for (int i = 0; i < m_SampleCount; i++) {
      std::vector<double> dataAsDouble(m_ChannelCount);
      std::transform(data.begin() + i * m_ChannelCount,
                     data.begin() + (i + 1) * m_ChannelCount,
                     dataAsDouble.begin(),
                     [](int x) { return static_cast<double>(x); });
      allData.push_back(dataAsDouble);
    }
  }

  return data::DataPoint(allData[0]);
}