#include "Utils/UsbDevice.h"

#if defined(_WIN32)
#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#endif
#include <regex>

bool UsbDevice::operator==(const UsbDevice &other) const
{
    return m_Port == other.m_Port && m_Vid == other.m_Vid && m_Pid == other.m_Pid;
}

UsbDevice::UsbDevice(const std::string &port, const std::string &vid, const std::string &pid)
    : m_Port(port), m_Vid(vid), m_Pid(pid) {}

UsbDevice UsbDevice::fromVidAndPid(const std::string &vid, const std::string &pid)
{
    for (const auto &device : UsbDevice::listAll())
    {
        if (device.getVid() == vid && device.getPid() == pid)
        {
            return device;
        }
    }
    throw std::runtime_error("Device not found");
}

std::vector<UsbDevice> UsbDevice::listAll()
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
            std::cout << "DeviceInstanceID: " << deviceInstanceStr << std::endl;

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
