#include <Devices/UsbDevice.h>

int main()
{
    try
    {
        std::string vid = "067B";
        std::string pid = "2303";
        auto device = UsbDevice::fromVidAndPid(vid, pid);
        device.connect();

        std::cout << "Opened port: " << device.getPort() << std::endl;
        // Communicate with the device here
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}