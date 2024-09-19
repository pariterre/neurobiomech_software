#ifndef __STIMWALKER_DEVICES_DEVICE_DATA_DATA_COLLECTION_H__
#define __STIMWALKER_DEVICES_DEVICE_DATA_DATA_COLLECTION_H__

#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>

#include "stimwalkerConfig.h"

namespace STIMWALKER_NAMESPACE::devices
{
    class Data;
    class Device;

    /// @brief Class to store data
    class DataCollection
    {
    public:
        /// @brief Register new data to the collection
        /// @return The dataId of the new data
        int registerNewDataId();

        /// @brief The number of data registered in the collection
        /// @return The number of data registered in the collection
        int getNbDataId() const;

        /// @brief Get the data
        /// @param dataId The id of the data
        /// @return The data
        const std::vector<std::unique_ptr<Data>> &getData(int dataId) const;

        /// @brief Add new data to the collection
        /// @param data The data
        void addData(int dataId, const Data &data);

        /// @brief Get the data in serialized form
        /// @return The data in serialized form
        nlohmann::json serialize() const;

        /// @brief Deserialize the data
        /// @param json The data in serialized form
        DataCollection deserialize(const nlohmann::json &json);

    protected:
        std::map<int, std::vector<std::unique_ptr<Data>>> m_data; ///< Data of the collection, indexed by id of the device
    };
}

#endif // __STIMWALKER_DEVICES_DEVICE_DATA_DATA_COLLECTION_H__