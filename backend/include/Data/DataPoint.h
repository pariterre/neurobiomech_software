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
  DataPoint(const std::vector<double> &data) : m_Data(data) {}

  /// @brief Deserialize a json object
  /// @param json The json object to deserialize
  DataPoint(const nlohmann::json &json)
      : m_Data(json.get<std::vector<double>>()) {}

  /// @brief Get the number of channels
  /// @return The number of channels
  size_t size() const;

  /// @brief Get the specific data
  /// @param index The index of the data
  /// @return The data at the given index
  const double &operator[](size_t index) const;

  /// @brief Convert the object to JSON
  /// @return The JSON object
  nlohmann::json serialize() const;

protected:
  /// @brief The data
  DECLARE_PROTECTED_MEMBER(std::vector<double>, Data);
};

} // namespace STIMWALKER_NAMESPACE::data

#endif // __STIMWALKER_DATA_DATA_POINT_H__