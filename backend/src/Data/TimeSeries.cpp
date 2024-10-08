#include "Data/TimeSeries.h"

using namespace STIMWALKER_NAMESPACE::data;

size_t TimeSeries::size() const { return static_cast<int>(m_Data.size()); }

void TimeSeries::clear() { m_Data.clear(); }

void TimeSeries::add(const DataPoint &data) { m_Data.push_back(data); }

void TimeSeries::add(const std::vector<double> &data) {
  m_Data.push_back(DataPoint(data));
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
