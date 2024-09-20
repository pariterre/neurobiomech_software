#include <Devices/MagstimRapidDevice.h>

#include <chrono>
#include <thread>

using namespace STIMWALKER_NAMESPACE::devices;

int main() {
  try {
    auto magstim = MagstimRapidDeviceMock::FindMagstimDevice();

    magstim.connect();
    std::cout << "Opened port: " << magstim.getPort() << std::endl;

    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(1750));
    magstim.send(MagstimRapidCommands::PRINT, "Hello, world!", true);
    std::this_thread::sleep_for(std::chrono::seconds(4));
    auto response =
        magstim.send(MagstimRapidCommands::ARM, std::chrono::milliseconds(200));
    std::cout << "Response: " << response.getValue() << std::endl;
    magstim.send(MagstimRapidCommands::PRINT, "Coucou!");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    magstim.send(MagstimRapidCommands::DISARM, std::chrono::milliseconds(1500));
    std::this_thread::sleep_for(std::chrono::seconds(8));
    magstim.send(MagstimRapidCommands::PRINT, "Too long..");

    magstim.disconnect();
    std::cout << "Closed port: " << magstim.getPort() << std::endl;

  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}