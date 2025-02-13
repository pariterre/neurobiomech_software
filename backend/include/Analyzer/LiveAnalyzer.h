#ifndef __NEUROBIO_ANALYZER_ANALYZER_H__
#define __NEUROBIO_ANALYZER_ANALYZER_H__

#include "neurobioConfig.h"
#include <map>
#include <memory>
#include <vector>

namespace NEUROBIO_NAMESPACE::data {
class TimeSeries;
} // namespace NEUROBIO_NAMESPACE::data

namespace NEUROBIO_NAMESPACE::analyzer {
class Prediction;

class LiveAnalyzer {
public:
  /// @brief Constructor of the Analyzer
  LiveAnalyzer() = default;

public:
  /// @brief Destructor of the Analyzer
  virtual ~LiveAnalyzer() = default;

public:
  /// @brief Predict some outcome from the sensor data
  /// @param data The data to analyze
  virtual std::unique_ptr<Prediction>
  predict(const std::map<size_t, const data::TimeSeries &> &data) = 0;
};

} // namespace NEUROBIO_NAMESPACE::analyzer
#endif // __NEUROBIO_ANALYZER_ANALYZER_H__