#include "Devices/Data/DataCollection.h"

#include "Devices/Data/Data.h"
#include "Utils/String.h"
#include "Devices/Data/CollectorData.h"

using namespace STIMWALKER_NAMESPACE::devices;

int DataCollection::registerNewDataId()
{
    static int dataId = 0;
    return dataId++;
}

int DataCollection::getNbDataId() const
{
    return m_data.size();
}

const std::vector<std::unique_ptr<Data>> &DataCollection::getData(int dataId) const
{
    return m_data.at(dataId);
}

void DataCollection::addData(int dataId, const Data &data)
{
    m_data[dataId].push_back(data.clone());
}

nlohmann::json DataCollection::serialize() const
{
    nlohmann::json json;
    for (const auto &[dataId, timeData] : m_data)
    {
        // Add an array with the name dataId
        auto dataIdString = STIMWALKER_NAMESPACE::utils::String(dataId);
        json[dataIdString.c_str()] = nlohmann::json::array();
        for (const auto &data : timeData)
        {
            json[dataIdString.c_str()].push_back(data->serialize());
        }
    }
    return json;
}

DataCollection DataCollection::deserialize(const nlohmann::json &json)
{
    DataCollection dataCollection;
    int dataId = 0;

    for (const auto &dataIdString : json)
    {
        for (const auto &data : dataIdString)
        {
            std::vector<double> dataValue = data["data"];
            time_t timestamp = data["timestamp"];
            devices::CollectorData donnees(timestamp, dataValue);
            dataCollection.addData(dataId, donnees);
        }
        dataId++;
    }

    return dataCollection;
}
