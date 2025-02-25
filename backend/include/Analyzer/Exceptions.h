#ifndef __NEUROBIO_ANALYZERS_EXCEPTIONS_H__
#define __NEUROBIO_ANALYZERS_EXCEPTIONS_H__
// Generate the custom exceptions

#include "neurobioConfig.h"
#include <chrono>
#include <exception>
#include <string>

#include "Utils/CppMacros.h"

namespace NEUROBIO_NAMESPACE ::analyzer {

class TimeWentBackwardException : public std::exception {
public:
  TimeWentBackwardException(
      const std::chrono::system_clock::time_point &predictionTime,
      const std::chrono::system_clock::time_point &lastAnalyzedTimeStamp)
      : m_Message(
            "Time went backwards (" +
            std::to_string(predictionTime.time_since_epoch().count()) + " < " +
            std::to_string(lastAnalyzedTimeStamp.time_since_epoch().count()) +
            "), skipping prediction") {}

  const char *what() const noexcept override { return m_Message.c_str(); }

protected:
  DECLARE_PROTECTED_MEMBER_NOGET(std::string, Message);
};

} // namespace NEUROBIO_NAMESPACE::analyzer

#endif // __NEUROBIO_ANALYZERS_EXCEPTIONS_H__