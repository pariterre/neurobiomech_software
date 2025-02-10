#ifndef __NEUROBIO_DATA_FIXED_TIME_SERIES_H__
#define __NEUROBIO_DATA_FIXED_TIME_SERIES_H__

#include "neurobioConfig.h"

#include "Data/TimeSeries.h"

namespace NEUROBIO_NAMESPACE::data {

/// @brief Class to store data
class FixedTimeSeries : public TimeSeries {
public:
  /// @brief Constructor
  /// @param deltaTime The time frequency of the data
  FixedTimeSeries(const std::chrono::microseconds &deltaTime);

  /// @brief Constructor
  /// @param startingTime The timestamp of the first data point
  /// @param deltaTime The time frequency of the data
  FixedTimeSeries(const std::chrono::system_clock::time_point &startingTime,
                  const std::chrono::microseconds &deltaTime);

  ~FixedTimeSeries() = default;

  /// @brief Add new data to the collection with the timestamp
  /// @param timeStamp The time stamp of the data. This can break the "fixed"
  /// time series if the time stamp is not a multiple of the delta time or is
  /// not in the right order
  /// @param data The data to add
  virtual void add(const std::chrono::microseconds &timeStamp,
                   const std::vector<double> &data) override;

  /// @brief Add new data to the collection with the timestamp set to
  /// StartingTime + delta time * number of data points.
  /// @param data The data to add.
  void add(const std::vector<double> &data) override;

protected:
  /// @brief The time frequency of the data
  DECLARE_PROTECTED_MEMBER(std::chrono::microseconds, DeltaTime);
};

} // namespace NEUROBIO_NAMESPACE::data

#endif // __NEUROBIO_DATA_FIXED_TIME_SERIES_H__