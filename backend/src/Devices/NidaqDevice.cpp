#include "Devices/NidaqDevice.h"

#include "Devices/Generic/Exceptions.h"

#include <iostream>
using namespace STIMWALKER_NAMESPACE::devices;

NidaqDevice::NidaqDevice(size_t channelCount, size_t frameRate)
    : Device(), DataCollector(channelCount, frameRate) {}

NidaqDevice::~NidaqDevice() {
  if (m_IsRecording) {
    stopRecording();
  }

  if (m_IsConnected) {
    disconnect();
  }
}

void NidaqDevice::connect() {
  if (m_IsConnected) {
    throw DeviceIsConnectedException("The device is already connected");
  }

  // TODO Implement the connection to the device

  m_IsConnected = true;
}

void NidaqDevice::disconnect() {
  if (!m_IsConnected) {
    throw DeviceIsNotConnectedException("The device is not connected");
  }

  if (m_IsRecording) {
    stopRecording();
  }

  // TODO Implement the disconnection from the device

  m_IsConnected = false;
}

void NidaqDevice::startRecording() {
  if (!m_IsConnected) {
    throw DeviceIsNotConnectedException("The device is not connected");
  }
  if (m_IsRecording) {
    throw DeviceIsRecordingException("The device is already recording");
  }

  // TODO Implement the start recording

  m_IsRecording = true;
}

void NidaqDevice::stopRecording() {
  if (!m_IsRecording) {
    throw DeviceIsNotRecordingException("The device is not recording");
  }

  // TODO Implement the stop recording

  m_IsRecording = false;
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
