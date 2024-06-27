#ifndef STIMWALKER_UTILS_TIMESTAMP_H
#define STIMWALKER_UTILS_TIMESTAMP_H

#include "stimwalkerConfig.h"
#include <cstdint>

namespace STIMWALKER_NAMESPACE::utils
{

    ///
    /// \brief Wrapper around chrono for milliseconds in C++
    ///
    class STIMWALKER_API Timestamp
    {
    public:
        ///
        /// \brief Construct timestamp
        ///
        Timestamp();

        ///
        /// \brief Get the current time since epoch in milliseconds
        /// \return The current time since epoch in milliseconds
        ///
        uint64_t timeSinceEpochMillisec();
    };
}

#endif // STIMWALKER_UTILS_TIMESTAMP_H
