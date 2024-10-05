#ifndef __STIMWALKER_DEVICES_GENERIC_EXCEPTIONS_H__
#define __STIMWALKER_DEVICES_GENERIC_EXCEPTIONS_H__
// Generate the custom exceptions

#include "stimwalkerConfig.h"
#include <exception>
#include <string>

namespace STIMWALKER_NAMESPACE ::devices {

class DeviceException : public std::exception {
public:
  DeviceException(const std::string &message) : m_message(message) {}

  const char *what() const noexcept override { return m_message.c_str(); }

protected:
  std::string m_message;
};

class UnknownCommandException : public DeviceException {
public:
  UnknownCommandException(const std::string &filename)
      : DeviceException(filename) {}
};

class DeviceIsConnectedException : public DeviceException {
public:
  DeviceIsConnectedException(const std::string &filename)
      : DeviceException(filename) {}
};

class DeviceShouldNotUseSendException : public DeviceException {
public:
  DeviceShouldNotUseSendException(const std::string &filename)
      : DeviceException(filename) {}
};

class DeviceConnexionFailedException : public DeviceException {
public:
  DeviceConnexionFailedException(const std::string &filename)
      : DeviceException(filename) {}
};

class DeviceFailedToStartRecordingException : public DeviceException {
public:
  DeviceFailedToStartRecordingException(const std::string &filename)
      : DeviceException(filename) {}
};

class DeviceFailedToStopRecordingException : public DeviceException {
public:
  DeviceFailedToStopRecordingException(const std::string &filename)
      : DeviceException(filename) {}
};

class DeviceIsNotConnectedException : public DeviceException {
public:
  DeviceIsNotConnectedException(const std::string &filename)
      : DeviceException(filename) {}
};

class DeviceIsRecordingException : public DeviceException {
public:
  DeviceIsRecordingException(const std::string &message)
      : DeviceException(message) {}
};

class DeviceIsNotRecordingException : public DeviceException {
public:
  DeviceIsNotRecordingException(const std::string &message)
      : DeviceException(message) {}
};
class DeviceDisconnectedException : public DeviceException {
public:
  DeviceDisconnectedException(const std::string &message)
      : DeviceException(message) {}
};

class SerialPortGenericException : public std::exception {
public:
  SerialPortGenericException(const std::string &message) : m_message(message) {}

  const char *what() const noexcept override { return m_message.c_str(); }

protected:
  std::string m_message;
};

class SerialPortDeviceNotFoundException : public SerialPortGenericException {
public:
  SerialPortDeviceNotFoundException(const std::string &message)
      : SerialPortGenericException(message) {}
};

class SerialPortIllegalOperationException : public SerialPortGenericException {
public:
  SerialPortIllegalOperationException(const std::string &message)
      : SerialPortGenericException(message) {}
};

} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_GENERIC_EXCEPTIONS_H__