#include "Data/DataPoint.h"

using namespace STIMWALKER_NAMESPACE::data;

size_t DataPoint::size() const { return m_Data.size(); }

const double &DataPoint::operator[](size_t index) const {
  return m_Data.at(index);
}

nlohmann::json DataPoint::serialize() const { return nlohmann::json(m_Data); }

DataPoint DataPoint::deserialize(const nlohmann::json &json) {
  return DataPoint(json.get<std::vector<double>>());
}
