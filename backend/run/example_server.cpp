#include "stimwalker.h"

#include <asio.hpp>

using namespace STIMWALKER_NAMESPACE;

int main() {
  auto &logger = utils::Logger::getInstance();
  logger.setLogLevel(utils::Logger::INFO);
  logger.info("Starting the application");

  try {
    // Create a TCP server asynchroniously
    asio::io_context context;
    server::TcpServerMock server(5000);
    auto worker = std::thread([&context, &server]() {
      server.startServer();
      context.run();
    });

    // Give a bit of time for the server to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Connect to this server using a TCP client
    server::TcpClient client;
    client.connect();
    client.addDelsysDevice();

    client.startDataStreaming();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Clean up things
    server.stopServer();
    context.stop();
    worker.join();

  } catch (std::exception &e) {
    logger.fatal(e.what());
    return EXIT_FAILURE;
  }

  logger.info("Exiting the application");
  return EXIT_SUCCESS;
}