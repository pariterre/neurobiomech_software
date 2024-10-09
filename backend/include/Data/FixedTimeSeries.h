#ifndef __STIMWALKER_DATA_FIXED_TIME_SERIES_H__
#define __STIMWALKER_DATA_FIXED_TIME_SERIES_H__

#include "stimwalkerConfig.h"

#include "Data/TimeSeries.h"

namespace STIMWALKER_NAMESPACE::data {

/// @brief Class to store data
class FixedTimeSeries : public TimeSeries {
public:
  /// @brief Constructor
  /// @param deltaTime The time frequency of the data
  FixedTimeSeries(const std::chrono::microseconds &deltaTime);
  FixedTimeSeries(const std::chrono::milliseconds &deltaTime);

  /// @brief Constructor
  /// @param startingTime The timestamp of the first data point
  /// @param deltaTime The time frequency of the data
  FixedTimeSeries(const std::chrono::system_clock::time_point &startingTime,
                  const std::chrono::microseconds &deltaTime);
  FixedTimeSeries(const std::chrono::system_clock::time_point &startingTime,
                  const std::chrono::milliseconds &deltaTime);
  FixedTimeSeries(const std::chrono::milliseconds &startingTime,
                  const std::chrono::microseconds &deltaTime);
  FixedTimeSeries(const std::chrono::milliseconds &startingTime,
                  const std::chrono::milliseconds &deltaTime);
  FixedTimeSeries(const std::chrono::microseconds &startingTime,
                  const std::chrono::microseconds &deltaTime);
  FixedTimeSeries(const std::chrono::microseconds &startingTime,
                  const std::chrono::milliseconds &deltaTime);
  ~FixedTimeSeries() = default;

  /// @brief This method should not be called as it would compute the timestamp
  /// repeatedly for no reason. If called, the first data point will be set to
  /// the current time
  /// @param data The data to add
  void add(const DataPoint &data) override;

  /// @brief Add new data to the collection. If this is the first data point,
  /// the timeStamp is set to the current time, the subsequent data points adds
  /// the DeltaTime to the previous timestamp
  /// @param data The data to add
  void add(const std::vector<double> &data) override;

protected:
  /// @brief The timestamp of the starting point. See [setStartingTime](@ref
  /// setStartingTime) for more information on how to change the starting time
  DECLARE_PROTECTED_MEMBER(std::chrono::system_clock::time_point, StartingTime);

  /// @brief The time frequency of the data
  DECLARE_PROTECTED_MEMBER(std::chrono::microseconds, DeltaTime);

public:
  /// @brief Set the starting time of the data. If no data are recorded yet,
  /// it will set the first data point to StartingTime. If data are already
  /// recorded, it will retroactively set the first data point to StartingTime
  /// and change the timestamp of the other data points accordingly
  void setStartingTime(const std::chrono::system_clock::time_point &time);
  void setStartingTime(const std::chrono::milliseconds &time);
  void setStartingTime(const std::chrono::microseconds &time);
};

} // namespace STIMWALKER_NAMESPACE::data

#endif // __STIMWALKER_DATA_FIXED_TIME_SERIES_H__