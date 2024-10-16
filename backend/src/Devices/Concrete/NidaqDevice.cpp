#include "Devices/Concrete/NidaqDevice.h"

#include "Devices/Exceptions.h"

#include <iostream>
using namespace STIMWALKER_NAMESPACE::devices;

NidaqDevice::NidaqDevice(size_t channelCount,
                         const std::chrono::microseconds &dataCheckIntervals)
    : AsyncDevice(dataCheckIntervals),
      DataCollector(channelCount, std::make_unique<data::TimeSeries>()) {}

NidaqDevice::~NidaqDevice() {
  if (m_IsStreamingData) {
    stopDataStreaming();
  }

  if (m_IsConnected) {
    disconnect();
  }
}

std::string NidaqDevice::deviceName() const { return "NidaqDevice"; }

std::string NidaqDevice::dataCollectorName() const {
  return "NidaqDataCollector";
}

bool NidaqDevice::handleConnect() {
  // TODO Implement the connection to the device
  return false;
}

bool NidaqDevice::handleDisconnect() {
  // TODO Implement the disconnection from the device
  return false;
}

bool NidaqDevice::handleStartDataStreaming() {
  // TODO Implement the start recording
  return false;
}

bool NidaqDevice::handleStopDataStreaming() {
  // TODO Implement the stop recording
  return false;
}

void NidaqDevice::handleNewData(const data::DataPoint &data) {
  // Do nothing
}

DeviceResponses
NidaqDevice::parseAsyncSendCommand(const DeviceCommands &command,
                                   const std::any &data) {
  throw InvalidMethodException(
      "This method should not be called for NidaqDevice");
}
