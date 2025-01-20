#include "Data/FixedTimeSeries.h"

using namespace NEUROBIO_NAMESPACE::data;

FixedTimeSeries::FixedTimeSeries(const std::chrono::microseconds &deltaTime)
    : m_DeltaTime(deltaTime), TimeSeries() {}

FixedTimeSeries::FixedTimeSeries(
    const std::chrono::system_clock::time_point &startingTime,
    const std::chrono::microseconds &deltaTime)
    : m_DeltaTime(deltaTime), TimeSeries(startingTime) {}

void FixedTimeSeries::add(const std::chrono::microseconds &timeStamp,
                          const std::vector<double> &data) {
  TimeSeries::add(timeStamp, data);
}

void FixedTimeSeries::add(const std::vector<double> &data) {
  TimeSeries::add(m_DeltaTime * m_Data.size(), data);
}
