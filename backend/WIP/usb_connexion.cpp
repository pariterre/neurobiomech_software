#include <Devices/MagstimRapidDevice.h>
#include <Utils/Logger.h>

#include <chrono>
#include <thread>

using namespace STIMWALKER_NAMESPACE::devices;

int main() {
  auto &logger = Logger::getInstance();
  logger.setLogLevel(Logger::INFO);

  logger.info("Starting the application");

  try {
    auto magstim = MagstimRapidDeviceMock::FindMagstimDevice();

    magstim.connect();
    logger.info("Opened port: " + magstim.getPort());

    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(1750));
    magstim.send(MagstimRapidCommands::PRINT, "Hello, world!", true);

    std::this_thread::sleep_for(std::chrono::seconds(4));
    auto response =
        magstim.send(MagstimRapidCommands::ARM, std::chrono::milliseconds(200));
    logger.info("Response: " + response.getValue());

    magstim.send(MagstimRapidCommands::PRINT, "Coucou!");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    magstim.send(MagstimRapidCommands::DISARM, std::chrono::milliseconds(1500));
    std::this_thread::sleep_for(std::chrono::seconds(8));
    magstim.send(MagstimRapidCommands::PRINT, "Too long..");

    magstim.disconnect();

    logger.info("Closed port: " + magstim.getPort());

  } catch (std::exception &e) {
    logger.error(e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}