#ifndef STIMULATOR_DEVICE_ABSTRACT_H
#define STIMULATOR_DEVICE_ABSTRACT_H

#include <vector>

namespace STIMWALKER_NAMESPACE{ namespace Devices {

class StimulatorData;

class StimulatorDeviceAbstract{
public:
    /// @brief Get the number of channels
    /// @return The number of channels
    virtual int getNbChannels() const = 0;

    /// @brief Perform a stimulation
    virtual void stimulate() = 0;

    /// @brief Set the callback function to call when a stimulation is performed
    /// @param onStimulate The callback function
    virtual void listenToOnStimulate(void* onStimulate(const StimulatorData& newData)) = 0;

    /// @brief Get the data
    /// @return The data
    virtual std::vector<StimulatorData> getData() const = 0;

};

}}

#endif // STIMULATOR_DEVICE_ABSTRACT_H