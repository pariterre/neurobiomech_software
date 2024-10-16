#include "Devices/Concrete/DelsysEmgDevice.h"
#include "Utils/Logger.h"
#include <chrono>
#include <thread>

using namespace STIMWALKER_NAMESPACE;

int main() {
  auto &logger = utils::Logger::getInstance();
  logger.setLogLevel(utils::Logger::INFO);

  logger.info("Starting the application");

  try {
    auto delsys = devices::DelsysEmgDeviceMock();

    delsys.connect();
    delsys.startDataStreaming();
    logger.info("The system is now connected and is streaming data");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    logger.info("The system has been streaming data for 1 seconds");

    delsys.stopDataStreaming();
    delsys.disconnect();
    logger.info(
        "The system has stopped streaming data and is now disconnected");

    const auto &data = delsys.getTimeSeries();
    logger.info("The data has been collected: " + std::to_string(data.size()) +
                " data points");

  } catch (std::exception &e) {
    logger.fatal(e.what());
    return EXIT_FAILURE;
  }

  logger.info("Exiting the application");
  return EXIT_SUCCESS;
}