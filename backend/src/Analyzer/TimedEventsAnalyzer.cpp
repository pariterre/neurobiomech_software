#include "Analyzer/TimedEventsAnalyzer.h"

#include "Analyzer/Exceptions.h"
#include "Data/DataPoint.h"
#include <numeric>

using namespace NEUROBIO_NAMESPACE::data;
using namespace NEUROBIO_NAMESPACE::analyzer;

TimedEventsAnalyzer::TimedEventsAnalyzer(
    const std::string &name,
    const std::vector<std::chrono::milliseconds> &initialTimeEventModel,
    const std::function<bool(const std::map<std::string, data::TimeSeries> &)>
        &shouldIncrementPhase,
    const std::function<std::chrono::system_clock::time_point(
        const std::map<std::string, data::TimeSeries> &)> &getCurrentTime,
    double learningRate)
    : m_ShouldIncrementPhase(shouldIncrementPhase),
      m_GetCurrentTime(getCurrentTime), m_CurrentPhaseIndex(0),
      m_InitialTimeEventModel(initialTimeEventModel),
      m_TimeEventModel(initialTimeEventModel),
      m_NextTimeEventModel(initialTimeEventModel), m_LearningRate(learningRate),
      m_FirstPass(true), m_CurrentPhaseTime(std::chrono::milliseconds(0)),
      Analyzer(name) {}

DataPoint TimedEventsAnalyzer::predict(
    const std::map<std::string, data::TimeSeries> &data) {
  // Analyze the data from the last analyzed time stamp to the most recent.

  auto currentTime = m_GetCurrentTime(data);
  // If it is the first pass, set the last analyzed time stamp to the current
  // time
  if (m_FirstPass) {
    m_LastAnalyzedTimeStamp = currentTime;
    m_FirstPass = false;
  }
  if (currentTime < m_LastAnalyzedTimeStamp) {
    throw TimeWentBackwardException(currentTime, m_LastAnalyzedTimeStamp);
  }

  // Update the current phase time
  m_CurrentPhaseTime += std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - m_LastAnalyzedTimeStamp);
  m_LastAnalyzedTimeStamp = currentTime;

  // Predict the percentage of the full events. If we went too far, stale the
  // prediction
  auto cappedValue =
      std::min(m_CurrentPhaseTime, m_TimeEventModel[m_CurrentPhaseIndex]);
  auto sumToCurrentPhase = std::accumulate(
      m_TimeEventModel.begin(), m_TimeEventModel.begin() + m_CurrentPhaseIndex,
      std::chrono::milliseconds(0));
  auto sumTotal =
      std::accumulate(m_TimeEventModel.begin(), m_TimeEventModel.end(),
                      std::chrono::milliseconds(0));
  auto predictedValue =
      static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(
                              cappedValue + sumToCurrentPhase)
                              .count()) /
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::milliseconds>(sumTotal)
              .count());

  // Check if we should increment the phase
  bool shouldIncrement = m_ShouldIncrementPhase(data);
  if (shouldIncrement)
    incrementModel();

  return DataPoint(std::chrono::duration_cast<std::chrono::microseconds>(
                       currentTime - m_ReferenceTime),
                   std::vector<double>{predictedValue},
                   {{"current_phase", m_CurrentPhaseIndex},
                    {"has_changed_phase", shouldIncrement}});
}

void TimedEventsAnalyzer::incrementModel() {
  // Increment the model to the next phase

  // Adjust the next model based on the prediction error it made
  auto predictionError =
      m_CurrentPhaseTime - m_TimeEventModel[m_CurrentPhaseIndex];
  auto correction = std::chrono::duration_cast<std::chrono::milliseconds>(
      predictionError * m_LearningRate);
  m_NextTimeEventModel[m_CurrentPhaseIndex] += correction;

  // Advance the phase
  m_CurrentPhaseTime = std::chrono::milliseconds(0);
  m_CurrentPhaseIndex = (m_CurrentPhaseIndex + 1) % m_TimeEventModel.size();

  // If we looped, transfer the next model to the current model
  if (m_CurrentPhaseIndex == 0) {
    m_TimeEventModel = m_NextTimeEventModel;
  }
}