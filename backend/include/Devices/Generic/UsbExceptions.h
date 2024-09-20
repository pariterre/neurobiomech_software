#ifndef __STIMWALKER_DEVICES_GENERIC_USB_EXCEPTIONS_H__
#define __STIMWALKER_DEVICES_GENERIC_USB_EXCEPTIONS_H__

#include "stimwalkerConfig.h"
#include <exception>
#include <string>

namespace STIMWALKER_NAMESPACE ::devices {

class UsbGenericException : public std::exception {
public:
  UsbGenericException(const std::string &message) : m_message(message) {}

  const char *what() const noexcept override { return m_message.c_str(); }

protected:
  std::string m_message;
};

class UsbDeviceNotFoundException : public UsbGenericException {
public:
  UsbDeviceNotFoundException(const std::string &message)
      : UsbGenericException(message) {}
};

class UsbIllegalOperationException : public UsbGenericException {
public:
  UsbIllegalOperationException(const std::string &message)
      : UsbGenericException(message) {}
};

} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_GENERIC_USB_EXCEPTIONS_H__