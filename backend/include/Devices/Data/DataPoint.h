#ifndef __STIMWALKER_DEVICES_DATA_DATA_POINT_H__
#define __STIMWALKER_DEVICES_DATA_DATA_POINT_H__

#include <map>
#include <nlohmann/json.hpp>

#include "Utils/CppMacros.h"
#include "stimwalkerConfig.h"

namespace STIMWALKER_NAMESPACE::devices {

/// @brief Class to store data
class DataPoint {
public:
  DataPoint(time_t timestamp, const std::vector<double> &data);

  /// @brief Convert the object to JSON
  /// @return The JSON object
  nlohmann::json serialize() const;

  /// @brief Deserialize the object
  /// @param json The JSON object
  static DataPoint deserialize(const nlohmann::json &json);

  /// @brief Get a copy of the current object
  /// @return A copy of the current object
  DataPoint copy() const;

protected:
  /// @brief The data timestamp
  DECLARE_PROTECTED_MEMBER(time_t, Timestamp);

  /// @brief The data
  DECLARE_PROTECTED_MEMBER(std::vector<double>, Data);
};
} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_DATA_DATA_POINT_H__