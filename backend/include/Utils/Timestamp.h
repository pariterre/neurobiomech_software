#ifndef __STIMWALKER_UTILS_TIMESTAMP_H__
#define __STIMWALKER_UTILS_TIMESTAMP_H__

#include <cstdint>

#include "stimwalkerConfig.h"

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
        uint64_t timeSinceEpoch();
    };
}

#endif // __STIMWALKER_UTILS_TIMESTAMP_H__
