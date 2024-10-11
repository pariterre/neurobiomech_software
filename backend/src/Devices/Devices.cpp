#include "Devices/Devices.h"

#include "Data/TimeSeries.h"
#include "Devices/Exceptions.h"
#include "Devices/Generic/DataCollector.h"
#include "Utils/Logger.h"
#include <thread>

using namespace STIMWALKER_NAMESPACE;
using namespace STIMWALKER_NAMESPACE::devices;

int Devices::add(std::unique_ptr<Device> device) {
  std::string deviceName = device->deviceName();
  static int deviceId = 0;

  // Add the device to the device collection if it does not exist yet
  m_Devices[deviceId] = std::move(device);

  // If we can dynamic cast the device to a data collector, add it to the data
  // collector collection
  if (auto dataCollector =
          std::dynamic_pointer_cast<DataCollector>(m_Devices[deviceId])) {
    m_DataCollectors[deviceId] = dataCollector;
  }

  return deviceId++;
}

void Devices::remove(int deviceId) {
  m_Devices.erase(deviceId);
  m_DataCollectors.erase(deviceId);
}

void Devices::clear() {
  m_Devices.clear();
  m_DataCollectors.clear();
}

Device &Devices::getDevice(int deviceId) {
  try {
    return *m_Devices.at(deviceId);
  } catch (const std::out_of_range &) {
    std::string message =
        "Device with id " + std::to_string(deviceId) + " does not exist";
    utils::Logger::getInstance().fatal(message);
    throw DeviceNotExistsException(message);
  }
}

const Device &Devices::getDevice(int deviceId) const {
  try {
    return *m_Devices.at(deviceId);
  } catch (const std::out_of_range &) {
    std::string message =
        "Device with id " + std::to_string(deviceId) + " does not exist";
    utils::Logger::getInstance().fatal(message);
    throw DeviceNotExistsException(message);
  }
}

DataCollector &Devices::getDataCollector(int deviceId) {
  try {
    return *m_DataCollectors.at(deviceId);
  } catch (const std::out_of_range &) {
    std::string message = "Data collector with id " + std::to_string(deviceId) +
                          " does not exist";
    utils::Logger::getInstance().fatal(message);
    throw DeviceNotExistsException(message);
  }
}

const DataCollector &Devices::getDataCollector(int deviceId) const {
  try {
    return *m_DataCollectors.at(deviceId);
  } catch (const std::out_of_range &) {
    std::string message = "Data collector with id " + std::to_string(deviceId) +
                          " does not exist";
    utils::Logger::getInstance().fatal(message);
    throw DeviceNotExistsException(message);
  }
}

void Devices::connect() {
  for (auto &[deviceId, device] : m_Devices) {
    device->connect();
  }
}

void Devices::disconnect() {
  for (auto &[deviceId, device] : m_Devices) {
    device->disconnect();
  }
}

void Devices::startRecording() {

  for (auto &[deviceId, dataCollector] : m_DataCollectors) {
    dataCollector->startRecording();
  }

  // For all the devices to have responded (either is connected or failed to)
  size_t hasConnected = 0;
  size_t hasFailedToConnect = 0;
  while (true) {
    hasConnected = 0;
    hasFailedToConnect = 0;

    for (auto &[deviceId, dataCollector] : m_DataCollectors) {
      if (dataCollector->getIsRecording()) {
        hasConnected++;
      }
      if (dataCollector->getHasFailedToStartRecording()) {
        hasFailedToConnect++;
      }
    }
    if (hasConnected + hasFailedToConnect == m_DataCollectors.size()) {
      break;
    }

    // If we get here, we have to give more time for the devices to connect
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // If any of the devices failed to start recording, stop all the devices and
  // throw an exception
  if (hasFailedToConnect > 0) {
    stopRecording();
    std::string errorMessage(
        "One or more devices failed to start recording, stopping all devices");
    utils::Logger::getInstance().fatal(errorMessage);
    throw DeviceFailedToStartRecordingException(errorMessage);
  }

  // If all the devices are ready, reset the time series for now and set the
  // starting time to now
  for (auto &[deviceId, dataCollector] : m_DataCollectors) {
    dataCollector->m_TimeSeries->clear();
  }
}

void Devices::stopRecording() {
  for (auto &[deviceId, dataCollector] : m_DataCollectors) {
    dataCollector->stopRecording();
  }
}

std::map<int, data::TimeSeries> Devices::getTrialData() const {
  std::map<int, data::TimeSeries> data;
  for (const auto &[deviceId, dataCollector] : m_DataCollectors) {
    data[deviceId] = dataCollector->getTrialData();
  }
  return data;
}

nlohmann::json Devices::serialize() const {
  nlohmann::json json;
  for (const auto &[deviceName, dataCollector] : m_DataCollectors) {
    json[deviceName] = dataCollector->getTrialData().serialize();
  }
  return json;
}
