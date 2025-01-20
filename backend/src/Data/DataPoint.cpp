#include "Data/DataPoint.h"

using namespace NEUROBIO_NAMESPACE::data;

size_t DataPoint::size() const { return m_Data.size(); }

const double &DataPoint::operator[](size_t index) const {
  return m_Data.at(index);
}

nlohmann::json DataPoint::serialize() const {
  return nlohmann::json({m_TimeStamp.count(), m_Data});
}
