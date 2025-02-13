#include "Analyzer/TimedEventsLiveAnalyzer.h"
#include "Devices/Concrete/DelsysEmgDevice.h"
#include "Utils/Logger.h"
#include <chrono>
#include <thread>

using namespace NEUROBIO_NAMESPACE;

bool shouldIncrementPhase(
    const std::map<size_t, const data::TimeSeries &> &data) {
  return true;
}

std::chrono::system_clock::time_point
getCurrentTime(const std::map<size_t, const data::TimeSeries &> &data) {
  return std::chrono::system_clock::now();
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

    std::this_thread::sleep_for(std::chrono::seconds(1));
    logger.info("The system has been recorded data for 1 seconds");

    delsys.stopRecording();
    delsys.stopDataStreaming();
    delsys.disconnect();
    logger.info("The system has stopped recording data, streaming data and is "
                "now disconnected");

    const auto &data = delsys.getTrialData();
    logger.info("The data has been collected: " + std::to_string(data.size()) +
                " data points");

    auto analyser = analyzer::TimedEventsLiveAnalyzer(
        {std::chrono::milliseconds(100), std::chrono::milliseconds(100)},
        shouldIncrementPhase, getCurrentTime);
    std::cout << analyser.predict({{0, data}})[0] << std::endl;
    std::cout << analyser.predict({{0, data}})[0] << std::endl;
    std::cout << analyser.predict({{0, data}})[0] << std::endl;
    std::cout << analyser.predict({{0, data}})[0] << std::endl;
    std::cout << analyser.predict({{0, data}})[0] << std::endl;

  } catch (std::exception &e) {
    logger.fatal(e.what());
    return EXIT_FAILURE;
  }

  logger.info("Exiting the application");
  return EXIT_SUCCESS;
}