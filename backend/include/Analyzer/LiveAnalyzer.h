#ifndef __NEUROBIO_ANALYZER_ANALYZER_H__
#define __NEUROBIO_ANALYZER_ANALYZER_H__

#include "neurobioConfig.h"

#include "Data/TimeSeries.h"

namespace NEUROBIO_NAMESPACE::analyzer {

class LiveAnalyzer {
public:
  /// @brief Constructor of the Analyzer
  LiveAnalyzer() = default;

public:
  /// @brief Destructor of the Analyzer
  virtual ~LiveAnalyzer() = default;

public:
  /// @brief Predict some outcome from the sensor data
  /// @param sensorData The data to analyze
  virtual void predict(const neurobio::data::TimeSeries &sensorData) = 0;
};

} // namespace NEUROBIO_NAMESPACE::analyzer
#endif // __NEUROBIO_ANALYZER_ANALYZER_H__