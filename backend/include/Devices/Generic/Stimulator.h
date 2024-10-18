#ifndef __STIMWALKER_DEVICES_GENERIC_STIMULATOR_H__
#define __STIMWALKER_DEVICES_GENERIC_STIMULATOR_H__

#include "stimwalkerConfig.h"
#include <vector>

#include "Utils/CppMacros.h"
#include "Utils/StimwalkerEvent.h"

namespace STIMWALKER_NAMESPACE::devices {
class DataPoint;

class Stimulator {
public:
  /// @brief Destructor
  virtual ~Stimulator() = default;

  /// @brief Perform a stimulation
  virtual void stimulate() = 0;

  /// @brief Get the number of channels in the stimulator
  /// @return The number of channels in the stimulator
  DECLARE_PROTECTED_MEMBER(int, StimulatorChannelCount)

public:
  /// @brief Set the callback function to call when a stimulation is performed
  /// @param onStimulate The callback function
  utils::StimwalkerEvent<const DataPoint &> OnStimulation;

protected:
  /// @brief Method that is internally called when a stimulation is performed.
  /// It is expected to be called by the device and then call the OnStimulation
  /// callback
  /// @param data The data of the stimulation
  virtual void HandleStimulation(const DataPoint &data) = 0;
};
} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_GENERIC_STIMULATOR_H__