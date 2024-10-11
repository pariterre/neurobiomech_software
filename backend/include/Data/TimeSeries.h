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
  TimeSeries() : m_StartingTime(std::chrono::system_clock::now()) {}
  TimeSeries(const std::chrono::system_clock::time_point &startingTime)
      : m_StartingTime(startingTime) {}
  virtual ~TimeSeries() = default;

  /// @brief Get the number of data in the collection
  /// @return The number of data in the collection
  size_t size() const;

  /// @brief Clear the data in the collection. This will not change the starting
  /// time
  void clear();

  /// @brief Add new data to the collection with the timestamp set to
  /// StartingTime + elapsed time since the first data point was added
  /// @param data The data to add. This also add a time stamp to the data equals
  /// to the elapsed since [m_StartingTime]
  virtual void add(const DataPoint &data);

  /// @brief Add new data to the collection with the timestamp set to
  /// StartingTime + elapsed time since the first data point was added
  /// @param timeStamp The time stamp to apply to the data
  /// @param data The data to add. This also add a time stamp to the data equals
  /// to the elapsed since [m_StartingTime]
  virtual void add(const std::chrono::microseconds &timeStamp,
                   const DataPoint &data);

  /// @brief Get the data at a specific index
  /// @param index The index of the data
  /// @return The data at the given index
  const std::pair<std::chrono::microseconds, DataPoint> &
  operator[](size_t index) const;

  /// @brief Get the data in serialized form
  /// @return The data in serialized form
  nlohmann::json serialize() const;

  /// @brief Deserialize the data
  /// @param json The data in serialized form
  static TimeSeries deserialize(const nlohmann::json &json);

protected:
  /// @brief The timestamp of the starting point. See [setStartingTime](@ref
  /// setStartingTime) for more information on how to change the starting time
  DECLARE_PROTECTED_MEMBER(std::chrono::system_clock::time_point, StartingTime);

public:
  /// @brief Clear the data in the collection and reset the starting time to the
  /// current time
  void reset();

public:
  /// @brief The data of the collection
  const std::vector<std::pair<std::chrono::microseconds, DataPoint>> &
  getData() const;

protected:
  std::vector<std::pair<std::chrono::microseconds, DataPoint>> m_Data;
};

} // namespace STIMWALKER_NAMESPACE::data

#endif // __STIMWALKER_DATA_TIME_SERIES_H__