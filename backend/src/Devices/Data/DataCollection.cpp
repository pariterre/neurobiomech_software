#include "Devices/Data/DataCollection.h"

#include "Devices/Data/CollectorData.h"
#include "Devices/Data/Data.h"
#include "Utils/String.h"

using namespace STIMWALKER_NAMESPACE::devices;

int DataCollection::registerNewDataId()
{
    static int dataId = 0;
    return dataId++;
}

int DataCollection::getNbDataId() const
{
    return static_cast<int>(m_data.size());
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

    // Iterate over key-value pairs in the json object
    for (const auto &device : json.items())
    {
        for (const auto &dataPoint : device.value())
        {
            // Extract "data" and "timestamp" from each object
            std::vector<double> dataValue = dataPoint.at("data").get<std::vector<double>>();
            time_t timestamp = dataPoint.at("timestamp").get<time_t>();

            // Create CollectorData object and add it to dataCollection
            devices::CollectorData donnees(timestamp, dataValue);
            dataCollection.addData(dataId, donnees);
        }

        // Increment the dataId after processing each key
        dataId++;
    }

    return dataCollection;
}
