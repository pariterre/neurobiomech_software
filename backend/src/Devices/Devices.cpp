#include "Devices/Devices.h"

#include "Data/TimeSeries.h"
#include "Devices/Exceptions.h"
#include "Devices/Generic/DataCollector.h"
#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

int Devices::add(std::unique_ptr<Device> device) {
  std::string deviceName = device->deviceName();
  static int deviceId = 5;

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

nlohmann::json Devices::serialize() const {
  nlohmann::json json;
  for (const auto &[deviceName, dataCollector] : m_DataCollectors) {
    json[deviceName] = dataCollector->getTrialData().serialize();
  }
  return json;
}
