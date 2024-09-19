#ifndef __STIMWALKER_DEVICES_GENERIC_DEVICE_H__
#define __STIMWALKER_DEVICES_GENERIC_DEVICE_H__

#include "stimwalkerConfig.h"

namespace STIMWALKER_NAMESPACE::devices
{

    /// @brief Abstract class for devices
    class Device
    {
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

}

#endif // __STIMWALKER_DEVICES_GENERIC_DEVICE_H__