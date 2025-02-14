#include "Analyzer/all.h"
#include "Devices/Concrete/DelsysEmgDevice.h"
#include "Utils/Logger.h"
#include <chrono>
#include <thread>

using namespace NEUROBIO_NAMESPACE;

int main() {
  auto &logger = utils::Logger::getInstance();
  logger.setLogLevel(utils::Logger::INFO);

  logger.info("Starting the application");

  try {
    auto delsys = devices::DelsysEmgDeviceMock();

    delsys.connect();
    delsys.startDataStreaming();
    delsys.startRecording();
    logger.info("The system is now connected, streaming and recording data");

    std::this_thread::sleep_for(std::chrono::seconds(10));
    logger.info("The system has been recorded data for 1 seconds");

    delsys.stopRecording();
    delsys.stopDataStreaming();
    delsys.disconnect();
    logger.info("The system has stopped recording data, streaming data and is "
                "now disconnected");

    const auto &data = delsys.getTrialData();
    logger.info("The data has been collected: " + std::to_string(data.size()) +
                " data points");

    auto analyzers = analyzer::Analyzers();
    // Add one analyzer for the left side
    // TODO: Allows for comparison symbol (e.g. "heelStrikeCondition": ">= 0.2")
    analyzers.add(nlohmann::json::parse(R"({
      "type": 0,
      "delsysDeviceIndex": 0,
      "channelIndex": 0,
      "heelStrikeThreshold": 0.2,
      "toeOffThreshold": -0.2,
      "learningRate": 0.5
    })"));
    // Add one analyzer for the right side
    analyzers.add(nlohmann::json::parse(R"({
      "type": 0,
      "delsysDeviceIndex": 0,
      "channelIndex": 1,
      "heelStrikeThreshold": -0.5,
      "toeOffThreshold": -0.5,
      "learningRate": 0.5
    })"));

    // Simulate the live predictions
    int packetSize = 10;
    for (size_t i = 0; i < data.size() / packetSize; ++i) {
      auto predictions = analyzers.predict(
          {{0, data.slice(i * packetSize, (i + 1) * packetSize)}});

      // Since we know that it is, downcast the prediction to event prediction
      auto predictionLeft = std::unique_ptr<analyzer::EventPrediction>(
          dynamic_cast<analyzer::EventPrediction *>(predictions[0].release()));
      auto predictionRight = std::unique_ptr<analyzer::EventPrediction>(
          dynamic_cast<analyzer::EventPrediction *>(predictions[1].release()));

      std::cout << "For " << std::setw(6) << std::setfill(' ')
                << (i + 1) * packetSize << ": L-" << std::fixed
                << std::setprecision(3) << predictionLeft->getValues()[0]
                << ", R-" << predictionRight->getValues()[0]
                << std::setprecision(std::cout.precision());

      if (predictionLeft->getHasPhaseIncremented())
        std::cout << " (Left phase incremented)";
      if (predictionRight->getHasPhaseIncremented())
        std::cout << " (Right phase incremented)";
      std::cout << std::endl;
    }

    // Print the final model
    const auto &model =
        dynamic_cast<const analyzer::TimedEventsLiveAnalyzer &>(analyzers[0]);
    std::cout << "Post-trained model: " << model.getTimeEventModel()[0].count()
              << ", " << model.getTimeEventModel()[1].count() << std::endl;

  } catch (std::exception &e) {
    logger.fatal(e.what());
    return EXIT_FAILURE;
  }

  logger.info("Exiting the application");
  return EXIT_SUCCESS;
}