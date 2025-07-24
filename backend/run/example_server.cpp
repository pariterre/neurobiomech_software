#include "neurobio.h"

#include <asio.hpp>

using namespace NEUROBIO_NAMESPACE;

auto analyzerExample = nlohmann::json::parse(R"({
        "name" : "Left Foot",
        "analyzer_type" : "cyclic_timed_events",
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
                "channel" : 0,
                "comparator" : "<=",
                "value" : -0.2
              }
            ]
          }
        ]
      })");

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
    std::uint32_t stateId = 0x10000001; // Random state ID
    if (!client.connect(stateId)) {
      logger.fatal("Failed to connect to the server");
      throw std::runtime_error("Failed to connect to the server");
    }

    // Add the devices
    bool areDevicesAdded = true;
    areDevicesAdded &= client.addDelsysAnalogDevice();
    areDevicesAdded &= client.addDelsysEmgDevice();
    areDevicesAdded &= client.addMagstimDevice();
    if (!areDevicesAdded) {
      logger.fatal("Failed to add the devices");
      throw std::runtime_error("Failed to add the devices");
    }

    // Add an analyzer to the server
    client.addAnalyzer(analyzerExample);

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

    // Remove the data analysez we have
    client.removeAnalyzer("Left Foot");

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