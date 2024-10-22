#include "Data/FixedTimeSeries.h"

using namespace STIMWALKER_NAMESPACE::data;

FixedTimeSeries::FixedTimeSeries(const std::chrono::microseconds &deltaTime)
    : m_DeltaTime(deltaTime), TimeSeries() {}

FixedTimeSeries::FixedTimeSeries(
    const std::chrono::system_clock::time_point &startingTime,
    const std::chrono::microseconds &deltaTime)
    : m_DeltaTime(deltaTime), TimeSeries(startingTime) {}

void FixedTimeSeries::add(const DataPoint &data) {
  m_Data.push_back(data::TimeSeriesData(m_DeltaTime * m_Data.size(), data));
}
