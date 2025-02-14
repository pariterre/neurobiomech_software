#include <gtest/gtest.h>

#include "Analyzer/Analyzers.h"
#include "Analyzer/WalkingCycleFromDelsysPressureAnalyzer.h"
#include "Data/FixedTimeSeries.h"

static double requiredPrecision(1e-8);

using namespace NEUROBIO_NAMESPACE;

data::TimeSeries generateData() {
  data::FixedTimeSeries data = data::FixedTimeSeries(
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::milliseconds(10)));
  for (size_t i = 0; i < 5000; ++i) {
    data.add({std::sin(i / 10.0), std::cos(i / 10.0)});
  }
  return data;
}

TEST(Analyzers, Constructors) {
  auto analyzers = analyzer::Analyzers();
  // Add a first analyzer
  size_t idxFirst = analyzers.add(nlohmann::json::parse(R"({
      "type": 0,
      "delsysDeviceIndex": 0,
      "channelIndex": 0,
      "heelStrikeThreshold": 0.5,
      "toeOffThreshold": 0.5,
      "learningRate": 0.5
    })"));

  // Add a second analyzer
  size_t idxSecond = analyzers.add(nlohmann::json::parse(R"({
      "type": 0,
      "delsysDeviceIndex": 0,
      "channelIndex": 1,
      "heelStrikeThreshold": -0.5,
      "toeOffThreshold": -0.5,
      "learningRate": 0.1
    })"));

  // Check the size
  ASSERT_EQ(analyzers.size(), 2);

  // Check the ids
  auto ids = analyzers.getAnalyzerIds();
  ASSERT_EQ(ids.size(), 2);
  ASSERT_EQ(ids[0], idxFirst);
  ASSERT_EQ(ids[1], idxSecond);

  // Check the json was correctly parsed
  const auto &analyzerFirst =
      dynamic_cast<const analyzer::WalkingCycleFromDelsysPressureAnalyzer &>(
          analyzers[idxFirst]);
  ASSERT_EQ(analyzerFirst.getDelsysDeviceIndex(), 0);
  ASSERT_EQ(analyzerFirst.getChannelIndex(), 0);
  ASSERT_NEAR(analyzerFirst.getHeelStrikeThreshold(), 0.5, requiredPrecision);
  ASSERT_NEAR(analyzerFirst.getToeOffThreshold(), 0.5, requiredPrecision);
  ASSERT_NEAR(analyzerFirst.getLearningRate(), 0.5, requiredPrecision);

  const auto &analyzerSecond =
      dynamic_cast<const analyzer::WalkingCycleFromDelsysPressureAnalyzer &>(
          analyzers[idxSecond]);
  ASSERT_EQ(analyzerSecond.getDelsysDeviceIndex(), 0);
  ASSERT_EQ(analyzerSecond.getChannelIndex(), 1);
  ASSERT_NEAR(analyzerSecond.getHeelStrikeThreshold(), -0.5, requiredPrecision);
  ASSERT_NEAR(analyzerSecond.getToeOffThreshold(), -0.5, requiredPrecision);
  ASSERT_NEAR(analyzerSecond.getLearningRate(), 0.1, requiredPrecision);

  // Remove the first analyzer
  analyzers.remove(0);
  ASSERT_EQ(analyzers.size(), 1);

  // Check the ids
  ids = analyzers.getAnalyzerIds();
  ASSERT_EQ(ids.size(), 1);
  ASSERT_EQ(ids[0], idxSecond);

  // Check this is indeed the second analyzer that is left
  const auto &analyzerLeft =
      dynamic_cast<const analyzer::WalkingCycleFromDelsysPressureAnalyzer &>(
          analyzers.getAnalyzer(idxSecond));
  ASSERT_EQ(analyzerLeft.getDelsysDeviceIndex(), 0);
  ASSERT_EQ(analyzerLeft.getChannelIndex(), 1);
  ASSERT_NEAR(analyzerLeft.getHeelStrikeThreshold(), -0.5, requiredPrecision);
  ASSERT_NEAR(analyzerLeft.getToeOffThreshold(), -0.5, requiredPrecision);
  ASSERT_NEAR(analyzerLeft.getLearningRate(), 0.1, requiredPrecision);

  // Clear the analyzers
  analyzers.clear();
  ASSERT_EQ(analyzers.size(), 0);

  // Check the ids
  ids = analyzers.getAnalyzerIds();
  ASSERT_EQ(ids.size(), 0);
}

TEST(Analyzers, Prediction) {
  auto analyzers = analyzer::Analyzers();

  // Add a first analyzer
  size_t idxFirst = analyzers.add(nlohmann::json::parse(R"({
      "type": 0,
      "delsysDeviceIndex": 0,
      "channelIndex": 0,
      "heelStrikeThreshold": 0.5,
      "toeOffThreshold": 0.5,
      "learningRate": 0.5
    })"));

  // Add a second analyzer
  size_t idxSecond = analyzers.add(nlohmann::json::parse(R"({
      "type": 0,
      "delsysDeviceIndex": 0,
      "channelIndex": 1,
      "heelStrikeThreshold": -0.5,
      "toeOffThreshold": -0.5,
      "learningRate": 0.1
    })"));

  // Check that the starting conditions are correct
  const auto &analyzerFirst =
      dynamic_cast<const analyzer::WalkingCycleFromDelsysPressureAnalyzer &>(
          analyzers[idxFirst]);
  const auto &analyzerSecond =
      dynamic_cast<const analyzer::WalkingCycleFromDelsysPressureAnalyzer &>(
          analyzers[idxSecond]);
  ASSERT_TRUE(analyzerFirst.getFirstPass());
  ASSERT_EQ(analyzerFirst.getCurrentPhaseIndex(), 0);
  ASSERT_EQ(analyzerFirst.getCurrentPhaseTime(), std::chrono::milliseconds(0));
  ASSERT_TRUE(analyzerSecond.getFirstPass());
  ASSERT_EQ(analyzerSecond.getCurrentPhaseIndex(), 0);
  ASSERT_EQ(analyzerSecond.getCurrentPhaseTime(), std::chrono::milliseconds(0));

  // Check the initial models
  const auto &modelFirst =
      dynamic_cast<const analyzer::TimedEventsLiveAnalyzer &>(
          analyzers[idxFirst]);
  const auto &modelSecond =
      dynamic_cast<const analyzer::TimedEventsLiveAnalyzer &>(
          analyzers[idxSecond]);
  ASSERT_EQ(modelFirst.getTimeEventModel()[0], std::chrono::milliseconds(400));
  ASSERT_EQ(modelFirst.getTimeEventModel()[1], std::chrono::milliseconds(600));
  ASSERT_EQ(modelSecond.getTimeEventModel()[0], std::chrono::milliseconds(400));
  ASSERT_EQ(modelSecond.getTimeEventModel()[1], std::chrono::milliseconds(600));

  // Predict data from a known set
  auto data = generateData();
  std::vector<std::vector<double>> predictions;
  for (size_t i = 0; i < data.size(); ++i) {
    auto prediction = analyzers.predict({{0, data.slice(i, i + 1)}});
    predictions.push_back({prediction[idxFirst]->getValues()[0],
                           prediction[idxSecond]->getValues()[0]});
  }

  // Check the predictions
  ASSERT_NEAR(predictions[0][0], 0.0, requiredPrecision);
  ASSERT_NEAR(predictions[0][1], 0.0, requiredPrecision);
  ASSERT_NEAR(predictions[1][0], 0.01, requiredPrecision);
  ASSERT_NEAR(predictions[1][1], 0.41, requiredPrecision);
  ASSERT_NEAR(predictions[1000][0], 0.49363057324840764, requiredPrecision);
  ASSERT_NEAR(predictions[1000][1], 0.56609195402298851, requiredPrecision);
  ASSERT_NEAR(predictions[4999][0], 0.14423076923076922, requiredPrecision);
  ASSERT_NEAR(predictions[4999][1], 0.2190923317683881, requiredPrecision);

  // Check the final model
  ASSERT_EQ(modelFirst.getTimeEventModel()[0], std::chrono::milliseconds(415));
  ASSERT_EQ(modelFirst.getTimeEventModel()[1], std::chrono::milliseconds(209));
  ASSERT_EQ(modelSecond.getTimeEventModel()[0], std::chrono::milliseconds(213));
  ASSERT_EQ(modelSecond.getTimeEventModel()[1], std::chrono::milliseconds(426));
}