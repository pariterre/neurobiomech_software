#include <Devices/UsbDevice.h>
#include <chrono>
#include <thread>

int main()
{
    try
    {
        std::string vid = "067B";
        std::string pid = "2303";
        auto device = UsbDevice::fromVidAndPid(vid, pid);
        if (!device.connect())
        {
            std::cerr << "Failed to open port: " << device.getPort() << std::endl;
            throw std::runtime_error("Failed to open port");
        }

        std::cout << "Opened port: " << device.getPort() << std::endl;

        // Wait for 5 seconds
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << device.getData() << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}