#include "Data/DataPoint.h"

using namespace STIMWALKER_NAMESPACE::data;

DataPoint::DataPoint(const std::vector<double> &data)
    : m_Timestamp(std::chrono::system_clock::now()), m_Data(data) {}

DataPoint::DataPoint(std::chrono::system_clock::time_point timestamp,
                     const std::vector<double> &data)
    : m_Timestamp(timestamp), m_Data(data) {}

DataPoint::DataPoint(std::chrono::milliseconds timestamp,
                     const std::vector<double> &data)
    : m_Timestamp(std::chrono::system_clock::time_point(timestamp)),
      m_Data(data) {}

DataPoint::DataPoint(std::chrono::microseconds timestamp,
                     const std::vector<double> &data)
    : m_Timestamp(std::chrono::system_clock::time_point(timestamp)),
      m_Data(data) {}

size_t DataPoint::size() const { return m_Data.size(); }

double DataPoint::operator[](size_t index) const { return m_Data[index]; }

nlohmann::json DataPoint::serialize() const {
  nlohmann::json json;
  json["timestamp"] = std::chrono::duration_cast<std::chrono::microseconds>(
                          m_Timestamp.time_since_epoch())
                          .count();
  json["data"] = m_Data;
  return json;
}

DataPoint DataPoint::deserialize(const nlohmann::json &json) {
  auto timestamp =
      std::chrono::microseconds(json.at("timestamp").get<int64_t>());
  std::vector<double> data = json.at("data").get<std::vector<double>>();
  return DataPoint(timestamp, data);
}

DataPoint DataPoint::copy() const { return DataPoint(m_Timestamp, m_Data); }