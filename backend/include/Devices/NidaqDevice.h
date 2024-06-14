#ifndef NI_DAQ_DEVICE_H
#define NI_DAQ_DEVICE_H

#include "stimwalkerConfig.h"
#include "Devices/Generic/DeviceAbstract.h"
#include "Devices/Generic/CollectorDeviceAbstract.h"

namespace STIMWALKER_NAMESPACE{ namespace devices {

/// @brief Abstract class for devices
class NidaqDevice : public DeviceAbstract, public CollectorDeviceAbstract {

public:
    int getNbChannels() const override;

    int getFrameRate() const override;

    bool getIsConnected() const override;

    void connect() override;

    void disconnect() override;

    void dispose() override;

    bool isRecording() const override;

    void startRecording() override;

    void stopRecording() override;

    void listenToOnDataCollected(void* onDataCollected(const CollectorData& newData));

    std::vector<CollectorData> getData() const override;

    CollectorData getData(int index) const override;

protected:
    bool m_isConnected; ///< Is the device connected
    bool m_isRecording; ///< Is the device currently recording

    int m_nbChannels; ///< Number of channels of the device
    int m_frameRate; ///< Frame rate of the device

    std::vector<CollectorData> m_data; ///< Data collected by the device
};

}}

#endif // NI_DAQ_DEVICE_H