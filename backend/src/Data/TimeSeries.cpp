#include "Data/TimeSeries.h"

using namespace STIMWALKER_NAMESPACE;
using namespace STIMWALKER_NAMESPACE::data;

TimeSeries::TimeSeries(const nlohmann::json &json)
    : m_StartingTime(std::chrono::system_clock::duration(
          json["startingTime"].get<int64_t>())),
      m_StopWatch(std::chrono::high_resolution_clock::now()),
      m_Data(utils::RollingVector<data::TimeSeriesData>(
          static_cast<size_t>(json["data"].size()))) {
  for (const auto &point : json["data"]) {
    m_Data.push_back(data::TimeSeriesData(
        std::chrono::microseconds(point[0].get<int>()), DataPoint(point[1])));
  }
}

size_t TimeSeries::size() const { return static_cast<int>(m_Data.size()); }

void TimeSeries::clear() { m_Data.clear(); }

void TimeSeries::add(const DataPoint &data) {
  m_Data.push_back(data::TimeSeriesData(
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now() - m_StopWatch),
      data));
}

void TimeSeries::add(const std::chrono::microseconds &timeStamp,
                     const DataPoint &data) {
  m_Data.push_back(data::TimeSeriesData(timeStamp, data));
}

const std::pair<std::chrono::microseconds, DataPoint> &
TimeSeries::operator[](size_t index) const {
  return m_Data.at(index);
}

TimeSeries TimeSeries::tail(size_t n) const {
  TimeSeries data(m_StartingTime);
  for (size_t i = m_Data.size() - n; i < m_Data.size(); i++) {
    data.add(m_Data[i].first, m_Data[i].second);
  }
  return data;
}

TimeSeries
TimeSeries::since(const std::chrono::system_clock::time_point &time) const {
  TimeSeries data(m_StartingTime);
  for (const auto &point : m_Data) {
    if (m_StartingTime + point.first >= time) {
      data.add(point.first, point.second);
    }
  }
  return data;
}

nlohmann::json TimeSeries::serialize() const {
  nlohmann::json json = nlohmann::json::object();
  json["startingTime"] = m_StartingTime.time_since_epoch().count();
  json["data"] = nlohmann::json::array();
  auto &jsonData = json["data"];
  for (const auto &point : m_Data) {
    jsonData.push_back({point.first.count(), point.second.serialize()});
  }
  return json;
}

void TimeSeries::reset() {
  m_Data.clear();
  m_StartingTime = std::chrono::system_clock::now();
  m_StopWatch = std::chrono::high_resolution_clock::now();
}

const utils::RollingVector<std::pair<std::chrono::microseconds, DataPoint>> &
TimeSeries::getData() const {
  return m_Data;
}
