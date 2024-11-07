#include "stimwalker.h"

#include <asio.hpp>

using namespace STIMWALKER_NAMESPACE;

int main() {
  auto &logger = utils::Logger::getInstance();
  logger.setLogLevel(utils::Logger::INFO);
  logger.info("Starting the application");

  try {
    // Create a TCP server asynchroniously
    server::TcpServerMock server;
    server.startServer();

    // Connect to this server using a TCP client
    server::TcpClient client;
    if (!client.connect()) {
      logger.fatal("Failed to connect to the server");
      throw std::runtime_error("Failed to connect to the server");
    }

    // Add the devices
    bool areDevicesAdded = true;
    areDevicesAdded &= client.addDelsysAnalogDevice();
    areDevicesAdded &= client.addDelsysEmgDevice();
    // areDevicesAdded &= client.addMagstimDevice();
    if (!areDevicesAdded) {
      logger.fatal("Failed to add the devices");
      throw std::runtime_error("Failed to add the devices");
    }

    // Give some time to the server to connect to the devices
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Start recording data
    client.startRecording();
    auto recordingTime = std::chrono::seconds(2);
    std::this_thread::sleep_for(recordingTime);
    client.stopRecording();
    auto data = client.getLastTrialData();
    logger.info("A second trial received containing: " +
                std::to_string(data["DelsysEmgDataCollector"].size()) +
                " EMG data series (expected about ~" +
                std::to_string(recordingTime.count() * 2000) + "), and " +
                std::to_string(data["DelsysAnalogDataCollector"].size()) +
                " Analog data series (expected about ~" +
                std::to_string(recordingTime.count() * 148) + ")");

    // Remove the only data collector we have
    client.removeMagstimDevice();
    client.startRecording();
    recordingTime = std::chrono::seconds(2);
    std::this_thread::sleep_for(recordingTime);
    client.stopRecording();
    data = client.getLastTrialData();
    logger.info("A second trial received containing: " +
                std::to_string(data["DelsysEmgDataCollector"].size()) +
                " EMG data series (expected about ~" +
                std::to_string(recordingTime.count() * 2000) + "), and " +
                std::to_string(data["DelsysAnalogDataCollector"].size()) +
                " Analog data series (expected about ~" +
                std::to_string(recordingTime.count() * 148) + ")");

    // std::cout << "A = [";
    // for (auto &dataPoints : data["DelsysAnalogDataCollector"].getData()) {
    //   std::cout << dataPoints[9] << ",";
    // }
    // std::cout << "]" << std::endl;

    // std::cout << "B = [";
    // for (auto &dataPoints : data["DelsysAnalogDataCollector"].getData()) {
    //   std::cout << dataPoints[10] << ",";
    // }
    // std::cout << "]" << std::endl;

    // std::cout << "C = [";
    // for (auto &dataPoints : data["DelsysAnalogDataCollector"].getData()) {
    //   std::cout << dataPoints[11] << ",";
    // }
    // std::cout << "]" << std::endl;

    std::cout << "D = [";
    for (auto &dataPoints : data["DelsysEmgDataCollector"].getData()) {
      std::cout << dataPoints[1] << ",";
    }
    std::cout << "]" << std::endl;

    // Clean up things
    client.disconnect();
    server.stopServer();

  } catch (std::exception &e) {
    logger.fatal(e.what());
    return EXIT_FAILURE;
  }

  logger.info("Exiting the application");
  return EXIT_SUCCESS;
}