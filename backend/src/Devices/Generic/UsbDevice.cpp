#include "Devices/Generic/UsbDevice.h"

#if defined(_WIN32)
#include <cfgmgr32.h>
#include <regex>
#include <setupapi.h>
#include <windows.h>
#else // Linux or macOS
#include <filesystem>
#include <fstream>
#endif // _WIN32

#include "Devices/Exceptions.h"
#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

UsbDevice::UsbDevice(const std::string &port, const std::string &vid,
                     const std::string &pid)
    : m_Vid(vid), m_Pid(pid),
      SerialPortDevice(port, std::chrono::milliseconds(1000)) {}

std::string UsbDevice::deviceName() const { return "GenericUsbDevice"; }

UsbDevice UsbDevice::fromVidAndPid(const std::string &vid,
                                   const std::string &pid) {
  std::vector<std::unique_ptr<UsbDevice>> devices = listAllUsbDevices();
  for (const auto &device : devices) {
    if (device->m_Vid == vid && device->m_Pid == pid) {
      return UsbDevice(device->m_Port, vid, pid);
    }
  }
  throw SerialPortDeviceNotFoundException("USB device not found");
}

DeviceResponses UsbDevice::parseAsyncSendCommand(const DeviceCommands &command,
                                                 const std::any &data) {
  return DeviceResponses::COMMAND_NOT_FOUND;
}

std::vector<std::unique_ptr<UsbDevice>> UsbDevice::listAllUsbDevices() {
  std::vector<std::unique_ptr<UsbDevice>> devices;

#if defined(_WIN32)
  HDEVINFO deviceInfoSet;
  SP_DEVINFO_DATA deviceInfoData;
  DWORD deviceIndex = 0;
  std::string vidPid;

  // Get the device information set for all devices in the Ports (COM & LPT)
  // class
  deviceInfoSet =
      SetupDiGetClassDevs(NULL, "USB", NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);

  if (deviceInfoSet == INVALID_HANDLE_VALUE) {
    std::cerr << "Failed to get device information set" << std::endl;
    return devices;
  }

  deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

  // Iterate through the device info set
  while (SetupDiEnumDeviceInfo(deviceInfoSet, deviceIndex, &deviceInfoData)) {
    deviceIndex++;

    // Get the DeviceInstanceID
    char deviceInstanceId[MAX_DEVICE_ID_LEN];
    if (SetupDiGetDeviceInstanceIdA(deviceInfoSet, &deviceInfoData,
                                    deviceInstanceId, sizeof(deviceInstanceId),
                                    NULL)) {
      std::string deviceInstanceStr(deviceInstanceId);

      // Check if this device is associated with the target COM port
      HKEY hDeviceRegistryKey =
          SetupDiOpenDevRegKey(deviceInfoSet, &deviceInfoData, DICS_FLAG_GLOBAL,
                               0, DIREG_DEV, KEY_READ);
      if (hDeviceRegistryKey != INVALID_HANDLE_VALUE) {
        char portName[256];
        DWORD portNameSize = sizeof(portName);
        DWORD regType = 0;

        if (RegQueryValueExA(hDeviceRegistryKey, "PortName", NULL, &regType,
                             (LPBYTE)portName,
                             &portNameSize) == ERROR_SUCCESS) {
          std::regex vidPidRegex("VID_([0-9A-F]+)&PID_([0-9A-F]+)",
                                 std::regex::icase);
          std::smatch match;
          if (std::regex_search(deviceInstanceStr, match, vidPidRegex)) {
            devices.push_back(std::make_unique<UsbDevice>(
                portName, match.str(1), match.str(2)));
          }
        }
        RegCloseKey(hDeviceRegistryKey);
      }
    }
  }
  SetupDiDestroyDeviceInfoList(deviceInfoSet);

#else
  // Iterate through each directory in the USB devices path
  for (const auto &devName : std::filesystem::directory_iterator("/dev")) {
    if (devName.path().string().find("ttyUSB") != std::string::npos ||
        devName.path().string().find("ttyACM") != std::string::npos) {
      std::string port = devName.path().string();

      std::ifstream file;
      file.open("/sys/class/tty/" + port + "/device/idVendor");
      std::string vid;
      file >> vid;
      file.close();

      file = std::ifstream("/sys/class/tty/" + port + "/device/idProduct");
      std::string pid;
      file >> pid;
      file.close();

      devices.push_back(std::make_unique<UsbDevice>(port, vid, pid));
    }
  }

#endif

  return devices;
}
