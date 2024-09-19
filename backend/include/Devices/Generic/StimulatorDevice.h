#ifndef __STIMWALKER_DEVICES_GENERIC_STIMULATOR_DEVICE_H__
#define __STIMWALKER_DEVICES_GENERIC_STIMULATOR_DEVICE_H__

#include "stimwalkerConfig.h"
#include <vector>

namespace STIMWALKER_NAMESPACE::devices
{

    class StimulatorData;

    class StimulatorDevice
    {
    public:
        /// @brief Destructor
        virtual ~StimulatorDevice() = default;

        /// @brief Get the number of channels
        /// @return The number of channels
        virtual int getNbChannels() const = 0;

        /// @brief Perform a stimulation
        virtual void stimulate() = 0;

        /// @brief Set the callback function to call when a stimulation is performed
        /// @param onStimulate The callback function
        virtual void listenToOnStimulate(void *onStimulate(const StimulatorData &newData)) = 0;

        /// @brief Get the data
        /// @return The data
        virtual std::vector<StimulatorData> getData() const = 0;
    };
}

#endif // __STIMWALKER_DEVICES_GENERIC_STIMULATOR_DEVICE_H__