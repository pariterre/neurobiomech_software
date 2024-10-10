#include "stimwalker.h"

using namespace STIMWALKER_NAMESPACE;

int main() {
  auto &logger = utils::Logger::getInstance();
  logger.setLogLevel(utils::Logger::INFO);
  logger.info("Starting the application");

  try {
    auto devices = devices::Devices();
    devices.add(std::make_unique<devices::DelsysEmgDeviceMock>());

    auto &delsys = devices.getDevice("DelsysEmgDevice");
    auto &delsysData = devices.getDataCollector("DelsysEmgDevice");

    delsys.connect();
    delsysData.startRecording();
    logger.info("The system is now connected and is recording");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    logger.info("The system has been recording for 1 seconds");

    delsysData.stopRecording();
    delsys.disconnect();
    logger.info("The system has stopped recording and is now disconnected");

    const auto &data = delsysData.getTrialData();
    logger.info("The data has been collected: " + std::to_string(data.size()) +
                " data points");

  } catch (std::exception &e) {
    logger.fatal(e.what());
    return EXIT_FAILURE;
  }

  logger.info("Exiting the application");
  return EXIT_SUCCESS;
}