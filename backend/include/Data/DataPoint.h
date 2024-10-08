#ifndef __STIMWALKER_DATA_DATA_POINT_H__
#define __STIMWALKER_DATA_DATA_POINT_H__

#include <map>
#include <nlohmann/json.hpp>

#include "Utils/CppMacros.h"
#include "stimwalkerConfig.h"

namespace STIMWALKER_NAMESPACE::data {

/// @brief Class to store data
class DataPoint {
public:
  /// @brief Constructor. Since timestamp is not provided, it is set to -1
  /// @param data The data to store
  DataPoint(const std::vector<double> &data);

  /// @brief Constructor
  /// @param timestamp The timestamp of the data
  /// @param data The data to store
  DataPoint(std::chrono::system_clock::time_point timestamp,
            const std::vector<double> &data);

  /// @brief Constructor from milliseconds
  /// @param timestamp The timestamp in milliseconds
  /// @param data The data to store
  DataPoint(std::chrono::milliseconds timestamp,
            const std::vector<double> &data);

  /// @brief Constructor from microseconds
  /// @param timestamp The timestamp in microseconds
  /// @param data The data to store
  DataPoint(std::chrono::microseconds timestamp,
            const std::vector<double> &data);

  /// @brief Get the number of channels
  /// @return The number of channels
  size_t size() const;

  /// @brief Get the specific data
  /// @param index The index of the data
  /// @return The data at the given index
  double operator[](size_t index) const;

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
  DECLARE_PROTECTED_MEMBER(std::chrono::system_clock::time_point, Timestamp);

  /// @brief The data
  DECLARE_PROTECTED_MEMBER(std::vector<double>, Data);
};

} // namespace STIMWALKER_NAMESPACE::data

#endif // __STIMWALKER_DATA_DATA_POINT_H__