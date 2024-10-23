#ifndef __STIMWALKER_DATA_DATA_POINT_H__
#define __STIMWALKER_DATA_DATA_POINT_H__

#include "stimwalkerConfig.h"

#include "Utils/CppMacros.h"
#include <map>
#include <nlohmann/json.hpp>

namespace STIMWALKER_NAMESPACE::data {

/// @brief Class to store data
class DataPoint {

public:
  /// @brief Empty constructor
  DataPoint() : m_TimeStamp(std::chrono::microseconds(0)), m_Data({}) {};

  /// @brief Contructor
  /// @param timeStamp The time stamp (i.e. elapsed time since starting time)
  /// @param data The data to store
  DataPoint(const std::chrono::microseconds &timeStamp,
            const std::vector<double> &data)
      : m_TimeStamp(timeStamp), m_Data(data) {}

  /// @brief Deserialize a json object
  /// @param json The json object to deserialize
  DataPoint(const nlohmann::json &json)
      : m_TimeStamp(json[0].get<int64_t>()),
        m_Data(json[1].get<std::vector<double>>()) {}

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
  /// @brief The time stamp of the data
  DECLARE_PROTECTED_MEMBER(std::chrono::microseconds, TimeStamp);

  /// @brief The data
  DECLARE_PROTECTED_MEMBER(std::vector<double>, Data);
};

} // namespace STIMWALKER_NAMESPACE::data

#endif // __STIMWALKER_DATA_DATA_POINT_H__