#include "Devices/UsbDevice.h"

#if defined(_WIN32)
#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#endif
#include <regex>

UsbDevice::UsbDevice(const std::string &port, const std::string &vid, const std::string &pid)
    : m_Port(port), m_Vid(vid), m_Pid(pid)
{
}

UsbDevice::UsbDevice(const UsbDevice &other)
    : m_Port(other.m_Port), m_Vid(other.m_Vid), m_Pid(other.m_Pid)
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

void UsbDevice::connect()
{
    // Start a worker thread to run the device using the [_initialize] method
    m_Context = std::make_unique<asio::io_context>();

    // Start the worker thread
    m_Worker = std::thread([this]
                           { _initialize(); m_Context->run(); });
}

void UsbDevice::disconnect()
{
    // Stop the worker thread
    m_Context->stop();
    m_Worker.join();
}

void UsbDevice::send(Commands command, const std::any &data)
{
    // Send a command to the worker to relay commands to the device
    m_Context->post([this, command, data]
                    {
                        std::lock_guard<std::mutex> lock(m_Mutex);
                        _parseCommand(command, data); });
}

void UsbDevice::_initialize()
{
    m_SerialPortContext = std::make_unique<asio::io_context>();
    m_SerialPort = std::make_unique<asio::serial_port>(*m_SerialPortContext, m_Port);
    m_SerialPort->set_option(asio::serial_port_base::baud_rate(9600));
    m_SerialPort->set_option(asio::serial_port_base::character_size(8));
    m_SerialPort->set_option(asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one));
    m_SerialPort->set_option(asio::serial_port_base::parity(asio::serial_port_base::parity::none));
    m_SerialPort->set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none));

    m_KeepAliveTimer = std::make_unique<asio::steady_timer>(*m_Context);
    m_PokeInterval = std::chrono::milliseconds(1000);
    _keepAlive(m_PokeInterval);
}

void UsbDevice::_parseCommand(Commands command, const std::any &data)
{
    switch (command)
    {
    case Commands::PRINT:
    {
        const std::string &text = std::any_cast<std::string>(data);
        std::cout << "Time since epoch: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() << " ms - "
                  << "Sent command: " << text << std::endl;
        break;
    }
    case Commands::CHANGE_POKE_INTERVAL:
    {
        const std::chrono::milliseconds interval = std::any_cast<const std::chrono::milliseconds>(data);
        _changePokeInterval(interval);
        std::cout << "Changed poke interval to " << interval.count() << "ms" << std::endl;
        break;
    }
    }

    // Write the command to the device
    // asio::write(*m_SerialPort, asio::buffer(command));
}

void UsbDevice::_keepAlive(const std::chrono::milliseconds &timeout)
{
    // Set a 5-second timer
    m_KeepAliveTimer->expires_after(timeout);

    m_KeepAliveTimer->async_wait([this](const asio::error_code &ec)
                                 {
                                    // If ec is not false, it means the timer was stopped to change the interval, or the 
                                    // device was disconnected. In both cases, do nothing and return
                                    if (ec) return;
                                    // Otherwise, send a PING command to the device
                                    std::lock_guard<std::mutex> lock(m_Mutex);
                                    _parseCommand(Commands::PRINT, std::string("PING"));
                                    _keepAlive(m_PokeInterval); });
}

void UsbDevice::_changePokeInterval(std::chrono::milliseconds interval)
{
    // Stop the timer

    // Compute the remaining time before the next PING command was supposed to be sent
    auto remainingTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        m_KeepAliveTimer->expiry() - asio::steady_timer::clock_type::now());
    auto elapsedTime = m_PokeInterval - remainingTime;

    // Set the interval to the requested value
    m_PokeInterval = interval;

    // Send a keep alive command with the remaining time
    m_KeepAliveTimer->cancel();
    _keepAlive(interval - elapsedTime);
}

void UsbDevice::_setFastCommunication(bool isFast)
{
#if defined(_WIN32)
    // Set RTS ON
    if (!EscapeCommFunction(m_SerialPort->native_handle(), isFast ? SETRTS : CLRRTS))
    {
        std::cerr << "Failed to set RTS to " << (isFast ? "ON" : "OFF") << std::endl;
    }
    else
    {
        std::cout << "RTS set to " << (isFast ? "ON" : "OFF") << std::endl;
    }
#else
    void useRTS(asio::serial_port & serial, bool fast)
    {
        int status;
        if (ioctl(serial.native_handle(), TIOCMGET, &status) < 0)
        {
            std::cerr << "Failed to get serial port status" << std::endl;
            return;
        }

        // Turn RTS ON or OFF
        if (fast)
        {
            status |= TIOCM_RTS;
            std::cout << "RTS set to ON (fast)" << std::endl;
        }
        else
        {
            status &= ~TIOCM_RTS;
            std::cout << "RTS set to OFF (slow)" << std::endl;
        }

        if (ioctl(serial.native_handle(), TIOCMSET, &status) < 0)
        {
            std::cerr << "Failed to set RTS" << std::endl;
        }
    }
#endif
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
