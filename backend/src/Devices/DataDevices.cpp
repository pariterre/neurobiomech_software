#include "Devices/DataDevices.h"

#include "Data/TimeSeries.h"

using namespace STIMWALKER_NAMESPACE::devices::data;

size_t DataDevices::size() const {
  return static_cast<int>(m_AllDevices.size());
}

void DataDevices::newDevice(const std::string &deviceName) {
  if (m_AllDevices.find(deviceName) != m_AllDevices.end()) {
    throw std::invalid_argument("Device already exists");
  }

  m_AllDevices[deviceName] = TimeSeries();
}

TimeSeries &DataDevices::device(const std::string &deviceName) {
  return m_AllDevices.at(deviceName);
}

TimeSeries &DataDevices::operator[](const std::string &deviceName) {
  return device(deviceName);
}

nlohmann::json DataDevices::serialize() const {
  nlohmann::json json;
  for (const auto &[deviceName, timeSeries] : m_AllDevices) {
    json[deviceName] = timeSeries.serialize();
  }
  return json;
}

DataDevices DataDevices::deserialize(const nlohmann::json &json) {
  DataDevices data;
  for (const auto &[deviceName, timeSeries] : json.items()) {
    data[deviceName] = TimeSeries::deserialize(timeSeries);
  }
  return data;
}
