#include "Devices/Devices.h"

#include "Data/TimeSeries.h"
#include "Devices/Exceptions.h"
#include "Devices/Generic/AsyncDataCollector.h"
#include "Devices/Generic/AsyncDevice.h"
#include "Devices/Generic/DataCollector.h"
#include "Utils/Logger.h"
#include <thread>

using namespace STIMWALKER_NAMESPACE;
using namespace STIMWALKER_NAMESPACE::devices;

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
  if (auto dataCollector =
          std::dynamic_pointer_cast<DataCollector>(m_Devices[deviceId])) {
    m_DataCollectors[deviceId] = dataCollector;
  }

  return deviceId++;
}

void Devices::remove(size_t deviceId) {
  m_Devices.erase(deviceId);
  m_DataCollectors.erase(deviceId);
}

size_t Devices::size() const { return m_Devices.size(); }

void Devices::clear() {
  if (m_IsConnected) {
    disconnect();
  }

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
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
  if (m_IsStreamingData) {
    stopDataStreaming();
  }

  bool allDisconnected = true;
  for (auto &[deviceId, device] : m_Devices) {
    allDisconnected = allDisconnected && device->disconnect();
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
  m_IsStreamingData = true;
  pauseRecording();

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

  // Wait for all the devices to start streaming data (or fail to)
  size_t hasStartedStreaming = 0;
  size_t hasFailedToStartStreaming = 0;
  while (true) {
    hasStartedStreaming = 0;
    hasFailedToStartStreaming = 0;

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

    // If we get here, we have to give more time for the devices to connect
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  // If any of the devices failed to start streaming data, stop all the devices
  if (hasFailedToStartStreaming > 0) {
    stopDataStreaming();
    m_IsStreamingData = false;
    utils::Logger::getInstance().fatal("One or more devices failed to start "
                                       "streaming data, stopping all devices");
    return false;
  }

  // If all the devices are ready, reset the time series for now and set the
  // starting time to now
  for (auto &[deviceId, dataCollector] : m_DataCollectors) {
    dataCollector->m_TimeSeries->reset();
  }

  utils::Logger::getInstance().info("All devices are now streaming data");
  m_IsStreamingData = true;
  return true;
}

bool Devices::stopDataStreaming() {
  // Put all the devices in pause mode as it is faster than stopping them
  for (auto &[deviceId, dataCollector] : m_DataCollectors) {
    dataCollector->pauseRecording();
  }

  bool allStopped = true;
  for (auto &[deviceId, dataCollector] : m_DataCollectors) {
    allStopped = allStopped && dataCollector->stopDataStreaming();
  }

  utils::Logger::getInstance().info("All devices have stopped streaming data");
  m_IsStreamingData = !allStopped;
  return allStopped;
}

void Devices::pauseRecording() {
  for (auto &[deviceId, dataCollector] : m_DataCollectors) {
    dataCollector->pauseRecording();
  }

  m_IsPaused = true;
  utils::Logger::getInstance().info("All devices have paused recording");
}

void Devices::resumeRecording() {
  for (auto &[deviceId, dataCollector] : m_DataCollectors) {
    dataCollector->resumeRecording();
  }

  m_IsPaused = false;
  utils::Logger::getInstance().info("All devices have resumed recording");
}

nlohmann::json Devices::serialize() const {
  nlohmann::json json;
  for (const auto &[deviceName, dataCollector] : m_DataCollectors) {
    json[deviceName] = dataCollector->getTimeSeries().serialize();
  }
  return json;
}
