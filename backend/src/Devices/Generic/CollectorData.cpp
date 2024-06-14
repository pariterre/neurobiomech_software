#include "Devices/Generic/CollectorData.h"

namespace STIMWALKER_NAMESPACE{ namespace devices {

CollectorData::CollectorData(
    double timestamp,
    const std::vector<double>& data
) : m_timestamp(timestamp),
    m_data(data) {
}

double CollectorData::getTimestamp() const {
    return m_timestamp;
}

const std::vector<double>& CollectorData::getData() const {
    return m_data;
}

double CollectorData::getData(int channel) const {
    return m_data[channel];
}

int CollectorData::getNbChannels() const {
    return m_data.size();
}

}}