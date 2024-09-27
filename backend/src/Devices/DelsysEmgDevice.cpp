#include "Devices/DelsysEmgDevice.h"

#include <cstring>
#include <iostream>
#include <stdexcept>

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

  sendCommand("START");
  m_IsRecording = true;
}

void DelsysEmgDevice::stopRecording() {
  if (!m_IsRecording) {
    throw DeviceIsNotRecordingException("The device is not recording");
  }

  sendCommand("STOP");

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

void DelsysEmgDevice::parseCommand(const std::string &command) {
  // TODO: Change the input string to enum
  m_CommandDevice.send(
      asio::buffer((command + m_TerminaisonCharacters).c_str()));
  char[128] response;
  m_CommandDevice.receive(asio::buffer(response));
  if (std::string(response) != "OK") {
    throw std::runtime_error("Command failed: " + command);
  }
}

void DelsysEmgDevice::HandleNewData(const DataPoint &data) {
  OnNewData.notifyListeners(data);
}

size_t DelsysEmgDevice::bufferSize() const {
  return m_DataChannelCount * m_BytesPerChannel;
}

std::vector<float> DelsysEmgDevice::read(size_t bufferSize) {

  std::vector<char> out = m_DataDevice.read(bufferSize);

  std::vector<float> data(bufferSize / m_BytesPerChannel);
  std::memcpy(data.data(), packet.data(), bufferSize);

  std::vector<std::vector<float>> reshaped_data(
      total_channels_, std::vector<float>(num_samples));
  for (int i = 0; i < num_samples; ++i) {
    for (int j = 0; j < total_channels_; ++j) {
      reshaped_data[j][i] = data[i * total_channels_ + j];
    }
  }

  if (is_recording_) {
    for (int i = 0; i < num_samples; ++i) {
      for (int j = 0; j < total_channels_; ++j) {
        recording_file_ << reshaped_data[j][i] << ",";
      }
      recording_file_ << "\n";
    }
    recording_file_.flush();
  }

  return reshaped_data;
}