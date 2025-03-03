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
  logger.info("Starting the neurobio server");

  int commandPort = 5000;
  int responsePort = 5001;
  int liveDataPort = 5002;
  int liveAnalysesPort = 5003;

  // If argv contains the ports (--portCommand=xxxx, --portResponse=xxxxx,
  // etc.), use them
  auto args = parseArgs(argc, argv);
  for (auto &arg : args) {
    if (arg.first == "portCommand") {
      commandPort = std::stoi(arg.second);
    } else if (arg.first == "portResponse") {
      responsePort = std::stoi(arg.second);
    } else if (arg.first == "portLiveData") {
      liveDataPort = std::stoi(arg.second);
    } else if (arg.first == "portLiveAnalyses") {
      liveAnalysesPort = std::stoi(arg.second);
    }
  }

  try {
    // Create a TCP server asynchroniously
    server::TcpServer server(commandPort, responsePort, liveDataPort,
                             liveAnalysesPort);
    server.startServerSync();

  } catch (std::exception &e) {
    logger.fatal(e.what());
    return EXIT_FAILURE;
  }

  logger.info("Exiting the neurobio");
  return EXIT_SUCCESS;
}