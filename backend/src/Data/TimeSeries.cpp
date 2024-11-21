#include "Data/TimeSeries.h"

using namespace STIMWALKER_NAMESPACE;
using namespace STIMWALKER_NAMESPACE::data;

TimeSeries::TimeSeries(const nlohmann::json &json)
    : m_StartingTime(std::chrono::system_clock::duration(
          json["startingTime"].get<int64_t>())),
      m_StopWatch(std::chrono::high_resolution_clock::now()),
      m_Data(utils::RollingVector<DataPoint>(
          static_cast<size_t>(json["data"].size()))) {
  for (const auto &point : json["data"]) {
    m_Data.push_back(std::move(
        DataPoint(std::chrono::microseconds(point[0].get<int>()), point[1])));
  }
}

size_t TimeSeries::size() const { return static_cast<int>(m_Data.size()); }

void TimeSeries::setRollingVectorMaxSize(size_t maxSize) {
  m_Data.setMaxSize(maxSize);
}

void TimeSeries::clear() { m_Data.clear(); }

void TimeSeries::add(const std::chrono::microseconds &timeStamp,
                     const std::vector<double> &data) {
  m_Data.push_back(std::move(DataPoint(timeStamp, zeroLevelData(data))));
}

void TimeSeries::add(const std::vector<double> &data) {
  m_Data.push_back(std::move(
      DataPoint(std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - m_StopWatch),
                zeroLevelData(data))));
}

const DataPoint &TimeSeries::operator[](size_t index) const {
  return m_Data.at(index);
}

TimeSeries TimeSeries::tail(size_t n) const {
  TimeSeries data(m_StartingTime);
  for (size_t i = m_Data.size() - n; i < m_Data.size(); i++) {
    data.m_Data.push_back(std::move(m_Data[i]));
  }
  return data;
}

const DataPoint &TimeSeries::front() const { return m_Data.front(); }

const DataPoint &TimeSeries::back() const { return m_Data.back(); }

TimeSeries
TimeSeries::since(const std::chrono::system_clock::time_point &time) const {
  TimeSeries data(m_StartingTime);
  for (const auto &point : m_Data) {
    if (m_StartingTime + point.getTimeStamp() >= time) {
      data.m_Data.push_back(std::move(point));
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
    jsonData.push_back(std::move(point.serialize()));
  }
  return json;
}

void TimeSeries::setZeroLevel(const std::chrono::milliseconds &duration) {
  if (m_Data.size() == 0) {
    // We cannot set the zero level if there is no data
    return;
  }

  auto oldZeroLevel = m_ZeroLevel;
  if (oldZeroLevel.size() == 0) {
    oldZeroLevel = std::vector<double>(m_Data[0].getData().size());
  }
  auto newZeroLevel = std::vector<double>(m_Data[0].getData().size());

  int dataCount = 0;
  auto first = m_Data.back().getTimeStamp() - duration;
  for (auto &data : m_Data) {
    if (data.getTimeStamp() < first) {
      continue;
    }
    for (size_t i = 0; i < data.getData().size(); i++) {
      // We add the old zero so we zero out from the actual collected values
      newZeroLevel[i] += data.getData()[i] + oldZeroLevel[i];
    }
    dataCount++;
  }

  for (size_t i = 0; i < m_ZeroLevel.size(); i++) {
    m_ZeroLevel[i] = newZeroLevel[i] / static_cast<double>(dataCount);
  }
}

std::vector<double>
TimeSeries::zeroLevelData(const std::vector<double> &data) const {
  std::vector<double> zeroLevelledData(data);
  if (m_ZeroLevel.size() == 0) {
    return zeroLevelledData;
  }

  for (size_t i = 0; i < data.size(); i++) {
    zeroLevelledData[i] -= m_ZeroLevel[i];
  }
  return zeroLevelledData;
}

void TimeSeries::reset() {
  m_Data.clear();
  m_StartingTime = std::chrono::system_clock::now();
  m_StopWatch = std::chrono::high_resolution_clock::now();
}
