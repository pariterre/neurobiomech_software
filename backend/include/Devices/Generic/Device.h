#ifndef DEVICE_ABSTRACT_H
#define DEVICE_ABSTRACT_H

#include "stimwalkerConfig.h"

namespace STIMWALKER_NAMESPACE{ namespace devices {

/// @brief Abstract class for devices
class Device {
public:
    /// @brief Destructor
    virtual ~Device() = default;

    /// @brief Get the number of channels
    virtual bool getIsConnected() const = 0;

    /// @brief Connect to the actual device
    virtual void connect() = 0;

    /// @brief Disconnect from the actual device
    virtual void disconnect() = 0;

    /// @brief Dispose the device (free resources)
    virtual void dispose() = 0;
};

}}

#endif // DEVICE_ABSTRACT_H