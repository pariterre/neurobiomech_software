#include "Devices/NidaqDevice.h"

#include "Devices/Generic/Exceptions.h"

#include <iostream>
using namespace STIMWALKER_NAMESPACE::devices;

NidaqDevice::NidaqDevice(size_t channelCount, size_t frameRate)
    : AsyncDevice(), DataCollector(channelCount, frameRate) {}

NidaqDevice::~NidaqDevice() {
  if (m_IsRecording) {
    stopRecording();
  }

  if (m_IsConnected) {
    disconnect();
  }
}

void NidaqDevice::disconnect() {
  if (!m_IsConnected) {
    throw DeviceIsNotConnectedException("The device is not connected");
  }

  if (m_IsRecording) {
    stopRecording();
  }

  AsyncDevice::disconnect();
}

void NidaqDevice::handleConnect() {
  // TODO Implement the connection to the device
}

void NidaqDevice::handleStartRecording() {
  // TODO Implement the start recording
}

void NidaqDevice::handleStopRecording() {
  // TODO Implement the stop recording
}

DeviceResponses NidaqDevice::parseSendCommand(const DeviceCommands &command,
                                              const std::any &data) {
  throw DeviceShouldNotUseSendException(
      "This method should not be called for NidaqDevice");
}

// // Mock implementation
// NidaqDeviceMock::NidaqDeviceMock(int nbChannels, int frameRate)
//     : NidaqDevice(nbChannels, frameRate) {}

// void NidaqDeviceMock::startRecording() {

//   {
//     std::lock_guard<std::mutex> lock(m_recordingMutex);
//     m_generateData = true;
//   }

//   m_recordingThread = std::thread(&NidaqDeviceMock::generateData, this);
// }

// void NidaqDeviceMock::stopRecordingInternal() {
//   {
//     std::lock_guard<std::mutex> lock(m_recordingMutex);
//     m_isRecording = false;
//     m_generateData = false;
//   }
//   if (m_recordingThread.joinable())
//     m_recordingThread.join();
// }

// void NidaqDeviceMock::generateData() {
//   while (true) {
//     {
//       std::lock_guard<std::mutex> lock(m_recordingMutex);
//       if (!m_generateData)
//         break;
//     }

//     // Generate data
//     auto now =
//         std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
//     auto data = std::vector<double>(getNbChannels(), 0.0);
//     CollectorData newData(now, data);
//     notifyListeners(newData);

//     std::this_thread::sleep_for(std::chrono::milliseconds(1 / m_frameRate));
//   }
// }
