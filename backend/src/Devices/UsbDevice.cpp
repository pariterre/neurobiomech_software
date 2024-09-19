#include "Devices/UsbDevice.h"

#if defined(_WIN32)
#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#endif
#include <regex>

UsbDevice::UsbDevice(const std::string &port, const std::string &vid, const std::string &pid)
    : m_Port(port), m_Vid(vid), m_Pid(pid), m_Data("") {}

UsbDevice::UsbDevice(const UsbDevice &other)
    : m_Port(other.m_Port), m_Vid(other.m_Vid), m_Pid(other.m_Pid), m_Data("")
{
    // Throws an exception if the serial port is open as it cannot be copied
    if (other.m_SerialPort != nullptr && other.m_SerialPort->is_open())
    {
        throw std::runtime_error("Cannot copy UsbDevice object with an open serial port");
    }
}

UsbDevice UsbDevice::fromVidAndPid(const std::string &vid, const std::string &pid)
{
    for (const auto &device : UsbDevice::listAllUsbDevices())
    {
        if (device.m_Vid == vid && device.m_Pid == pid)
        {
            return device;
        }
    }
    throw std::runtime_error("Device not found");
}

bool UsbDevice::connect()
{
    try
    {
        // TODO Declare io and serial as class members?
        asio::io_service io;
        m_SerialPort = std::make_unique<asio::serial_port>(io, m_Port);
        m_SerialPort->set_option(asio::serial_port_base::baud_rate(9600));
        m_SerialPort->set_option(asio::serial_port_base::character_size(8));
        m_SerialPort->set_option(asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one));
        m_SerialPort->set_option(asio::serial_port_base::parity(asio::serial_port_base::parity::none));

        // Get number of bytes in the queue of the serial port
        // std::size_t bytes = m_SerialPort->available();
        asio::streambuf buf;
        asio::read(*m_SerialPort, buf, asio::transfer_exactly(100));

        std::istream is(&buf);
        is >> m_Data;

        std::cout << "Data: " << m_Data << std::endl;

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Connection error: " << e.what() << std::endl;
        return false;
    }
}

void UsbDevice::setRapidMode(bool rapid)
{
    if (rapid)
    {
        m_SerialPort->set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::hardware));
    }
    else
    {
        m_SerialPort->set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none));
    }
}

std::vector<UsbDevice> UsbDevice::listAllUsbDevices()
{
    std::vector<UsbDevice> devices;

#if defined(_WIN32)
    HDEVINFO deviceInfoSet;
    SP_DEVINFO_DATA deviceInfoData;
    DWORD deviceIndex = 0;
    std::string vidPid;

    // Get the device information set for all devices in the Ports (COM & LPT) class
    deviceInfoSet = SetupDiGetClassDevs(NULL, "USB", NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);

    if (deviceInfoSet == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to get device information set" << std::endl;
        return devices;
    }

    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    // Iterate through the device info set
    while (SetupDiEnumDeviceInfo(deviceInfoSet, deviceIndex, &deviceInfoData))
    {
        deviceIndex++;

        // Get the DeviceInstanceID
        char deviceInstanceId[MAX_DEVICE_ID_LEN];
        if (SetupDiGetDeviceInstanceIdA(deviceInfoSet, &deviceInfoData, deviceInstanceId, sizeof(deviceInstanceId), NULL))
        {
            std::string deviceInstanceStr(deviceInstanceId);

            // Check if this device is associated with the target COM port
            HKEY hDeviceRegistryKey = SetupDiOpenDevRegKey(deviceInfoSet, &deviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
            if (hDeviceRegistryKey != INVALID_HANDLE_VALUE)
            {
                char portName[256];
                DWORD portNameSize = sizeof(portName);
                DWORD regType = 0;

                if (RegQueryValueExA(hDeviceRegistryKey, "PortName", NULL, &regType, (LPBYTE)portName, &portNameSize) == ERROR_SUCCESS)
                {
                    std::regex vidPidRegex("VID_([0-9A-F]+)&PID_([0-9A-F]+)", std::regex::icase);
                    std::smatch match;
                    if (std::regex_search(deviceInstanceStr, match, vidPidRegex))
                    {
                        devices.push_back(UsbDevice(portName, match.str(1), match.str(2)));
                    }
                }
                RegCloseKey(hDeviceRegistryKey);
            }
        }
    }
    SetupDiDestroyDeviceInfoList(deviceInfoSet);

#else
    for (const auto &devName : std::filesystem::directory_iterator("/dev"))
    {
        if (devName.path().string().find("ttyUSB") != std::string::npos ||
            devName.path().string().find("ttyACM") != std::string::npos)
        {
            std::string port = devName.path().string();

            file.open("/sys/class/tty/" + port + "/device/idVendor");
            std::string vid;
            file >> vid;
            file.close();

            std::ifstream file("/sys/class/tty/" + port + "/device/idProduct");
            std::string pid;
            file >> pid;
            file.close();

            devices.push_back(UsbDevice(port, vid, pid));
        }
    }
#endif

    return devices;
}

bool UsbDevice::operator==(const UsbDevice &other) const
{
    return m_Port == other.m_Port && m_Vid == other.m_Vid && m_Pid == other.m_Pid;
}
