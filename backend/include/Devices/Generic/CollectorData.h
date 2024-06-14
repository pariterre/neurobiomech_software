#ifndef COLLECTOR_DATA_H
#define COLLECTOR_DATA_H

#include "stimwalkerConfig.h"
#include <ctime>
#include <vector>

namespace STIMWALKER_NAMESPACE::devices
{
    /// @brief Data collected by a Collector device
    class CollectorData
    {
    public:
        /// @brief Constructor
        /// @param timestamp The timestamp of the data
        /// @param data The data
        CollectorData(
            double timestamp,
            const std::vector<double> &data);

        /// @brief Destructor
        virtual ~CollectorData() = default;

        /// @brief Get the timestamp
        /// @return The timestamp
        double getTimestamp() const;

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

    protected:
        std::time_t m_timestamp;    ///< Timestamp of the data
        std::vector<double> m_data; ///< Data
    };

}

#endif // COLLECTOR_DATA_H
