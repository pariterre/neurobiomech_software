#include "Analyzer/WalkingLiveAnalyzer.h"

using namespace NEUROBIO_NAMESPACE::analyzer;

WalkingLiveAnalyzer::WalkingLiveAnalyzer(
    const std::vector<double> &timeEventModel,
    const std::vector<double> &expectedSensoryInputs, double learningRate,
    double sensorSensitivity)
    : m_TimeEventModel(timeEventModel),
      m_ExpectedSensoryInputs(expectedSensoryInputs),
      m_LearningRate(learningRate), m_SensorSensitivity(sensorSensitivity),
      m_LastAnalyzedTimeStamp(std::chrono::system_clock::now()) {
  // Make sure the time event model and the expected sensory inputs have the
  // same size
  if (m_TimeEventModel.size() != m_ExpectedSensoryInputs.size()) {
    throw std::invalid_argument("The time event model and the expected sensory "
                                "inputs must have the same size");
  }
}

WalkingLiveAnalyzer::~WalkingLiveAnalyzer() {}

void WalkingLiveAnalyzer::predict(
    const neurobio::data::TimeSeries &sensorData) {
  // Analyze the data from the last analyzed time stamp to the most recent.
}