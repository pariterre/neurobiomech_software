#include "Data/DataPoint.h"

using namespace STIMWALKER_NAMESPACE::data;

size_t DataPoint::size() const { return m_Data.size(); }

double DataPoint::operator[](size_t index) const { return m_Data[index]; }

nlohmann::json DataPoint::serialize() const {
  nlohmann::json json;
  json["timestamp"] = m_Timestamp.count();
  json["data"] = m_Data;
  return json;
}

DataPoint DataPoint::deserialize(const nlohmann::json &json) {
  return DataPoint(
      std::chrono::microseconds(json.at("timestamp").get<int64_t>()),
      json.at("data").get<std::vector<double>>());
}

DataPoint DataPoint::copy() const { return DataPoint(m_Timestamp, m_Data); }