#include "Devices/DevicesData.h"

#include "Data/TimeSeries.h"

using namespace STIMWALKER_NAMESPACE::data;
using namespace STIMWALKER_NAMESPACE::devices;

size_t DevicesData::size() const {
  return static_cast<int>(m_AllDevices.size());
}

void DevicesData::newDevice(const std::string &deviceName) {
  if (m_AllDevices.find(deviceName) != m_AllDevices.end()) {
    throw std::invalid_argument("Device already exists");
  }

  m_AllDevices[deviceName] = TimeSeries();
}

TimeSeries &DevicesData::device(const std::string &deviceName) {
  return m_AllDevices.at(deviceName);
}

TimeSeries &DevicesData::operator[](const std::string &deviceName) {
  return device(deviceName);
}

nlohmann::json DevicesData::serialize() const {
  nlohmann::json json;
  for (const auto &[deviceName, timeSeries] : m_AllDevices) {
    json[deviceName] = timeSeries.serialize();
  }
  return json;
}

DevicesData DevicesData::deserialize(const nlohmann::json &json) {
  DevicesData data;
  for (const auto &[deviceName, timeSeries] : json.items()) {
    data[deviceName] = TimeSeries::deserialize(timeSeries);
  }
  return data;
}
