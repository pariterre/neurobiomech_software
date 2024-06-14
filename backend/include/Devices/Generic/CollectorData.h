#ifndef COLLECTOR_DATA_H
#define COLLECTOR_DATA_H

#include "stimwalkerConfig.h"

namespace STIMWALKER_NAMESPACE{ namespace devices {

/// @brief Abstract class for data
class CollectorData {
protected:
    double m_timestamp; ///< Timestamp of the data
};


}}
#endif // COLLECTOR_DATA_H
