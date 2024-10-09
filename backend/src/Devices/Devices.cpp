#include "Devices/Devices.h"

#include "Data/TimeSeries.h"
#include "Devices/Generic/DataCollector.h"
#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

void Devices::add(Device &device) {
  std::string deviceName = device.deviceName();

  // Add the device to the device collection if it does not exist yet
  if (m_Devices.find(deviceName) != m_Devices.end()) {
    utils::Logger::getInstance().fatal("Device " + deviceName +
                                       " already exists");
    throw std::invalid_argument("Device " + deviceName + " already exists");
  }
  m_Devices[deviceName] = device;
}

void Devices::add(DataCollector &dataCollector) {
  std::string dataCollectorName = dataCollector.dataCollectorName();

  // Add the data collector to the data collector collection if it does not
  // exist yet
  if (m_DataCollectors.find(dataCollectorName) != m_DataCollectors.end()) {
    utils::Logger::getInstance().fatal("Data collector " + dataCollectorName +
                                       " already exists");
    throw std::invalid_argument("Data collector " + dataCollectorName +
                                " already exists");
  }
  m_DataCollectors[dataCollectorName] = dataCollector;
}

nlohmann::json Devices::serialize() const {
  nlohmann::json json;
  for (const auto &[deviceName, dataCollector] : m_DataCollectors) {
    json[deviceName] = dataCollector.getTrialData().serialize();
  }
  return json;
}
