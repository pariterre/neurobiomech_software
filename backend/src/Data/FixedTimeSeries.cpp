#include "Data/FixedTimeSeries.h"

using namespace STIMWALKER_NAMESPACE::data;

FixedTimeSeries::FixedTimeSeries(const std::chrono::microseconds &deltaTime)
    : m_DeltaTime(deltaTime), m_StartingTime(std::chrono::system_clock::now()),
      TimeSeries() {}

FixedTimeSeries::FixedTimeSeries(const std::chrono::milliseconds &deltaTime)
    : m_DeltaTime(deltaTime), m_StartingTime(std::chrono::system_clock::now()),
      TimeSeries() {}

void FixedTimeSeries::add(const DataPoint &data) {
  if (m_Data.empty()) {
    m_Data.push_back(DataPoint(m_StartingTime, data.getData()));
  } else {
    auto lastTimestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        m_Data.back().getTimestamp().time_since_epoch());
    m_Data.push_back(DataPoint(lastTimestamp + m_DeltaTime, data.getData()));
  }
}

void FixedTimeSeries::add(const std::vector<double> &data) {
  if (m_Data.empty()) {
    m_Data.push_back(DataPoint(m_StartingTime, data));
  } else {
    auto lastTimestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        m_Data.back().getTimestamp().time_since_epoch());
    m_Data.push_back(DataPoint(lastTimestamp + m_DeltaTime, data));
  }
}

void FixedTimeSeries::setStartingTime(
    const std::chrono::system_clock::time_point &time) {
  m_StartingTime = time;
  if (m_Data.empty()) {
    return;
  }

  // We have to const cast the timestamp to modify it
  const_cast<std::chrono::system_clock::time_point &>(
      m_Data[0].getTimestamp()) = time;

  for (size_t i = 1; i < m_Data.size(); i++) {
    const_cast<std::chrono::system_clock::time_point &>(
        m_Data[i].getTimestamp()) =
        std::chrono::system_clock::time_point(m_StartingTime +
                                              (m_DeltaTime * i));
  }
}