#ifndef __STIMWALKER_DEVICES_DATA_DATA_H__
#define __STIMWALKER_DEVICES_DATA_DATA_H__

#include <map>
#include <nlohmann/json.hpp>

#include "stimwalkerConfig.h"

namespace STIMWALKER_NAMESPACE::devices
{

    /// @brief Class to store data
    class Data
    {
    public:
        /// @brief Destructor
        virtual ~Data() = default;

        /// @brief Get the timestamp
        /// @return The timestamp
        virtual time_t getTimestamp() const = 0;

        // @brief Get the data
        /// @return The data
        virtual const std::vector<double> &getData() const = 0;

        /// @brief Convert the object to JSON
        /// @return The JSON object
        virtual nlohmann::json serialize() const = 0;

        /// @brief Deserialize the object
        /// @param json The JSON object
        virtual void deserialize(const nlohmann::json &json) = 0;

        /// @brief Get a unique_ptr of the current object
        /// @return A unique_ptr of the current object
        virtual std::unique_ptr<Data> clone() const = 0;
    };
}

#endif // __STIMWALKER_DEVICES_DATA_DATA_H__