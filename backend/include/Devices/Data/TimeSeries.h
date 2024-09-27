#ifndef __STIMWALKER_DEVICES_DATA_TIME_SERIES_H__
#define __STIMWALKER_DEVICES_DATA_TIME_SERIES_H__

#include "stimwalkerConfig.h"
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>

#include "Utils/CppMacros.h"

namespace STIMWALKER_NAMESPACE::devices {
class DataPoint;

/// @brief Class to store data
class TimeSeries {
public:
  /// @brief Get the number of data in the collection
  /// @return The number of data in the collection
  int dataCount() const;

  /// @brief Add new data to the collection
  /// @param data The data to add
  void add(const DataPoint &data);

  /// @brief Get the data in serialized form
  /// @return The data in serialized form
  nlohmann::json serialize() const;

  /// @brief Deserialize the data
  /// @param json The data in serialized form
  static TimeSeries deserialize(const nlohmann::json &json);

protected:
  /// @brief The data of the collection
  DECLARE_PROTECTED_MEMBER(std::vector<DataPoint>, Data);
};
} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_DATA_TIME_SERIES_H__