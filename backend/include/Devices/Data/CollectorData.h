#ifndef COLLECTOR_DATA_H
#define COLLECTOR_DATA_H

#include <ctime>
#include <vector>

#include "stimwalkerConfig.h"
#include "Devices/Data/Data.h"

namespace STIMWALKER_NAMESPACE::devices
{
    /// @brief Data collected by a Collector device
    class CollectorData : public Data
    {
    public:
        /// @brief Constructor
        /// @param timestamp The timestamp of the data
        /// @param data The data
        CollectorData(
            time_t timestamp,
            const std::vector<double> &data);

        /// @brief Destructor
        virtual ~CollectorData() = default;

        time_t getTimestamp() const override;

        /// @brief Get the data
        /// @return The data
        const std::vector<double> &getData() const;

        /// @brief Get the data for a specific channel
        /// @param channel The channel
        /// @return The data for the channel
        double getData(int channel) const;

        /// @brief Get the number of channels
        /// @return The number of channels
        int getNbChannels() const;

        nlohmann::json serialize() const override;

        void deserialize(const nlohmann::json &json) override;

        std::unique_ptr<Data> clone() const override;

    protected:
        std::time_t m_timestamp;    ///< Timestamp of the data
        std::vector<double> m_data; ///< Data
    };

}

#endif // COLLECTOR_DATA_H
