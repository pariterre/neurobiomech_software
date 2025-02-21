#include "Devices/Devices.h"

#include "Data/TimeSeries.h"
#include "Devices/Exceptions.h"
#include "Devices/Generic/AsyncDataCollector.h"
#include "Devices/Generic/AsyncDevice.h"
#include "Devices/Generic/DataCollector.h"
#include "Utils/Logger.h"
#include <thread>

using namespace NEUROBIO_NAMESPACE;
using namespace NEUROBIO_NAMESPACE::devices;

Devices::~Devices() {
  if (m_IsConnected) {
    disconnect();
  }
}

size_t Devices::add(std::unique_ptr<Device> device) {
  std::string deviceName = device->deviceName();
  static size_t deviceId = 0;

  // Add the device to the device collection if it does not exist yet
  m_Devices[deviceId] = std::move(device);

  // If we can dynamic cast the device to a data collector, add it to the data
  // collector collection
  std::lock_guard<std::mutex> lock(m_MutexDataCollectors);
  if (auto dataCollector =
          std::dynamic_pointer_cast<DataCollector>(m_Devices[deviceId])) {
    m_DataCollectors[deviceId] = dataCollector;
  }

  return deviceId++;
}

bool Devices::zeroLevelDevice(const std::string &deviceName) {
  for (auto &[deviceId, device] : m_Devices) {
    if (device->deviceName() == deviceName) {
      m_DataCollectors[deviceId]->setZeroLevel(std::chrono::milliseconds(1000));
    }
  }
  return true;
}

void Devices::remove(size_t deviceId) {
  m_Devices[deviceId]->disconnect();
  std::lock_guard<std::mutex> lock(m_MutexDataCollectors);
  m_DataCollectors.erase(deviceId);
  m_Devices.erase(deviceId);
}

std::vector<size_t> Devices::getDeviceIds() const {
  std::vector<size_t> deviceIds;
  for (auto &[deviceId, device] : m_Devices) {
    deviceIds.push_back(deviceId);
  }
  return deviceIds;
}

std::vector<std::string> Devices::getDeviceNames() const {
  std::vector<std::string> deviceNames;
  for (auto &[deviceId, device] : m_Devices) {
    deviceNames.push_back(device->deviceName());
  }
  return deviceNames;
}

size_t Devices::size() const { return m_Devices.size(); }

void Devices::clear() {
  if (m_IsConnected) {
    disconnect();
  }

  std::lock_guard<std::mutex> lock(m_MutexDataCollectors);
  m_Devices.clear();
  m_DataCollectors.clear();
}

const Device &Devices::operator[](size_t deviceId) const {
  try {
    return *m_Devices.at(deviceId);
  } catch (const std::out_of_range &) {
    std::string message =
        "Device with id " + std::to_string(deviceId) + " does not exist";
    utils::Logger::getInstance().fatal(message);
    throw DeviceNotFoundException(message);
  }
}

const Device &Devices::getDevice(size_t deviceId) const {
  try {
    return *m_Devices.at(deviceId);
  } catch (const std::out_of_range &) {
    std::string message =
        "Device with id " + std::to_string(deviceId) + " does not exist";
    utils::Logger::getInstance().fatal(message);
    throw DeviceNotFoundException(message);
  }
}

const DataCollector &Devices::getDataCollector(size_t deviceId) const {
  try {
    std::lock_guard<std::mutex> lock(
        const_cast<std::mutex &>(m_MutexDataCollectors));
    return *m_DataCollectors.at(deviceId);
  } catch (const std::out_of_range &) {
    std::string message = "Data collector with id " + std::to_string(deviceId) +
                          " does not exist";
    utils::Logger::getInstance().fatal(message);
    throw DeviceNotFoundException(message);
  }
}

bool Devices::connect() {
  m_IsConnected = false;

  for (auto &[deviceId, device] : m_Devices) {
    // Try to connect the device asynchronously so it takes less time
    try {
      auto &asyncDevice = dynamic_cast<AsyncDevice &>(*device);
      asyncDevice.connectAsync();
    } catch (const std::bad_cast &) {
      device->connect();
    }
  }

  // Wait for all the devices to connect (or fail to connect)
  size_t hasConnected = 0;
  size_t hasFailedToConnect = 0;
  while (true) {
    hasConnected = 0;
    hasFailedToConnect = 0;

    for (auto &[deviceId, device] : m_Devices) {
      if (device->getIsConnected()) {
        hasConnected++;
      }
      if (device->getHasFailedToConnect()) {
        hasFailedToConnect++;
      }
    }
    if (hasConnected + hasFailedToConnect == m_Devices.size()) {
      break;
    }

    // If we get here, we have to give more time for the devices to connect
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // If any of the devices failed to connect, disconnect all the devices
  if (hasFailedToConnect > 0) {
    disconnect();
    utils::Logger::getInstance().fatal(
        "One or more devices failed to connect, disconnecting all devices");
    return false;
  }

  utils::Logger::getInstance().info("All devices are now connected");
  m_IsConnected = true;
  return true;
}

bool Devices::disconnect() {
  if (m_IsRecording) {
    stopRecording();
  }

  if (m_IsStreamingData) {
    stopDataStreaming();
  }

  bool allDisconnected = true;
  for (auto &[deviceId, device] : m_Devices) {
    allDisconnected = device->disconnect() && allDisconnected;
  }

  m_IsConnected = !allDisconnected;
  if (m_IsConnected) {
    utils::Logger::getInstance().fatal(
        "One or more devices failed to disconnect");
  } else {
    utils::Logger::getInstance().info("All devices are now disconnected");
  }
  return allDisconnected;
}

bool Devices::startDataStreaming() {
  // We have to set this to true now because other methods might rely on it
  m_IsStreamingData = true;
  {
    std::lock_guard<std::mutex> lock(m_MutexDataCollectors);
    for (auto &[deviceId, dataCollector] : m_DataCollectors) {
      try {
        // Try to start the data streaming asynchronously so it takes less time
        auto &asyncDataCollector =
            dynamic_cast<AsyncDataCollector &>(*dataCollector);
        asyncDataCollector.startDataStreamingAsync();
      } catch (const std::bad_cast &) {
        dataCollector->startDataStreaming();
      }
    }
  }
  // Wait for all the devices to start streaming data (or fail to)
  size_t hasStartedStreaming = 0;
  size_t hasFailedToStartStreaming = 0;
  while (true) {
    hasStartedStreaming = 0;
    hasFailedToStartStreaming = 0;
    {
      std::lock_guard<std::mutex> lock(m_MutexDataCollectors);
      for (auto &[deviceId, dataCollector] : m_DataCollectors) {
        if (dataCollector->getIsStreamingData()) {
          hasStartedStreaming++;
        }
        if (dataCollector->getHasFailedToStartDataStreaming()) {
          hasFailedToStartStreaming++;
        }
      }
      if (hasStartedStreaming + hasFailedToStartStreaming ==
          m_DataCollectors.size()) {
        break;
      }
    }

    // If we get here, we have to give more time for the devices to connect
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // If any of the devices failed to start streaming data, stop all the devices
  if (hasFailedToStartStreaming > 0) {
    stopDataStreaming();
    m_IsStreamingData = false;
    utils::Logger::getInstance().fatal("One or more devices failed to start "
                                       "streaming data, stopping all devices");
    return false;
  }

  // If all the devices are ready, reset the live time series and set the
  // starting time to now
  {
    std::lock_guard<std::mutex> lock(m_MutexDataCollectors);
    for (auto &[deviceId, dataCollector] : m_DataCollectors) {
      dataCollector->resetLiveData();
    }
  }

  utils::Logger::getInstance().info("All devices are now streaming data");
  m_IsStreamingData = true;
  return true;
}

bool Devices::stopDataStreaming() {
  if (m_IsRecording) {
    stopRecording();
  }

  // Stop all the recording (this is just to make sure in case some device is
  // started while m_IsRecording is false)
  {
    std::lock_guard<std::mutex> lock(m_MutexDataCollectors);
    for (auto &[deviceId, dataCollector] : m_DataCollectors) {
      dataCollector->stopRecording();
    }
  }

  bool allStopped = true;
  {
    std::lock_guard<std::mutex> lock(m_MutexDataCollectors);
    for (auto &[deviceId, dataCollector] : m_DataCollectors) {
      allStopped = dataCollector->stopDataStreaming() && allStopped;
    }
  }

  utils::Logger::getInstance().info("All devices have stopped streaming data");
  m_IsStreamingData = !allStopped;
  return allStopped;
}

bool Devices::startRecording() {
  bool hasStartedRecording = true;

  for (auto &[deviceId, dataCollector] : m_DataCollectors) {
    hasStartedRecording =
        dataCollector->startRecording() && hasStartedRecording;
  }

  if (!hasStartedRecording) {
    utils::Logger::getInstance().fatal(
        "One or more devices failed to start "
        "recording, stopping to record on all devices");
    stopRecording();
    return false;
  }

  m_IsRecording = true;
  utils::Logger::getInstance().info("All devices are now recording");
  return true;
}

bool Devices::stopRecording() {
  bool hasStoppedRecording = true;

  {
    std::lock_guard<std::mutex> lock(m_MutexDataCollectors);
    for (auto &[deviceId, dataCollector] : m_DataCollectors) {
      hasStoppedRecording =
          dataCollector->stopRecording() && hasStoppedRecording;
    }
  }

  if (!hasStoppedRecording) {
    utils::Logger::getInstance().fatal(
        "One or more devices failed to stop recording");
    return false;
  }

  m_IsRecording = false;
  utils::Logger::getInstance().info("All devices have stopped recording");
  return true;
}

std::map<std::string, data::TimeSeries> Devices::getLiveData() const {
  std::map<std::string, data::TimeSeries> data;
  for (const auto &[deviceId, dataCollector] : m_DataCollectors) {
    data[dataCollector->dataCollectorName()] = dataCollector->getLiveData();
  }
  return data;
}

nlohmann::json Devices::getLiveDataSerialized() const {
  nlohmann::json json;
  std::lock_guard<std::mutex> lock(
      const_cast<std::mutex &>(m_MutexDataCollectors));
  for (const auto &[deviceId, dataCollector] : m_DataCollectors) {
    json[std::to_string(deviceId)] = {
        {"name", dataCollector->dataCollectorName()},
        {"data", dataCollector->getSerializedLiveData()}};
  }
  return json;
}

nlohmann::json Devices::getLastTrialDataSerialized() const {
  nlohmann::json json;
  std::lock_guard<std::mutex> lock(
      const_cast<std::mutex &>(m_MutexDataCollectors));
  for (const auto &[deviceId, dataCollector] : m_DataCollectors) {
    json[std::to_string(deviceId)] = {
        {"name", dataCollector->dataCollectorName()},
        {"data", dataCollector->getTrialData().serialize()}};
  }
  return json;
}

std::map<std::string, data::TimeSeries>
Devices::deserializeData(const nlohmann::json &json) {
  auto data = std::map<std::string, data::TimeSeries>();
  for (const auto &[deviceIndex, deviceData] : json.items()) {
    std::string name = deviceData["name"];
    data[name] = data::TimeSeries(deviceData["data"]);
  }
  return data;
}