#include "neurobio.h"

#include <asio.hpp>

using namespace NEUROBIO_NAMESPACE;

int main() {
  auto &logger = utils::Logger::getInstance();
  logger.setLogFile("neurobio.log");
  logger.setLogLevel(utils::Logger::INFO);
  logger.info("------------------------------");
  logger.info("Starting the neurobio server");

  try {
    // Create a TCP server asynchroniously
    server::TcpServer server;
    server.startServerSync();

  } catch (std::exception &e) {
    logger.fatal(e.what());
    return EXIT_FAILURE;
  }

  logger.info("Exiting the neurobio");
  return EXIT_SUCCESS;
}