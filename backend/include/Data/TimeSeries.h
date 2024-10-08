#ifndef __STIMWALKER_DATA_TIME_SERIES_H__
#define __STIMWALKER_DATA_TIME_SERIES_H__

#include "stimwalkerConfig.h"
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>

#include "Data/DataPoint.h"
#include "Utils/CppMacros.h"

namespace STIMWALKER_NAMESPACE::data {

/// @brief Class to store data
class TimeSeries {
public:
  TimeSeries() = default;
  virtual ~TimeSeries() = default;

  /// @brief Get the number of data in the collection
  /// @return The number of data in the collection
  size_t size() const;

  /// @brief Clear the data in the collection
  void clear();

  /// @brief Add new data to the collection.
  /// @param data The data to add
  virtual void add(const DataPoint &data);

  /// @brief Add new data to the collection.
  /// @param data The data to add
  virtual void add(const std::vector<double> &data);

  /// @brief Get the data at a specific index
  /// @param index The index of the data
  /// @return The data at the given index
  const DataPoint &operator[](size_t index) const;

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

} // namespace STIMWALKER_NAMESPACE::data

#endif // __STIMWALKER_DATA_TIME_SERIES_H__