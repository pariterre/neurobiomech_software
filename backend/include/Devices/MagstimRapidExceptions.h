#ifndef __STIMWALKER_DEVICES_MAGSTIM_RAPID_EXCEPTIONS_H__
#define __STIMWALKER_DEVICES_MAGSTIM_RAPID_EXCEPTIONS_H__

#include "Generic/UsbExceptions.h"

namespace STIMWALKER_NAMESPACE ::devices {

class MagsimRapidAlreadyArmedException : public UsbGenericException {
public:
  MagsimRapidAlreadyArmedException(const std::string &message)
      : UsbGenericException(message) {}
};

class MagsimRapidNotArmedException : public UsbGenericException {
public:
  MagsimRapidNotArmedException(const std::string &message)
      : UsbGenericException(message) {}
};

} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_MAGSTIM_RAPID_EXCEPTIONS_H__