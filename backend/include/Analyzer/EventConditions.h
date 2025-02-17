#ifndef __NEUROBIO_ANALYZER_EVENT_CONDITIONS_H__
#define __NEUROBIO_ANALYZER_EVENT_CONDITIONS_H__

#include "neurobioConfig.h"

#include "Data/TimeSeries.h"
#include "Utils/CppMacros.h"
#include "nlohmann/json.hpp"
#include <functional>
#include <map>
#include <string>

namespace NEUROBIO_NAMESPACE::analyzer {

class Condition {
  friend class Conditions;

public:
  /// @brief If the condition is active
  /// @param data The data to check the condition against
  /// @return True if the condition is active, false otherwise
  virtual bool
  isActive(const std::map<std::string, data::TimeSeries> &data) const = 0;
};

class ThresholdedCondition : public Condition {
public:
  /// @brief Constructor of the ThresholdedCondition
  /// @param deviceName The name of the device to gather the data from
  /// @param channelIndex The channel index to gather the data from
  /// @param comparator The comparison function to change the phase
  /// @param threshold The threshold to detect the change of phase
  ThresholdedCondition(const std::string &deviceName, size_t channelIndex,
                       std::function<bool(double, double)> comparator,
                       double threshold);

  /// @brief Constructor of the ThresholdedCondition
  ThresholdedCondition(const nlohmann::json &json);

public:
  bool
  isActive(const std::map<std::string, data::TimeSeries> &data) const override;

protected:
  /// @brief The name of the device to gather the data from
  DECLARE_PROTECTED_MEMBER(std::string, DeviceName);

  /// @brief The channel to gather the data from
  DECLARE_PROTECTED_MEMBER(size_t, ChannelIndex);

  /// @brief The threshold to change the phase
  DECLARE_PROTECTED_MEMBER(double, Threshold);

  /// @brief The comparison function to change the phase
  DECLARE_PROTECTED_MEMBER(std::function<bool(double, double)>, Comparator);
};

class DirectionCondition : public ThresholdedCondition {
public:
  /// @brief Constructor of the DirectionCondition
  /// @param deviceName The name of the device to gather the data from
  /// @param channelIndex The channel index to gather the data from
  /// @param direction The direction ("positive" or "negative") needed to change
  /// the phase
  DirectionCondition(const std::string &deviceName, size_t channelIndex,
                     const std::string &direction);

  DirectionCondition(const nlohmann::json &json);

public:
  bool
  isActive(const std::map<std::string, data::TimeSeries> &data) const override;
};

class EventConditions {
public:
  /// @brief Constructor of the Conditions from a json object
  EventConditions(const nlohmann::json &json);

  /// @brief Destructor of the Conditions
  ~EventConditions() = default;

  /// @brief Collapse the all the indices
  static void collapseNameToIndices(
      std::vector<std::unique_ptr<EventConditions>> &conditions) {
    for (auto &condition : conditions) {
      // Collapse the previous phase index
      for (size_t i = 0; i < conditions.size(); i++) {
        if (condition->m_PreviousName == conditions[i]->m_Name) {
          condition->m_PreviousIndex = i;
          break;
        }
      }
    }
  }

  bool isActive(size_t currentPhaseIndex,
                const std::map<std::string, data::TimeSeries> &data) const;

protected:
  /// @brief The name of the condition
  DECLARE_PROTECTED_MEMBER(std::string, Name);

  /// @brief The previous phase name
  DECLARE_PROTECTED_MEMBER(std::string, PreviousName);

  /// @brief The previous phase index
  DECLARE_PROTECTED_MEMBER(size_t, PreviousIndex);

  /// @brief The conditions to change the phase
  DECLARE_PROTECTED_MEMBER(std::vector<std::unique_ptr<Condition>>, Conditions)
};

} // namespace NEUROBIO_NAMESPACE::analyzer

#endif // __NEUROBIO_ANALYZER_EVENT_CONDITIONS_H__