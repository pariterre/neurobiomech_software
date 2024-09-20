#include "Devices/Generic/UsbDevice.h"

#if defined(_WIN32)
#include <cfgmgr32.h>
#include <setupapi.h>
#include <windows.h>
#else // Linux or macOS
#include <filesystem>
#include <fstream>
#endif // _WIN32
#include <regex>
#include <thread>

#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::devices;

UsbDevice::UsbDevice(const std::string &port, const std::string &vid,
                     const std::string &pid)
    : m_Port(port), m_Vid(vid), m_Pid(pid) {}

UsbDevice::UsbDevice(const UsbDevice &other)
    : m_Port(other.m_Port), m_Vid(other.m_Vid), m_Pid(other.m_Pid) {
  // Throws an exception if the serial port is open as it cannot be copied
  if (other.m_SerialPort != nullptr && other.m_SerialPort->is_open()) {
    throw UsbIllegalOperationException(
        "Cannot copy UsbDevice object with an open serial port");
  }
}

UsbDevice UsbDevice::fromVidAndPid(const std::string &vid,
                                   const std::string &pid) {
  for (const auto &device : UsbDevice::listAllUsbDevices()) {
    if (device.m_Vid == vid && device.m_Pid == pid) {
      return device;
    }
  }
  throw UsbDeviceNotFoundException("USB device not found");
}

void UsbDevice::connect() {
  // Start a worker thread to run the device using the [_initialize] method
  m_Context = std::make_unique<asio::io_context>();

  // Start the worker thread
  m_Worker = std::thread([this] {
    _initialize();
    m_Context->run();
  });

  // Give a bit of time for the worker thread to start
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void UsbDevice::disconnect() {
  // Just leave a bit of time if there are any pending commands to process
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // Stop the worker thread
  m_Context->stop();
  m_Worker.join();
}

UsbResponses UsbDevice::send(const UsbCommands &command, const std::any &data,
                             bool ignoreResponse) {
  // Create a promise and get the future associated with it
  std::promise<UsbResponses> promise;
  std::future<UsbResponses> future = promise.get_future();
  // Send a command to the worker to relay commands to the device
  m_Context->post(
      [this, command, data = data, p = &promise, ignoreResponse]() mutable {
        std::lock_guard<std::mutex> lock(m_Mutex);

        // Parse the command and get the response
        auto response = _parseCommand(command, data);

        // Set the response value in the promise
        if (!ignoreResponse) {
          p->set_value(response);
        }
      });

  if (ignoreResponse) {
    return UsbResponses::OK;
  }

  // Wait for the response from the worker thread and return the result
  return future.get();
}

void UsbDevice::_initialize() {
  m_SerialPortContext = std::make_unique<asio::io_context>();
  _connectSerialPort();
}

UsbResponses UsbDevice::_parseCommand(const UsbCommands &command,
                                      const std::any &data) {
  auto &logger = Logger::getInstance();

  try {
    switch (command.getValue()) {
    case UsbCommands::PRINT:
      logger.info("Sent command: " + std::any_cast<std::string>(data));

      return UsbResponses::OK;
    }
  } catch (const std::bad_any_cast &) {
    std::cerr << "The data you provided with the command ("
              << command.toString() << ") is invalid" << std::endl;
    return UsbResponses::ERROR;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return UsbResponses::ERROR;
  }

  return UsbResponses::COMMAND_NOT_FOUND;
}

void UsbDevice::_connectSerialPort() {
  m_SerialPort =
      std::make_unique<asio::serial_port>(*m_SerialPortContext, m_Port);
  m_SerialPort->set_option(asio::serial_port_base::baud_rate(9600));
  m_SerialPort->set_option(asio::serial_port_base::character_size(8));
  m_SerialPort->set_option(asio::serial_port_base::stop_bits(
      asio::serial_port_base::stop_bits::one));
  m_SerialPort->set_option(
      asio::serial_port_base::parity(asio::serial_port_base::parity::none));
  m_SerialPort->set_option(asio::serial_port_base::flow_control(
      asio::serial_port_base::flow_control::none));
}

void UsbDevice::_setFastCommunication(bool isFast) {
  auto &logger = Logger::getInstance();

#if defined(_WIN32)
  // Set RTS ON
  if (!EscapeCommFunction(m_SerialPort->native_handle(),
                          isFast ? SETRTS : CLRRTS)) {
    logger.error("Failed to set RTS to " + std::to_string(isFast));
  } else {
    logger.info("RTS set to " + (isFast ? "ON" : "OFF"));
  }
#else
  int status;
  if (ioctl(m_SerialPort->native_handle(), TIOCMGET, &status) < 0) {
    std::cerr << "Failed to get serial port status" << std::endl;
    return;
  }

  // Turn RTS ON or OFF
  if (isFast) {
    status |= TIOCM_RTS;
    logger.info("RTS set to ON (fast)");
  } else {
    status &= ~TIOCM_RTS;
    logger.info("RTS set to OFF (slow)");
  }

  if (ioctl(m_SerialPort->native_handle(), TIOCMSET, &status) < 0) {
    logger.error("Failed to set RTS");
  }

#endif
}

std::vector<UsbDevice> UsbDevice::listAllUsbDevices() {
  std::vector<UsbDevice> devices;

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
            devices.push_back(UsbDevice(portName, match.str(1), match.str(2)));
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

      devices.push_back(UsbDevice(port, vid, pid));
    }
  }

#endif

  return devices;
}

bool UsbDevice::operator==(const UsbDevice &other) const {
  return m_Port == other.m_Port && m_Vid == other.m_Vid && m_Pid == other.m_Pid;
}
