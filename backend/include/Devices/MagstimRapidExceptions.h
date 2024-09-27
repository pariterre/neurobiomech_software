#ifndef __STIMWALKER_DEVICES_MAGSTIM_RAPID_EXCEPTIONS_H__
#define __STIMWALKER_DEVICES_MAGSTIM_RAPID_EXCEPTIONS_H__

#include "Generic/Exceptions.h"

namespace STIMWALKER_NAMESPACE ::devices {

class MagsimRapidAlreadyArmedException : public SerialPortGenericException {
public:
  MagsimRapidAlreadyArmedException(const std::string &message)
      : SerialPortGenericException(message) {}
};

class MagsimRapidNotArmedException : public SerialPortGenericException {
public:
  MagsimRapidNotArmedException(const std::string &message)
      : SerialPortGenericException(message) {}
};

} // namespace STIMWALKER_NAMESPACE::devices

#endif // __STIMWALKER_DEVICES_MAGSTIM_RAPID_EXCEPTIONS_H__