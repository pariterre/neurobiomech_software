#ifndef __NEUROBIO_ANALYZER_WALKING_ANALYZER_H__
#define __NEUROBIO_ANALYZER_WALKING_ANALYZER_H__

#include "Analyzer/LiveAnalyzer.h"
#include "Utils/CppMacros.h"

namespace NEUROBIO_NAMESPACE::analyzer {

class WalkingLiveAnalyzer : public LiveAnalyzer {

public:
  /// @brief Constructor of the WalkingLiveAnalyzer
  WalkingLiveAnalyzer(const std::vector<double> &timeEventModel,
                      const std::vector<double> &expectedSensoryInputs,
                      double learningRate = 0.2,
                      double sensorSensitivity = 0.5);

public:
  /// @brief Destructor of the WalkingLiveAnalyzer
  ~WalkingLiveAnalyzer() override;

public:
  void predict(const neurobio::data::TimeSeries &sensorData) override;

protected:
  /// @brief The time event model (one value representing the expected time
  /// events in a step)
  DECLARE_PROTECTED_MEMBER(std::vector<double>, TimeEventModel);

  /// @brief The sensory inputs of each event. The vector must be of the same
  /// size as the TimeEventModel
  DECLARE_PROTECTED_MEMBER(std::vector<double>, ExpectedSensoryInputs);

  /// @brief The learning rate of the walking analyzer
  DECLARE_PROTECTED_MEMBER(double, LearningRate);

  /// @brief The sensitivity to the sensor data
  DECLARE_PROTECTED_MEMBER(double, SensorSensitivity);

  /// @brief The time stamp of the last analyzed data
  DECLARE_PROTECTED_MEMBER(std::chrono::system_clock::time_point,
                           LastAnalyzedTimeStamp);
};

} // namespace NEUROBIO_NAMESPACE::analyzer

#endif // __NEUROBIO_ANALYZER_WALKING_ANALYZER_H__