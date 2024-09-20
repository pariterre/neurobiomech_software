#include <Devices/UsbDevice.h>

#include <chrono>
#include <thread>

using namespace STIMWALKER_NAMESPACE::devices;

int main() {
  try {
    std::string vid = "067B";
    std::string pid = "2303";
    auto device = UsbDevice::fromVidAndPid(vid, pid);

    device.connect();
    std::cout << "Opened port: " << device.getPort() << std::endl;

    // Simulate some work
    std::this_thread::sleep_for(std::chrono::seconds(2));
    device.send(UsbDevice::Commands::PRINT, "Hello, world!");
    std::this_thread::sleep_for(std::chrono::seconds(4));
    device.send(UsbDevice::Commands::CHANGE_POKE_INTERVAL,
                std::chrono::milliseconds(200));
    device.send(UsbDevice::Commands::PRINT, "Coucou!");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    device.send(UsbDevice::Commands::CHANGE_POKE_INTERVAL,
                std::chrono::milliseconds(1500));
    std::this_thread::sleep_for(std::chrono::seconds(8));
    device.send(UsbDevice::Commands::PRINT, "Too long..");

    device.disconnect();
    std::cout << "Closed port: " << device.getPort() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return 0;
}