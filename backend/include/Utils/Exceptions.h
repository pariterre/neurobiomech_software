// Generate the custom exceptions

#include "stimwalkerConfig.h"
#include <exception>
#include <string>

namespace STIMWALKER_NAMESPACE::utils
{

    /// @brief Base class for all exceptions in the Utils namespace
    class UtilsException : public std::exception
    {
    public:
        /// @brief Base class for all exceptions in the Utils namespace
        /// @param message
        UtilsException(const std::string &message) : m_message(message)
        {
        }

        /// @brief Get the message of the exception
        /// @return The message of the exception
        const char *what() const noexcept override
        {
            return m_message.c_str();
        }

    protected:
        /// @brief The message of the exception
        std::string m_message;
    };

    /// @brief Exception thrown when a file is not found
    class FileNotFoundException : public UtilsException
    {
    public:
        /// @brief Exception thrown when a file is not found
        /// @param filename
        FileNotFoundException(const std::string &filename) : UtilsException(filename) {}
    };

    /// @brief Exception thrown when a file is not found
    class OutOfBoundsException : public UtilsException
    {
    public:
        /// @brief Exception thrown when an index is out of bounds
        /// @param message
        OutOfBoundsException(const std::string &message) : UtilsException(message) {}
    };

    /// @brief Exception thrown when the dimensions of a matrix are not correct
    class WrongDimensionsException : public UtilsException
    {
    public:
        /// @brief Exception thrown when the dimensions of a matrix are not correct
        /// @param message
        WrongDimensionsException(const std::string &message) : UtilsException(message) {}
    };

}
