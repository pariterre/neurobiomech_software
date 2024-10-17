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
    auto worker = std::thread([&server]() { server.startServer(); });

    // Give a bit of time for the server to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Connect to this server using a TCP client
    server::TcpClient client;
    client.connect();

    // Add the devices
    client.addDelsysDevice();
    client.addMagstimDevice();

    // Start recording data
    client.startRecording();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    client.stopRecording();
    auto data = client.getData();

    // Remove the only data collector we have
    client.removeDelsysDevice();
    client.startRecording();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    client.stopRecording();
    // auto data = client.getData();

    // Clean up things
    client.disconnect();
    server.stopServer();
    worker.join();

  } catch (std::exception &e) {
    logger.fatal(e.what());
    return EXIT_FAILURE;
  }

  logger.info("Exiting the application");
  return EXIT_SUCCESS;
}