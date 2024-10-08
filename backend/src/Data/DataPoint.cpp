#include "Data/DataPoint.h"

using namespace STIMWALKER_NAMESPACE::devices::data;

DataPoint::DataPoint(const std::vector<double> &data)
    : m_Timestamp(-1), m_Data(data) {}

DataPoint::DataPoint(time_t timestamp, const std::vector<double> &data)
    : m_Timestamp(timestamp), m_Data(data) {}

size_t DataPoint::size() const { return m_Data.size(); }

double DataPoint::operator[](size_t index) const { return m_Data[index]; }

nlohmann::json DataPoint::serialize() const {
  nlohmann::json json;
  json["timestamp"] = m_Timestamp;
  json["data"] = m_Data;
  return json;
}

DataPoint DataPoint::deserialize(const nlohmann::json &json) {
  time_t timestamp = json.at("timestamp").get<time_t>();
  std::vector<double> data = json.at("data").get<std::vector<double>>();
  return DataPoint(timestamp, data);
}

DataPoint DataPoint::copy() const { return DataPoint(m_Timestamp, m_Data); }