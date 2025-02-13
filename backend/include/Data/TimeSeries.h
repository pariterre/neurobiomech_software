#ifndef __NEUROBIO_DATA_TIME_SERIES_H__
#define __NEUROBIO_DATA_TIME_SERIES_H__

#include "neurobioConfig.h"

#include "Data/DataPoint.h"
#include "Utils/CppMacros.h"
#include "Utils/RollingVector.h"
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>

namespace NEUROBIO_NAMESPACE::data {

/// @brief Class to store data
class TimeSeries {

public:
  TimeSeries()
      : m_StartingTime(std::chrono::system_clock::now()),
        m_StopWatch(std::chrono::high_resolution_clock::now()) {}
  TimeSeries(const std::chrono::system_clock::time_point &startingTime)
      : m_StartingTime(startingTime),
        m_StopWatch(std::chrono::high_resolution_clock::now()) {}

  /// @brief Deserialize the data
  /// @param json The data in serialized form
  TimeSeries(const nlohmann::json &json);

  virtual ~TimeSeries() = default;

  /// @brief Get the number of data in the collection
  /// @return The number of data in the collection
  size_t size() const;

  /// @brief Set the maximum size of the rolling vector
  /// @param maxSize The maximum size of the rolling vector
  void setRollingVectorMaxSize(size_t maxSize);

  /// @brief Clear the data in the collection. This will not change the starting
  /// time
  void clear();

  /// @brief Add new data to the collection with the timestamp set to
  /// StartingTime + elapsed time since the first data point was added
  /// @param timeStamp The time stamp of the data
  /// @param data The data to add
  virtual void add(const std::chrono::microseconds &timeStamp,
                   const std::vector<double> &data);

  /// @brief Add new data to the collection with the timestamp set to
  /// StartingTime + elapsed time since the first data point was added
  /// @param data The data to add. This also add a time stamp to the data equals
  /// to the elapsed since [m_StartingTime]
  virtual void add(const std::vector<double> &data);

  /// @brief Get the data at a specific index
  /// @param index The index of the data
  /// @return The data at the given index
  const DataPoint &operator[](size_t index) const;

  /// @brief Get the slice of data from [from] to [to]
  /// @param from The starting index
  /// @param to The ending index
  TimeSeries slice(size_t from, size_t to) const;

  /// @brief Get the last n data
  /// @param n The number of data to get from the end
  TimeSeries tail(size_t n) const;

  /// @brief Get the first data
  /// @return The first data
  const DataPoint &front() const;

  /// @brief Get the last data
  /// @return The last data
  const DataPoint &back() const;

  /// @brief Get the data since a specific time
  /// @param time The time to get the data since
  TimeSeries since(const std::chrono::system_clock::time_point &time) const;

  /// @brief Get the data in serialized form
  /// @return The data in serialized form
  nlohmann::json serialize() const;

protected:
  /// @brief The value of the mean of the data when set to zero. This
  /// automatically subtract this value from the data when added
  DECLARE_PROTECTED_MEMBER(std::vector<double>, ZeroLevel);

public:
  /// @brief Set how long from now we should mean the data to compute the zero
  /// level
  void setZeroLevel(const std::chrono::milliseconds &duration);

protected:
  /// @brief Transform a data set to a zero levelled data set
  /// @param data The data to transform
  /// @return The zero levelled data
  std::vector<double> zeroLevelData(const std::vector<double> &data) const;

protected:
  /// @brief The timestamp of the starting point. See [setStartingTime](@ref
  /// setStartingTime) for more information on how to change the starting time
  DECLARE_PROTECTED_MEMBER(std::chrono::system_clock::time_point, StartingTime);

  /// @brief This is set when [TimeSeries] is create or [reset] method is
  /// called. Contrary to [m_StartingTime] it cannot be used as the initial
  /// point for the data. However, this allows to compute the elapsed time since
  /// reset. This is therefore used to create the timestamp of the data
  DECLARE_PROTECTED_MEMBER(std::chrono::high_resolution_clock::time_point,
                           StopWatch);

public:
  /// @brief Clear the data in the collection and reset the starting time to the
  /// current time
  void reset();

protected:
  /// @brief The data of the collection
  DECLARE_PROTECTED_MEMBER(utils::RollingVector<DataPoint>, Data);
};

} // namespace NEUROBIO_NAMESPACE::data

#endif // __NEUROBIO_DATA_TIME_SERIES_H__