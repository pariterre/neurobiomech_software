#include "Data/FixedTimeSeries.h"

using namespace STIMWALKER_NAMESPACE::data;

FixedTimeSeries::FixedTimeSeries(const std::chrono::microseconds &deltaTime)
    : m_StartingTime(std::chrono::system_clock::now()), m_DeltaTime(deltaTime),
      TimeSeries() {}

FixedTimeSeries::FixedTimeSeries(const std::chrono::milliseconds &deltaTime)
    : m_StartingTime(std::chrono::system_clock::now()), m_DeltaTime(deltaTime),
      TimeSeries() {}

FixedTimeSeries::FixedTimeSeries(
    const std::chrono::system_clock::time_point &startingTime,
    const std::chrono::microseconds &deltaTime)
    : m_StartingTime(startingTime), m_DeltaTime(deltaTime), TimeSeries() {}

FixedTimeSeries::FixedTimeSeries(
    const std::chrono::system_clock::time_point &startingTime,
    const std::chrono::milliseconds &deltaTime)
    : m_StartingTime(startingTime), m_DeltaTime(deltaTime), TimeSeries() {}

FixedTimeSeries::FixedTimeSeries(const std::chrono::milliseconds &startingTime,
                                 const std::chrono::microseconds &deltaTime)
    : m_StartingTime(std::chrono::system_clock::time_point(startingTime)),
      m_DeltaTime(deltaTime), TimeSeries() {}

FixedTimeSeries::FixedTimeSeries(const std::chrono::milliseconds &startingTime,
                                 const std::chrono::milliseconds &deltaTime)
    : m_StartingTime(std::chrono::system_clock::time_point(startingTime)),
      m_DeltaTime(deltaTime), TimeSeries() {}
FixedTimeSeries::FixedTimeSeries(const std::chrono::microseconds &startingTime,
                                 const std::chrono::microseconds &deltaTime)
    : m_StartingTime(std::chrono::system_clock::time_point(startingTime)),
      m_DeltaTime(deltaTime), TimeSeries() {}

FixedTimeSeries::FixedTimeSeries(const std::chrono::microseconds &startingTime,
                                 const std::chrono::milliseconds &deltaTime)
    : m_StartingTime(std::chrono::system_clock::time_point(startingTime)),
      m_DeltaTime(deltaTime), TimeSeries() {}

void FixedTimeSeries::add(const DataPoint &data) {
  m_Data.push_back(DataPoint(m_StartingTime + (m_DeltaTime * m_Data.size()),
                             data.getData()));
}

void FixedTimeSeries::add(const std::vector<double> &data) {
  m_Data.push_back(
      DataPoint(m_StartingTime + (m_DeltaTime * m_Data.size()), data));
}

void FixedTimeSeries::setStartingTime(
    const std::chrono::system_clock::time_point &time) {
  m_StartingTime = time;

  // Retroactively set the time of the data relative to the new starting time
  for (size_t i = 0; i < m_Data.size(); i++) {
    // We have to const_cast the timestamp to modify it
    const_cast<std::chrono::system_clock::time_point &>(
        m_Data[i].getTimestamp()) =
        std::chrono::system_clock::time_point(m_StartingTime +
                                              (m_DeltaTime * i));
  }
}

void FixedTimeSeries::setStartingTime(const std::chrono::milliseconds &time) {
  setStartingTime(std::chrono::system_clock::time_point(time));
}

void FixedTimeSeries::setStartingTime(const std::chrono::microseconds &time) {
  setStartingTime(std::chrono::system_clock::time_point(time));
}