#include "neurobio.h"

#include <asio.hpp>

using namespace NEUROBIO_NAMESPACE;

std::map<std::string, std::string> parseArgs(int argc, char *argv[]) {
  std::map<std::string, std::string> args;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    size_t pos = arg.find("=");
    if (pos != std::string::npos && arg.substr(0, 2) == "--") {
      std::string key = arg.substr(2, pos - 2);
      std::string value = arg.substr(pos + 1);
      args[key] = value;
    }
  }
  return args;
}

int main(int argc, char *argv[]) {
  auto &logger = utils::Logger::getInstance();
  logger.setLogFile("neurobio.log");
  logger.setLogLevel(utils::Logger::INFO);
  logger.info("------------------------------");

  int commandPort = 5000;
  int messagePort = 5001;
  int liveDataPort = 5002;
  int liveAnalysesPort = 5003;
  bool useMock = false;

  // If argv contains the ports (--portCommand=xxxx, --portMessage=xxxxx,
  // etc.), use them
  auto args = parseArgs(argc, argv);
  for (auto &arg : args) {
    if (arg.first == "portCommand") {
      commandPort = std::stoi(arg.second);
    } else if (arg.first == "portMessage") {
      messagePort = std::stoi(arg.second);
    } else if (arg.first == "portLiveData") {
      liveDataPort = std::stoi(arg.second);
    } else if (arg.first == "portLiveAnalyses") {
      liveAnalysesPort = std::stoi(arg.second);
    } else if (arg.first == "useMock") {
      useMock = (arg.second == "true");
    } else if (arg.first == "help") {
      logger.info("Usage: neurobio [--portCommand=xxxx] [--portMessage=xxxxx] "
                  "[--portLiveData=xxxxx] [--portLiveAnalyses=xxxxx] "
                  "[--useMock=<true|false>]");
      return EXIT_SUCCESS;
    }
  }

  try {
    if (useMock) {
      logger.warning("Starting the neurobio server using the MOCK server");
    } else {
      logger.info("Starting the neurobio server");
    }

    // Create a TCP server asynchroniously
    auto mainServer =
        useMock ? std::make_unique<server::TcpServerMock>(
                      commandPort, messagePort, liveDataPort, liveAnalysesPort)
                : std::make_unique<server::TcpServer>(
                      commandPort, messagePort, liveDataPort, liveAnalysesPort);
    mainServer->startServerSync();

  } catch (std::exception &e) {
    logger.fatal(e.what());
    return EXIT_FAILURE;
  }

  logger.info("Exiting the neurobio");
  return EXIT_SUCCESS;
}