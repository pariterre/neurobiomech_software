#include "stimwalker.h"

#include <asio.hpp>

using namespace STIMWALKER_NAMESPACE;

int main() {
  auto &logger = utils::Logger::getInstance();
  logger.setLogFile("stimwalker.log");
  logger.setLogLevel(utils::Logger::INFO);
  logger.info("------------------------------");
  logger.info("Starting the stimwalker server");

  try {
    // Create a TCP server asynchroniously
    server::TcpServer server;
    server.startServerSync();

  } catch (std::exception &e) {
    logger.fatal(e.what());
    return EXIT_FAILURE;
  }

  logger.info("Exiting the stimwalker");
  return EXIT_SUCCESS;
}