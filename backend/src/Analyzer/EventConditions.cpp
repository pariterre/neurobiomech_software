#include "Analyzer/EventConditions.h"

using namespace NEUROBIO_NAMESPACE::analyzer;

std::vector<std::unique_ptr<Condition>>
parseConditions(const nlohmann::json &json) {
  std::vector<std::unique_ptr<Condition>> conditions;
  for (const auto &condition : json) {
    std::string conditionType = condition["type"];

    if (conditionType == "threshold")
      conditions.push_back(std::make_unique<ThresholdedCondition>(condition));
    else if (conditionType == "direction")
      conditions.push_back(std::make_unique<DirectionCondition>(condition));
    else
      throw std::invalid_argument("Invalid condition type: " + conditionType);
  }
  return conditions;
}

/// @brief Get the comparator from the json object
/// @param json The json object to get the comparator from
/// @return The comparator function
std::function<bool(double, double)>
getThresholdComparator(const std::string &comparator) {
  if (comparator == ">")
    return std::greater<double>();
  else if (comparator == ">=")
    return std::greater_equal<double>();
  else if (comparator == "<")
    return std::less<double>();
  else if (comparator == "<=")
    return std::less_equal<double>();
  else if (comparator == "==")
    return std::equal_to<double>();
  else if (comparator == "!=")
    return std::not_equal_to<double>();
  else
    throw std::invalid_argument("Invalid comparator: " + comparator);
}

ThresholdedCondition::ThresholdedCondition(
    const std::string &deviceName, size_t channelIndex,
    std::function<bool(double, double)> comparator, double threshold)
    : m_DeviceName(deviceName), m_ChannelIndex(channelIndex),
      m_Comparator(comparator), m_Threshold(threshold) {}

ThresholdedCondition::ThresholdedCondition(const nlohmann::json &json)
    : m_DeviceName(json.at("device")), m_ChannelIndex(json.at("channel")),
      m_Comparator(getThresholdComparator(json.at("comparator"))),
      m_Threshold(json.at("value")) {}

bool ThresholdedCondition::isActive(
    const std::map<std::string, data::TimeSeries> &data) const {
  auto channel = data.at(m_DeviceName).back().getData().at(m_ChannelIndex);
  return m_Comparator(channel, m_Threshold);
}

std::function<bool(double, double)>
getDirectionComparator(const std::string &direction) {
  if (direction == "positive")
    return getThresholdComparator("<=");
  else if (direction == "negative")
    return getThresholdComparator(">=");
  else
    throw std::invalid_argument("Invalid direction: " + direction);
}

DirectionCondition::DirectionCondition(const std::string &deviceName,
                                       size_t channelIndex,
                                       const std::string &direction)
    : ThresholdedCondition(deviceName, channelIndex,
                           getDirectionComparator(direction), 0.0) {}

DirectionCondition::DirectionCondition(const nlohmann::json &json)
    : ThresholdedCondition(json.at("device"), json.at("channel"),
                           getDirectionComparator(json.at("direction")), 0.0) {}

bool DirectionCondition::isActive(
    const std::map<std::string, data::TimeSeries> &data) const {
  auto device = data.at(m_DeviceName);
  size_t dataSize = device.size();
  if (dataSize < 2) {
    throw std::invalid_argument("Device data size is too small");
  }

  auto penultimateData = device[dataSize - 2].getData().at(m_ChannelIndex);
  auto lastData = device[dataSize - 1].getData().at(m_ChannelIndex);
  return m_Comparator(penultimateData, lastData);
}

EventConditions::EventConditions(const nlohmann::json &json)
    : m_Name(json.at("name")), m_PreviousName(json.at("previous")),
      m_Conditions(parseConditions(json.at("start_when"))) {}

bool EventConditions::isActive(
    size_t currentPhaseIndex,
    const std::map<std::string, data::TimeSeries> &data) const {
  if (m_PreviousIndex != currentPhaseIndex) {
    return false;
  }

  for (const auto &condition : m_Conditions) {
    if (!condition->isActive(data)) {
      return false;
    }
  }
  return true;
}
