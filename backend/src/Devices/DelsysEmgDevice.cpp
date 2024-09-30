#include "Devices/DelsysEmgDevice.h"

#include <cstring>
#include <iostream>
#include <stdexcept>

#include "Devices/Data/DataPoint.h"
#include "Devices/Generic/Exceptions.h"

using namespace STIMWALKER_NAMESPACE::devices;

DelsysEmgDevice::DelsysEmgDevice(std::vector<size_t> channelIndices,
                                 size_t frameRate, const std::string &host,
                                 size_t commandPort, size_t dataPort)
    : m_ChannelIndices(channelIndices),
      m_CommandDevice(TcpDevice(host, commandPort)),
      m_DataDevice(TcpDevice(host, dataPort)),
      m_TerminaisonCharacters("\r\n\r\n"), m_BytesPerChannel(4), AsyncDevice(),
      DataCollector(channelIndices.size(), frameRate) {}

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

  m_CommandDevice.send(DelsysCommands::START);
  m_IsRecording = true;
}

void DelsysEmgDevice::stopRecording() {
  if (!m_IsRecording) {
    throw DeviceIsNotRecordingException("The device is not recording");
  }

  m_CommandDevice.send(DelsysCommands::STOP);

  m_IsRecording = false;
}

void DelsysEmgDevice::handleConnect() {
  if (getIsConnected()) {
    throw DeviceIsConnectedException("The device is already connected");
  }

  m_CommandDevice.connect();
  m_CommandDevice.read(1024); // Consume the welcome message

  m_DataDevice.connect();
  m_IsConnected = true;
}

DeviceResponses DelsysEmgDevice::parseCommand(const DeviceCommands &command,
                                              const std::any &data) {

  std::string fullCommand(command.toString() + m_TerminaisonCharacters);
  m_CommandDevice.send(command, data);

  // Response is exactly 128 bytes long
  std::vector<char> response = m_CommandDevice.read(128);

  // Check if the response is not OK
  if (std::strncmp(response.data(), "OK", 2) != 0) {
    throw std::runtime_error("Command failed: " + command.toString());
  }

  return DeviceResponses::OK;
}

size_t DelsysEmgDevice::bufferSize() const {
  return m_DataChannelCount * m_BytesPerChannel;
}

data::DataPoint DelsysEmgDevice::read() {
  // The raw data are supposed to be a single vector char of size
  // timeCount x channelCount x bytesPerChannel. We need to reshape it
  // to a timeCount x channelCount matrix of floats.
  std::vector<char> raw(m_DataDevice.read(bufferSize()));

  std::vector<double> data(m_DataChannelCount);
  for (int i = 0; i < m_DataChannelCount; ++i) {
    data[i] = *reinterpret_cast<float *>(raw.data() + i * m_BytesPerChannel);
  }
  data::DataPoint dataPoint(data);

  if (m_IsRecording) {
    onNewData.notifyListeners(dataPoint);
  }
  return dataPoint;
}