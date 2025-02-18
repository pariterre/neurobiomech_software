#include <gtest/gtest.h>

#include "Analyzer/Analyzers.h"
#include "Analyzer/CyclicTimedEventsAnalyzer.h"
#include "Analyzer/EventConditions.h"
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

void checkAnalyzerSize(size_t idxFirst, size_t idxSecond,
                       const std::vector<size_t> &ids) {
  ASSERT_EQ(ids.size(), 2);
  ASSERT_EQ(ids[0], idxFirst);
  ASSERT_EQ(ids[1], idxSecond);
}

analyzer::Analyzers generateAnalyzers() {

  auto analyzers = analyzer::Analyzers();
  // Add one analyzer for the left side
  size_t idxFirst = analyzers.add(nlohmann::json::parse(R"({
        "name" : "Left Foot",
        "analyzer_type" : "cyclic_from_analogs",
        "time_reference_device" : "DelsysAnalogDataCollector",
        "learning_rate" : 0.5,
        "initial_phase_durations" : [400, 600],
        "events" : [
          {
            "name" : "heel_strike",
            "previous" : "toe_off",
            "start_when" : [
              {
                "type": "threshold",
                "device" : "DelsysAnalogDataCollector",
                "channel" : 0,
                "comparator" : ">=",
                "value" : 0.2
              }
            ]
          },
          {
            "name" : "toe_off",
            "previous" : "heel_strike",
            "start_when" : [
              {
                "type": "threshold",
                "device" : "DelsysAnalogDataCollector",
                "channel" : 1,
                "comparator" : ">=",
                "value" : 0.2
              }
            ]
          }
        ]
      })"));
  // Add one analyzer for the right side
  size_t idxSecond = analyzers.add(nlohmann::json::parse(R"({
        "name" : "Right Foot",
        "analyzer_type" : "cyclic_from_analogs",
        "time_reference_device" : "DelsysAnalogDataCollector",
        "learning_rate" : 0.1,
        "initial_phase_durations" : [100, 150],
        "events" : [
          {
            "name" : "heel_strike",
            "previous" : "toe_off",
            "start_when" : [
              {
                "type": "direction",
                "device" : "DelsysAnalogDataCollector",
                "channel" : 0,
                "direction" : "positive"
              }
            ]
          },
          {
            "name" : "toe_off",
            "previous" : "heel_strike",
            "start_when" : [
              {
                "type": "direction",
                "device" : "DelsysAnalogDataCollector",
                "channel" : 1,
                "direction" : "negative"
              }
            ]
          }
        ]
      })"));

  // Check the ids
  checkAnalyzerSize(idxFirst, idxSecond, analyzers.getAnalyzerIds());

  return analyzers;
}

TEST(Analyzers, Constructors) {
  auto analyzers = generateAnalyzers();
  auto ids = analyzers.getAnalyzerIds();

  auto idLeftFoot = analyzers.getAnalyzerId("Left Foot");
  auto idRightFoot = analyzers.getAnalyzerId("Right Foot");
  EXPECT_THROW(analyzers.getAnalyzerId("Unknown"), std::invalid_argument);
  ASSERT_EQ(idLeftFoot, ids[0]);
  ASSERT_EQ(idRightFoot, ids[1]);

  auto mockData = generateData();

  // Check the size
  ASSERT_EQ(analyzers.size(), 2);

  // Check the json was correctly parsed
  {
    const auto &analyzerFirst =
        dynamic_cast<const analyzer::CyclicTimedEventsAnalyzer &>(
            analyzers[ids[0]]);
    ASSERT_EQ(analyzerFirst.getName(), "Left Foot");
    ASSERT_NEAR(analyzerFirst.getLearningRate(), 0.5, requiredPrecision);
    ASSERT_EQ(analyzerFirst.getTimeDeviceReferenceName(),
              "DelsysAnalogDataCollector");
    ASSERT_EQ(analyzerFirst.getTimeEventModel().size(), 2);
    ASSERT_EQ(analyzerFirst.getTimeEventModel()[0],
              std::chrono::milliseconds(400));
    ASSERT_EQ(analyzerFirst.getTimeEventModel()[1],
              std::chrono::milliseconds(600));
    ASSERT_EQ(analyzerFirst.getEventConditions().size(), 2);
    ASSERT_EQ(analyzerFirst.getEventConditions()[0]->getConditions().size(), 1);
    ASSERT_EQ(analyzerFirst.getEventConditions()[0]->getName(), "heel_strike");
    ASSERT_EQ(analyzerFirst.getEventConditions()[0]->getPreviousName(),
              "toe_off");
    ASSERT_EQ(analyzerFirst.getEventConditions()[0]->getPreviousIndex(), 1);

    auto conditionFirst = dynamic_cast<analyzer::ThresholdedCondition *>(
        const_cast<std::unique_ptr<analyzer::Condition> &>(
            analyzerFirst.getEventConditions()[0]->getConditions()[0])
            .release());
    ASSERT_EQ(conditionFirst->getDeviceName(), "DelsysAnalogDataCollector");
    ASSERT_EQ(conditionFirst->getChannelIndex(), 0);
    ASSERT_NEAR(conditionFirst->getThreshold(), 0.2, requiredPrecision);
    ASSERT_FALSE(conditionFirst->isActive(
        {{"DelsysAnalogDataCollector", mockData.slice(0, 1)}}));

    ASSERT_EQ(analyzerFirst.getEventConditions()[1]->getConditions().size(), 1);
    ASSERT_EQ(analyzerFirst.getEventConditions()[1]->getName(), "toe_off");
    ASSERT_EQ(analyzerFirst.getEventConditions()[1]->getPreviousName(),
              "heel_strike");
    ASSERT_EQ(analyzerFirst.getEventConditions()[1]->getPreviousIndex(), 0);

    auto conditionSecond = dynamic_cast<analyzer::ThresholdedCondition *>(
        const_cast<std::unique_ptr<analyzer::Condition> &>(
            analyzerFirst.getEventConditions()[1]->getConditions()[0])
            .release());

    ASSERT_EQ(conditionSecond->getDeviceName(), "DelsysAnalogDataCollector");
    ASSERT_EQ(conditionSecond->getChannelIndex(), 1);
    ASSERT_NEAR(conditionSecond->getThreshold(), 0.2, requiredPrecision);
    ASSERT_TRUE(conditionSecond->isActive(
        {{"DelsysAnalogDataCollector", mockData.slice(0, 1)}}));
  }

  {
    const auto &analyzerSecond =
        dynamic_cast<const analyzer::CyclicTimedEventsAnalyzer &>(
            analyzers[ids[1]]);
    ASSERT_EQ(analyzerSecond.getName(), "Right Foot");
    ASSERT_NEAR(analyzerSecond.getLearningRate(), 0.1, requiredPrecision);
    ASSERT_EQ(analyzerSecond.getTimeDeviceReferenceName(),
              "DelsysAnalogDataCollector");
    ASSERT_EQ(analyzerSecond.getTimeEventModel().size(), 2);
    ASSERT_EQ(analyzerSecond.getTimeEventModel()[0],
              std::chrono::milliseconds(100));
    ASSERT_EQ(analyzerSecond.getTimeEventModel()[1],
              std::chrono::milliseconds(150));
    ASSERT_EQ(analyzerSecond.getEventConditions().size(), 2);
    ASSERT_EQ(analyzerSecond.getEventConditions()[0]->getConditions().size(),
              1);
    ASSERT_EQ(analyzerSecond.getEventConditions()[0]->getName(), "heel_strike");
    ASSERT_EQ(analyzerSecond.getEventConditions()[0]->getPreviousName(),
              "toe_off");
    ASSERT_EQ(analyzerSecond.getEventConditions()[0]->getPreviousIndex(), 1);

    auto conditionFirst = dynamic_cast<analyzer::DirectionCondition *>(
        const_cast<std::unique_ptr<analyzer::Condition> &>(
            analyzerSecond.getEventConditions()[0]->getConditions()[0])
            .release());
    ASSERT_EQ(conditionFirst->getDeviceName(), "DelsysAnalogDataCollector");
    ASSERT_EQ(conditionFirst->getChannelIndex(), 0);
    ASSERT_NEAR(conditionFirst->getThreshold(), 0.0, requiredPrecision);
    EXPECT_THROW(conditionFirst->isActive(
                     {{"DelsysAnalogDataCollector", mockData.slice(0, 1)}}),
                 std::invalid_argument);
    ASSERT_TRUE(conditionFirst->isActive(
        {{"DelsysAnalogDataCollector", mockData.slice(0, 2)}}));

    ASSERT_EQ(analyzerSecond.getEventConditions()[1]->getConditions().size(),
              1);
    ASSERT_EQ(analyzerSecond.getEventConditions()[1]->getName(), "toe_off");
    ASSERT_EQ(analyzerSecond.getEventConditions()[1]->getPreviousName(),
              "heel_strike");
    ASSERT_EQ(analyzerSecond.getEventConditions()[1]->getPreviousIndex(), 0);

    auto conditionSecond = dynamic_cast<analyzer::DirectionCondition *>(
        const_cast<std::unique_ptr<analyzer::Condition> &>(
            analyzerSecond.getEventConditions()[1]->getConditions()[0])
            .release());

    ASSERT_EQ(conditionSecond->getDeviceName(), "DelsysAnalogDataCollector");
    ASSERT_EQ(conditionSecond->getChannelIndex(), 1);
    ASSERT_NEAR(conditionSecond->getThreshold(), 0.0, requiredPrecision);
    ASSERT_TRUE(conditionSecond->isActive(
        {{"DelsysAnalogDataCollector", mockData.slice(0, 2)}}));
  }

  // Remove the first analyzer
  analyzers.remove("Left Foot");
  ASSERT_EQ(analyzers.size(), 1);

  // Check the ids
  auto newIds = analyzers.getAnalyzerIds();
  ASSERT_EQ(newIds.size(), 1);
  ASSERT_EQ(newIds[0], ids[1]);

  // Check this is indeed the second analyzer that is left
  const auto &analyzerRemaining =
      dynamic_cast<const analyzer::CyclicTimedEventsAnalyzer &>(
          analyzers.getAnalyzer(newIds[0]));
  ASSERT_EQ(analyzerRemaining.getName(), "Right Foot Predictor");

  // Clear the analyzers
  analyzers.clear();
  ASSERT_EQ(analyzers.size(), 0);

  // Check the ids
  ASSERT_EQ(analyzers.getAnalyzerIds().size(), 0);
}

TEST(Analyzers, Prediction) {
  auto analyzers = generateAnalyzers();
  auto ids = analyzers.getAnalyzerIds();

  // Check that the starting conditions are correct
  const auto &analyzerFirst =
      dynamic_cast<const analyzer::CyclicTimedEventsAnalyzer &>(
          analyzers[ids[0]]);
  const auto &analyzerSecond =
      dynamic_cast<const analyzer::CyclicTimedEventsAnalyzer &>(
          analyzers[ids[1]]);
  ASSERT_TRUE(analyzerFirst.getFirstPass());
  ASSERT_EQ(analyzerFirst.getCurrentPhaseIndex(), 0);
  ASSERT_EQ(analyzerFirst.getCurrentPhaseTime(), std::chrono::milliseconds(0));
  ASSERT_TRUE(analyzerSecond.getFirstPass());
  ASSERT_EQ(analyzerSecond.getCurrentPhaseIndex(), 0);
  ASSERT_EQ(analyzerSecond.getCurrentPhaseTime(), std::chrono::milliseconds(0));

  // Check the initial models
  const auto &modelFirst =
      dynamic_cast<const analyzer::TimedEventsAnalyzer &>(analyzers[ids[0]]);
  const auto &modelSecond =
      dynamic_cast<const analyzer::TimedEventsAnalyzer &>(analyzers[ids[1]]);
  ASSERT_EQ(modelFirst.getTimeEventModel()[0], std::chrono::milliseconds(400));
  ASSERT_EQ(modelFirst.getTimeEventModel()[1], std::chrono::milliseconds(600));
  ASSERT_EQ(modelSecond.getTimeEventModel()[0], std::chrono::milliseconds(100));
  ASSERT_EQ(modelSecond.getTimeEventModel()[1], std::chrono::milliseconds(150));

  // Predict data from a known set
  auto data = generateData();
  std::vector<std::vector<double>> predictions;
  for (size_t i = 0; i < data.size(); i += 2) {
    auto prediction = analyzers.predict(
        {{"DelsysAnalogDataCollector", data.slice(i, i + 2)}});
    predictions.push_back({prediction["Left Foot"]->getValues()[0],
                           prediction["Right Foot"]->getValues()[0]});
  }

  // Check the predictions
  ASSERT_NEAR(predictions[0][0], 0.0, requiredPrecision);
  ASSERT_NEAR(predictions[0][1], 0.0, requiredPrecision);
  ASSERT_NEAR(predictions[1][0], 0.42, requiredPrecision);
  ASSERT_NEAR(predictions[1][1], 0.48, requiredPrecision);
  ASSERT_NEAR(predictions[500][0], 1.0, requiredPrecision);
  ASSERT_NEAR(predictions[500][1], 0.31654676258992803, requiredPrecision);
  ASSERT_NEAR(predictions[2499][0], 0.58904109589041098, requiredPrecision);
  ASSERT_NEAR(predictions[2499][1], 1.0, requiredPrecision);

  // Check the final model
  ASSERT_EQ(modelFirst.getTimeEventModel()[0], std::chrono::milliseconds(43));
  ASSERT_EQ(modelFirst.getTimeEventModel()[1], std::chrono::milliseconds(30));
  ASSERT_EQ(modelSecond.getTimeEventModel()[0], std::chrono::milliseconds(47));
  ASSERT_EQ(modelSecond.getTimeEventModel()[1], std::chrono::milliseconds(69));
}