#include "Devices/Data/TimeSeriesData.h"

#include "Devices/Data/Data.h"
#include "Utils/String.h"

using namespace STIMWALKER_NAMESPACE::devices;

int TimeSeriesData::dataCount() const {
  return static_cast<int>(m_Data.size());
}

void TimeSeriesData::add(const Data &data) { m_Data.push_back(data); }

nlohmann::json TimeSeriesData::serialize() const {
  nlohmann::json json = nlohmann::json::array();
  for (const auto &data : m_Data) {
    json.push_back(data.serialize());
  }
  return json;
}

TimeSeriesData TimeSeriesData::deserialize(const nlohmann::json &json) {
  TimeSeriesData data;
  for (const auto &point : json) {
    time_t timestamp = point.at("timestamp").get<time_t>();
    std::vector<double> dataValue = point.at("data").get<std::vector<double>>();
    data.add(devices::Data(timestamp, dataValue));
  }
  return data;
}
