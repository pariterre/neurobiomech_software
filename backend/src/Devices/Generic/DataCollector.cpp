#include "Devices/Generic/DataCollector.h"

#include "Devices/Generic/Exceptions.h"
#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

void DataCollector::startRecording() {
  if (m_IsRecording) {
    throw DeviceIsRecordingException("The device is already recording");
  }

  // RENDU ICI!!!! Start a thread and call handleStartRecording in the thread
  handleStartRecording();
  m_IsRecording = true;
}

void DataCollector::stopRecording() {
  if (!m_IsRecording) {
    throw DeviceIsNotRecordingException("The device is not recording");
  }

  // RENDU ICI!!!! Start a thread and call handleStopRecording in the thread
  handleStopRecording();
  m_IsRecording = false;
}