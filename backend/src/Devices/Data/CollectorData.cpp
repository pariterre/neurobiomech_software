#include "Devices/Data/CollectorData.h"

using namespace STIMWALKER_NAMESPACE::devices;

CollectorData::CollectorData(
    time_t timestamp,
    const std::vector<double> &data) : m_timestamp(timestamp),
                                       m_data(data)
{
}

time_t CollectorData::getTimestamp() const
{
    return m_timestamp;
}

const std::vector<double> &CollectorData::getData() const
{
    return m_data;
}

double CollectorData::getData(int channel) const
{
    return m_data[channel];
}

int CollectorData::getNbChannels() const
{
    return m_data.size();
}

nlohmann::json CollectorData::serialize() const
{
    nlohmann::json json;
    json["timestamp"] = m_timestamp;
    json["data"] = m_data;
    return json;
}

void CollectorData::deserialize(const nlohmann::json &json)
{
    m_timestamp = json["timestamp"];
    m_data = json["data"].get<std::vector<double>>();
}

std::unique_ptr<Data> CollectorData::clone() const
{
    return std::make_unique<CollectorData>(m_timestamp, m_data);
}
