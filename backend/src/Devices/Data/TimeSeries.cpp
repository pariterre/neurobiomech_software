#include "Devices/Data/TimeSeries.h"

#include "Devices/Data/DataPoint.h"
#include "Utils/String.h"

using namespace STIMWALKER_NAMESPACE::devices;

int TimeSeries::dataCount() const { return static_cast<int>(m_Data.size()); }

void TimeSeries::add(const DataPoint &data) { m_Data.push_back(data); }

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
    time_t timestamp = point.at("timestamp").get<time_t>();
    std::vector<double> dataValue = point.at("data").get<std::vector<double>>();
    data.add(DataPoint(timestamp, dataValue));
  }
  return data;
}
