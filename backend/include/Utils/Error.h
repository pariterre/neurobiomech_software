#ifndef STIMWALKER_UTILS_ERROR_H
#define STIMWALKER_UTILS_ERROR_H

#include "stimwalkerConfig.h"
#include "Utils/String.h"

namespace STIMWALKER_NAMESPACE
{
namespace utils
{
class String;

///
/// \brief Raise error or warning while stimwalking
///
class STIMWALKER_API Error
{
public:
    ///
    /// \brief Throw an error message
    /// \param message The error message to display
    ///
    [[noreturn]] static void raise(
        const String &message);

    ///
    /// \brief Assert that raises the error message if false
    /// \param cond The condition to assert
    /// \param message The error message to display in case of failing
    ///
    static void check(
        bool cond,
        const String &message);

    ///
    /// \brief Non-blocking assert that displays the error message if false
    /// \param cond The condition to assert
    /// \param message The warning message to display in case of failing
    ///
    static void warning(
        bool cond,
        const String &message);
};

}
}

#endif // STIMWALKER_UTILS_ERROR_H


