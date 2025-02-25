#include "Data/DataPoint.h"

using namespace NEUROBIO_NAMESPACE::data;

ExtraInfo parseExtraInfo(const nlohmann::json &json) {
  ExtraInfo extraInfo;

  for (const auto &[key, value] : json.items()) {
    if (value.is_number_integer()) {
      extraInfo[key] = value.get<int>();
    } else if (value.is_number_float()) {
      extraInfo[key] = value.get<double>();
    } else if (value.is_boolean()) {
      extraInfo[key] = value.get<bool>();
    } else if (value.is_string()) {
      extraInfo[key] = value.get<std::string>();
    } else {
      throw std::runtime_error("Unsupported type in ExtraInfo for key: " + key);
    }
  }

  return extraInfo;
}

nlohmann::json serializeExtraInfo(const ExtraInfo &extraInfo) {
  nlohmann::json j;
  for (const auto &[key, value] : extraInfo) {
    j[key] = std::visit([](const auto &val) -> nlohmann::json { return val; },
                        value);
  }
  return j;
}

DataPoint::DataPoint()
    : m_TimeStamp(std::chrono::microseconds(0)), m_Data({}) {};

DataPoint::DataPoint(const std::chrono::microseconds &timeStamp,
                     const std::vector<double> &data,
                     const ExtraInfo &extraInfo)
    : m_TimeStamp(timeStamp), m_Data(data), m_ExtraInfo(extraInfo) {}

DataPoint::DataPoint(const nlohmann::json &json)
    : m_TimeStamp(json.at(0).get<int64_t>()),
      m_Data(json.at(1).get<std::vector<double>>()),
      m_ExtraInfo(parseExtraInfo(json.at(2))) {}

size_t DataPoint::size() const { return m_Data.size(); }

double DataPoint::operator[](size_t index) const { return m_Data.at(index); }

nlohmann::json DataPoint::serialize() const {
  return nlohmann::json(
      {m_TimeStamp.count(), m_Data, serializeExtraInfo(m_ExtraInfo)});
}
