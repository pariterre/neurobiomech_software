#ifndef COLLECTOR_DEVICE_ABSTRACT_H
#define COLLECTOR_DEVICE_ABSTRACT_H

#include "stimwalkerConfig.h"
#include <functional>
#include <vector>

namespace STIMWALKER_NAMESPACE::devices
{
    class CollectorData;

    /// @brief Abstract class for data collectors
    class Collector
    {
    public:
        /// @brief Destructor
        virtual ~Collector() = default;

        /// @brief Get the number of channels
        /// @return The number of channels
        virtual int getNbChannels() const = 0;

        /// @brief Get the frame rate
        /// @return The frame rate
        virtual int getFrameRate() const = 0;

        /// @brief Check if the device is currently recording
        virtual bool isRecording() const = 0;

        /// @brief Start collecting data
        virtual void startRecording() = 0;

        /// @brief Stop collecting data
        virtual void stopRecording() = 0;

        /// @brief Set the callback function to call when data is collected
        /// @param callback The callback function
        /// @return A unique identifier for the listener so it can be removed later
        virtual int onNewData(
            std::function<void(const CollectorData &newData)> callback) = 0;

        /// @brief Remove a listener
        /// @param listenerId The unique identifier of the listener
        virtual void removeListener(int listenerId) = 0;

        /// @brief Get the data
        /// @return The data
        virtual std::vector<CollectorData> getData() const = 0;

        /// @brief Get the data for a specific time index
        /// @param index The time index
        virtual CollectorData getData(int index) const = 0;
    };

}

#endif // COLLECTOR_DEVICE_ABSTRACT_H
