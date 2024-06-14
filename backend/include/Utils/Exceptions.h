// Generate the custom exceptions

#include "stimwalkerConfig.h"
#include <exception>
#include <string>

namespace STIMWALKER_NAMESPACE::utils
{

    class UtilsException : public std::exception
    {
    public:
        UtilsException(const std::string &message) : m_message(message)
        {
        }

        const char *what() const noexcept override
        {
            return m_message.c_str();
        }

    protected:
        std::string m_message;
    };

    class FileNotFoundException : public UtilsException
    {
    public:
        FileNotFoundException(const std::string &filename) : UtilsException(filename) {}
    };

    class OutOfBoundsException : public UtilsException
    {
    public:
        OutOfBoundsException(const std::string &message) : UtilsException(message) {}
    };

    class WrongDimensionsException : public UtilsException
    {
    public:
        WrongDimensionsException(const std::string &message) : UtilsException(message) {}
    };

}
