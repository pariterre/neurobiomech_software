#include "Devices/Data/DataDevices.h"

#include "Devices/Data/TimeSeries.h"

using namespace STIMWALKER_NAMESPACE::devices;

size_t DataDevices::length() const {
  return static_cast<int>(m_AllDevices.size());
}

void DataDevices::newDevice(const std::string &deviceName) {
  if (m_AllDevices.find(deviceName) != m_AllDevices.end()) {
    throw std::invalid_argument("Device already exists");
  }

  m_AllDevices[deviceName] = std::vector<TimeSeries>();
}

std::vector<TimeSeries> &DataDevices::device(const std::string &deviceName) {
  return m_AllDevices.at(deviceName);
}

nlohmann::json DataDevices::serialize() const {
  nlohmann::json json;
  for (const auto &[deviceName, deviceData] : m_AllDevices) {
    json[deviceName] = nlohmann::json::array();
    for (const auto &data : deviceData) {
      json[deviceName].push_back(data.serialize());
    }
  }
  return json;
}

DataDevices DataDevices::deserialize(const nlohmann::json &json) {
  DataDevices data;
  for (const auto &[deviceName, deviceData] : json.items()) {
    data.newDevice(deviceName);
    auto &device = data.device(deviceName);
    for (const auto &point : deviceData) {
      device.push_back(TimeSeries::deserialize(point));
    }
  }
  return data;
}
