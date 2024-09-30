#include <Devices/MagstimRapidDevice.h>
#include <Utils/Logger.h>

#include <chrono>
#include <thread>

using namespace STIMWALKER_NAMESPACE;

int main() {
  // auto &logger = utils::Logger::getInstance();
  // logger.setLogLevel(utils::Logger::INFO);

  // logger.info("Starting the application");

  // try {
  //   auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

  //   magstim.connect();
  //   logger.info("Opened port: " + magstim.getPort());

  //   // Give the system some time
  //   std::this_thread::sleep_for(std::chrono::milliseconds(100));

  //   // Get the temperature
  //   // magstim.send(MagstimRapidCommands::GET_TEMPERATURE, "");

  //   // Simulate some work
  //   std::this_thread::sleep_for(std::chrono::milliseconds(1750));
  //   logger.info("Sending Hello World");
  //   magstim.send(devices::MagstimRapidCommands::PRINT, "Hello, world!",
  //   true);

  //   std::this_thread::sleep_for(std::chrono::seconds(4));
  //   auto response = magstim.send(devices::MagstimRapidCommands::ARM,
  //                                std::chrono::milliseconds(200));
  //   logger.info("Response: " + response.toString());

  //   magstim.send(devices::MagstimRapidCommands::PRINT, "Coucou!");
  //   std::this_thread::sleep_for(std::chrono::seconds(1));
  //   magstim.send(devices::MagstimRapidCommands::DISARM,
  //                std::chrono::milliseconds(1500));
  //   std::this_thread::sleep_for(std::chrono::seconds(8));
  //   magstim.send(devices::MagstimRapidCommands::PRINT, "Too long..");

  //   magstim.disconnect();
  //   logger.info("Closed port: " + magstim.getPort());
  // } catch (std::exception &e) {
  //   logger.fatal(e.what());
  //   return EXIT_FAILURE;
  // }

  auto magstim = devices::MagstimRapidDeviceMock::FindMagstimDevice();

  // Is not connected when created
  magstim.getIsConnected();

  // Connect to the device, now shows as connected
  magstim.connect();
  magstim.getIsConnected();

  // Disconnecting, shows as not connected anymore
  magstim.disconnect();

  return EXIT_SUCCESS;
}