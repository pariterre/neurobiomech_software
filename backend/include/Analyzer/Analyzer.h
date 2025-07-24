#ifndef __NEUROBIO_ANALYZER_ANALYZER_H__
#define __NEUROBIO_ANALYZER_ANALYZER_H__

#include "neurobioConfig.h"

#include "Utils/CppMacros.h"
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace NEUROBIO_NAMESPACE::data {
class DataPoint;
class TimeSeries;
} // namespace NEUROBIO_NAMESPACE::data

namespace NEUROBIO_NAMESPACE::analyzer {

class Analyzer {
public:
  /// @brief Constructor of the Analyzer
  Analyzer(const std::string &name) : m_Name(name) {};

public:
  /// @brief Destructor of the Analyzer
  virtual ~Analyzer() = default;

  /// @brief Predict some outcome from the sensor data
  /// @param data The data to analyze
  virtual data::DataPoint
  predict(const std::map<std::string, data::TimeSeries> &data) = 0;

protected:
  /// @brief The name of the analyzer
  DECLARE_PROTECTED_MEMBER(std::string, Name);

  /// @brief The reference time to base the DataPoint timeframe on
  DECLARE_PROTECTED_MEMBER(std::chrono::system_clock::time_point,
                           ReferenceTime);

public:
  /// @brief Set the reference time
  /// @param time The reference time
  void setReferenceTime(const std::chrono::system_clock::time_point &time) {
    m_ReferenceTime = time;
  }

  /// @brief Get the configuration of the analyzer in a json format
  /// @return The configuration of the analyzer in a json format
  virtual nlohmann::json getSerializedConfiguration() const = 0;
};

} // namespace NEUROBIO_NAMESPACE::analyzer
#endif // __NEUROBIO_ANALYZER_LIVE_ANALYZER_H__