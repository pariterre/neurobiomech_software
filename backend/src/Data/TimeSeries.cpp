#include "Data/TimeSeries.h"

using namespace STIMWALKER_NAMESPACE::data;

size_t TimeSeries::size() const { return static_cast<int>(m_Data.size()); }

void TimeSeries::clear() { m_Data.clear(); }

void TimeSeries::add(DataPoint &data) {
  if (data.m_Timestamp.count() == -1) {
    data.m_Timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now() - m_StartingTime);
  }
  m_Data.push_back(data);
}

const DataPoint &TimeSeries::operator[](size_t index) const {
  return m_Data[index];
}

nlohmann::json TimeSeries::serialize() const {
  nlohmann::json json = nlohmann::json::array();
  for (const auto &data : m_Data) {
    json.push_back(data.serialize());
  }
  return json;
}

TimeSeries TimeSeries::deserialize(const nlohmann::json &json) {
  TimeSeries data;
  for (const auto &point : json) {
    data.add(DataPoint::deserialize(point));
  }
  return data;
}

void TimeSeries::reset() {
  m_Data.clear();
  m_StartingTime = std::chrono::system_clock::now();
}
