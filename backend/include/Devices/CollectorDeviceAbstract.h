#ifndef COLLECTOR_DEVICE_ABSTRACT_H
#define COLLECTOR_DEVICE_ABSTRACT_H

#include <vector>

namespace STIMWALKER_NAMESPACE{ 

namespace Devices {
class CollectorData;

/// @brief Abstract class for data collectors
class CollectorDeviceAbstract {
public:
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
    /// @param onDataCollected The callback function
    virtual void listenToOnDataCollected(void* onDataCollected(const CollectorData& newData)) = 0;

    /// @brief Get the data
    /// @return The data
    virtual std::vector<CollectorData> getData() const = 0;

    /// @brief Get the data for a specific time index
    /// @param index The time index
    virtual CollectorData getData(int index) const = 0;

};  

}}

#endif // COLLECTOR_DEVICE_ABSTRACT_H
