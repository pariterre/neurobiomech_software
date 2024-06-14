#ifndef STIMULATOR_DATA_H
#define STIMULATOR_DATA_H

#include "stimwalkerConfig.h"
#include <vector>

namespace STIMWALKER_NAMESPACE::devices
{

    /// @brief Abstract class for data
    class StimulatorData
    {
    protected:
        double m_timestamp;                                 ///< Timestamp of the data
        std::vector<std::pair<int, double>> m_stimulations; ///< Channel of the data
    };

}
#endif // STIMULATOR_DATA_H
