#include "Data/TimeSeries.h"

using namespace STIMWALKER_NAMESPACE::data;

size_t TimeSeries::size() const { return static_cast<int>(m_Data.size()); }

void TimeSeries::clear() { m_Data.clear(); }

void TimeSeries::add(const DataPoint &data) {
  m_Data.push_back(
      std::make_pair(std::chrono::duration_cast<std::chrono::microseconds>(
                         std::chrono::system_clock::now() - m_StartingTime),
                     data));
}

void TimeSeries::add(const std::chrono::microseconds &timeStamp,
                     const DataPoint &data) {
  m_Data.push_back(std::make_pair(timeStamp, data));
}

const std::pair<std::chrono::microseconds, DataPoint> &
TimeSeries::operator[](size_t index) const {
  return m_Data.at(index);
}

nlohmann::json TimeSeries::serialize() const {
  nlohmann::json json = nlohmann::json::array();
  for (const auto &point : m_Data) {
    json.push_back({point.first.count(), point.second.serialize()});
  }
  return json;
}

TimeSeries TimeSeries::deserialize(const nlohmann::json &json) {
  TimeSeries data;
  for (const auto &point : json) {
    data.add(std::chrono::microseconds(point[0].get<int>()),
             DataPoint::deserialize(point[1]));
  }
  return data;
}

void TimeSeries::reset() {
  m_Data.clear();
  m_StartingTime = std::chrono::system_clock::now();
}

const std::vector<std::pair<std::chrono::microseconds, DataPoint>> &
TimeSeries::getData() const {
  return m_Data;
}
