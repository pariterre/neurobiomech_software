#include "Devices/Data/Data.h"

using namespace STIMWALKER_NAMESPACE::devices;

Data::Data(time_t timestamp, const std::vector<double> &data)
    : m_Timestamp(timestamp), m_Data(data) {}

nlohmann::json Data::serialize() const {
  nlohmann::json json;
  json["timestamp"] = m_Timestamp;
  json["data"] = m_Data;
  return json;
}

Data Data::deserialize(const nlohmann::json &json) {
  time_t timestamp = json.at("timestamp").get<time_t>();
  std::vector<double> data = json.at("data").get<std::vector<double>>();
  return Data(timestamp, data);
}

Data Data::copy() const { return Data(m_Timestamp, m_Data); }