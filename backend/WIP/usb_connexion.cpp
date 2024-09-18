#include <asio.hpp>
#include <Utils/UsbDevice.h>

int main()
{
    try
    {
        std::string vid = "067B";
        std::string pid = "2303";
        auto device = UsbDevice::fromVidAndPid(vid, pid);

        asio::io_service io;
        asio::serial_port serial(io, device.getPort());
        serial.set_option(asio::serial_port_base::baud_rate(9600));

        std::cout << "Opened port: " << device.getPort() << std::endl;
        // Communicate with the device here
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}