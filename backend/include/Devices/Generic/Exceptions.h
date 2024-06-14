// Generate the custom exceptions

#include "stimwalkerConfig.h"
#include <exception>
#include <string>

namespace STIMWALKER_NAMESPACE { namespace devices {

class DeviceException : public std::exception {
public:
    DeviceException(const std::string& message) : m_message(message) {
    }

    const char* what() const noexcept override {
        return m_message.c_str();
    }

protected:
    std::string m_message;
};

class DeviceIsConnectedException : public DeviceException {
public:
    DeviceIsConnectedException(const std::string& filename) : DeviceException(filename) {}
};

class DeviceIsNotConnectedException : public DeviceException {
public:
    DeviceIsNotConnectedException(const std::string& filename) : DeviceException(filename) {}
};

class DeviceIsRecordingException : public DeviceException {
public:
    DeviceIsRecordingException(const std::string& message) : DeviceException(message) {}
};

class DeviceIsNotRecordingException : public DeviceException {
public:
    DeviceIsNotRecordingException(const std::string& message) : DeviceException(message) {}
};

}}



