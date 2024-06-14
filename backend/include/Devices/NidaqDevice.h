#ifndef NI_DAQ_DEVICE_H
#define NI_DAQ_DEVICE_H

#include "stimwalkerConfig.h"
#include <memory>
#include "Devices/Generic/Device.h"
#include "Devices/Generic/Collector.h"

namespace STIMWALKER_NAMESPACE{ namespace devices {

/// @brief Abstract class for devices
class NidaqDevice : public Device, public Collector {

public:
    /// @brief Constructor
    /// @param nbChannels The number of channels
    /// @param frameRate The frame rate
    NidaqDevice(
        int nbChannels,
        int frameRate
    );

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

    /// @brief Throw an exception if the device can't connect
    void throwIfCantConnect() const;

    /// @brief Throw an exception if the device can't disconnect
    void throwIfCantDisconnect() const;

    /// @brief Throw an exception if the device can't start recording
    void throwIfCantStartRecording() const;

    /// @brief Throw an exception if the device can't stop recording
    void throwIfCantStopRecording() const;


};

class NidaqDeviceMock : public NidaqDevice {    
public:
    NidaqDeviceMock(
        int nbChannels,
        int frameRate
    );

    void connect() override;

    void disconnect() override;

    void startRecording() override;

    void stopRecording() override;
};

}}

#endif // NI_DAQ_DEVICE_H