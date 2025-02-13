#include "Analyzer/TimedEventsLiveAnalyzer.h"
#include "Devices/Concrete/DelsysEmgDevice.h"
#include "Utils/Logger.h"
#include <chrono>
#include <thread>

using namespace NEUROBIO_NAMESPACE;

static int currentPhase = 0;
std::chrono::milliseconds lastPhaseChangeTimestamp;
bool shouldIncrementPhase(const std::map<size_t, data::TimeSeries> &data) {
  auto &dataDevice = data.begin()->second;
  auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        dataDevice.back().getTimeStamp()) -
                    lastPhaseChangeTimestamp;

  if (currentPhase == 0 && timePassed >= std::chrono::milliseconds(400)) {
    lastPhaseChangeTimestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            dataDevice.back().getTimeStamp());
    currentPhase = 1;
    return true;
  } else if (currentPhase == 1 &&
             timePassed >= std::chrono::milliseconds(600)) {
    lastPhaseChangeTimestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            dataDevice.back().getTimeStamp());
    currentPhase = 0;
    return true;
  }
  return false;
}

std::chrono::system_clock::time_point
getCurrentTime(const std::map<size_t, data::TimeSeries> &data) {
  // Get the first (and only) data device
  auto &dataDevice = data.begin()->second;

  // Get the current time stamp by adding passed time to starting time
  return dataDevice.getStartingTime() + dataDevice.back().getTimeStamp();
}

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

    auto eventAnalyzer = analyzer::TimedEventsLiveAnalyzer(
        {std::chrono::milliseconds(100), std::chrono::milliseconds(100)},
        shouldIncrementPhase, getCurrentTime, 0.5);

    std::cout << "Pre-trained model: "
              << eventAnalyzer.getTimeEventModel()[0].count() << ", "
              << eventAnalyzer.getTimeEventModel()[1].count() << std::endl;
    for (size_t i = 0; i < 100; i++) {
      auto prediction = std::unique_ptr<analyzer::EventPrediction>(
          dynamic_cast<analyzer::EventPrediction *>(
              eventAnalyzer.predict({{0, data.slice(i * 100, (i + 1) * 100)}})
                  .release()));

      std::cout << "From " << i * 100 << " to " << (i + 1) * 100 << ": "
                << prediction->getValues()[0];
      if (prediction->getHasPhaseIncremented())
        std::cout << " (phase incremented)";
      std::cout << std::endl;

      std::cout << prediction->serialize().dump() << std::endl;
    }
    std::cout << "Post-trained model: "
              << eventAnalyzer.getTimeEventModel()[0].count() << ", "
              << eventAnalyzer.getTimeEventModel()[1].count() << std::endl;

  } catch (std::exception &e) {
    logger.fatal(e.what());
    return EXIT_FAILURE;
  }

  logger.info("Exiting the application");
  return EXIT_SUCCESS;
}