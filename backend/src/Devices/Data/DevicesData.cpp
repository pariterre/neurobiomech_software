#include "Devices/Data/DevicesData.h"

#include "Devices/Data/TimeSeriesData.h"

using namespace STIMWALKER_NAMESPACE::devices;

size_t DevicesData::length() const {
  return static_cast<int>(m_AllDevices.size());
}

void DevicesData::addDevice(const std::string &deviceName) {
  m_AllDevices[deviceName] = std::vector<TimeSeriesData>();
}

std::vector<TimeSeriesData> &
DevicesData::device(const std::string &deviceName) {
  return m_AllDevices.at(deviceName);
}

nlohmann::json DevicesData::serialize() const {
  nlohmann::json json;
  for (const auto &[deviceName, deviceData] : m_AllDevices) {
    json[deviceName] = nlohmann::json::array();
    for (const auto &data : deviceData) {
      json[deviceName].push_back(data.serialize());
    }
  }
  return json;
}

DevicesData DevicesData::deserialize(const nlohmann::json &json) {
  DevicesData data;
  for (const auto &[deviceName, deviceData] : json.items()) {
    data.addDevice(deviceName);
    auto &device = data.device(deviceName);
    for (const auto &point : deviceData) {
      device.push_back(TimeSeriesData::deserialize(point));
    }
  }
  return data;
}
