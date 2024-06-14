#ifndef DEVICE_DATA_H
#define DEVICE_DATA_H

#include "stimwalkerConfig.h"
#include "nlohmann/json.hpp"

#include <map>

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

#endif // DEVICE_DATA_H