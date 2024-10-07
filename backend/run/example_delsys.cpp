#include <Devices/DelsysEmgDevice.h>
#include <Utils/Logger.h>

#include <chrono>
#include <thread>

using namespace STIMWALKER_NAMESPACE;

int main() {
  auto &logger = utils::Logger::getInstance();
  logger.setLogLevel(utils::Logger::INFO);

  logger.info("Starting the application");

  try {
    // TODO Implement the tests
    auto delsys = devices::DelsysEmgDeviceMock();

    delsys.connect();
    delsys.startRecording();
    logger.info("The system is now connected and is recording");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    logger.info("The system has been recording for 1 seconds");

    delsys.stopRecording();
    delsys.disconnect();
    logger.info("The system has stopped recording and is now disconnected");

    const auto &data = delsys.getTrialData();
    logger.info("The data has been collected: " + std::to_string(data.size()) +
                " data points");

  } catch (std::exception &e) {
    logger.fatal(e.what());
    return EXIT_FAILURE;
  }

  logger.info("Exiting the application");
  return EXIT_SUCCESS;
}