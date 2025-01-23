#ifndef __NEUROBIO_DEVICES_GENERIC_EXCEPTIONS_H__
#define __NEUROBIO_DEVICES_GENERIC_EXCEPTIONS_H__
// Generate the custom exceptions

#include "neurobioConfig.h"
#include <exception>
#include <string>

#include "Utils/CppMacros.h"

namespace NEUROBIO_NAMESPACE ::devices {

class DeviceException : public std::exception {
public:
  DeviceException(const std::string &message) : m_Message(message) {}

  const char *what() const noexcept override { return m_Message.c_str(); }

protected:
  DECLARE_PROTECTED_MEMBER_NOGET(std::string, Message);
};

class DeviceDataNotAvailableException : public DeviceException {
public:
  DeviceDataNotAvailableException(const std::string &filename)
      : DeviceException(filename) {}
};

class DeviceNotFoundException : public DeviceException {
public:
  DeviceNotFoundException(const std::string &filename)
      : DeviceException(filename) {}
};

class UnknownCommandException : public DeviceException {
public:
  UnknownCommandException(const std::string &filename)
      : DeviceException(filename) {}
};

class InvalidMethodException : public DeviceException {
public:
  InvalidMethodException(const std::string &filename)
      : DeviceException(filename) {}
};

} // namespace NEUROBIO_NAMESPACE::devices

#endif // __NEUROBIO_DEVICES_GENERIC_EXCEPTIONS_H__