#include "Devices/Devices.h"

#include "Data/TimeSeries.h"
#include "Devices/Generic/DataCollector.h"
#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

void Devices::add(std::unique_ptr<Device> device) {
  std::string deviceName = device->deviceName();

  // Add the device to the device collection if it does not exist yet
  if (m_Devices.find(deviceName) != m_Devices.end()) {
    utils::Logger::getInstance().fatal("Device " + deviceName +
                                       " already exists");
    throw std::invalid_argument("Device " + deviceName + " already exists");
  }
  m_Devices[deviceName] = std::move(device);

  // If we can dynamic cast the device to a data collector, add it to the data
  // collector collection
  if (auto dataCollector =
          std::dynamic_pointer_cast<DataCollector>(m_Devices[deviceName])) {
    m_DataCollectors[deviceName] = dataCollector;
  }
}

Device &Devices::getDevice(const std::string &deviceName) {
  if (m_Devices.find(deviceName) == m_Devices.end()) {
    utils::Logger::getInstance().fatal("Device " + deviceName +
                                       " does not exist");
    throw std::invalid_argument("Device " + deviceName + " does not exist");
  }
  return *m_Devices.at(deviceName);
}

const Device &Devices::getDevice(const std::string &deviceName) const {
  return getDevice(deviceName);
}

DataCollector &Devices::getDataCollector(const std::string &deviceName) {
  if (m_DataCollectors.find(deviceName) == m_DataCollectors.end()) {
    utils::Logger::getInstance().fatal("Data collector " + deviceName +
                                       " does not exist");
    throw std::invalid_argument("Data collector " + deviceName +
                                " does not exist");
  }
  return *m_DataCollectors.at(deviceName);
}

const DataCollector &
Devices::getDataCollector(const std::string &deviceName) const {
  return getDataCollector(deviceName);
}

nlohmann::json Devices::serialize() const {
  nlohmann::json json;
  for (const auto &[deviceName, dataCollector] : m_DataCollectors) {
    json[deviceName] = dataCollector->getTrialData().serialize();
  }
  return json;
}
