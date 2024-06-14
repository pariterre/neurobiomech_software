#include "Devices/NidaqDevice.h"

#include "Devices/Generic/Exceptions.h"

namespace STIMWALKER_NAMESPACE{ namespace devices {

NidaqDevice::NidaqDevice(
    int nbChannels,
    int frameRate
) : m_nbChannels(nbChannels),
    m_frameRate(frameRate),
    m_isConnected(false),
    m_isRecording(false) {
}

int NidaqDevice::getNbChannels() const {
    return m_nbChannels;
}

int NidaqDevice::getFrameRate() const {
    return m_frameRate;
}

bool NidaqDevice::getIsConnected() const {
    return m_isConnected;
}

void NidaqDevice::connect() {
    throwIfCantConnect();
    // TODO: Implement this method
}

void NidaqDevice::disconnect() {
    throwIfCantDisconnect();
    // TODO: Implement this method
}

void NidaqDevice::dispose() {
    try {
        stopRecording();
    } catch (const DeviceIsRecordingException& e) {
        // Do nothing
    }

    try {
        disconnect();
    } catch (const DeviceIsNotConnectedException& e) {
        // Do nothing
    }
    
}

bool NidaqDevice::isRecording() const {
    return m_isRecording;
}

void NidaqDevice::startRecording() {
    throwIfCantStartRecording();
    // TODO: Implement this method
}

void NidaqDevice::stopRecording() {
    throwIfCantStopRecording();
    // TODO: Implement this method
}

void NidaqDevice::listenToOnDataCollected(void* onDataCollected(const CollectorData& newData)) {
    // TODO: Implement this method
}

std::vector<CollectorData> NidaqDevice::getData() const {
    return m_data;
}

CollectorData NidaqDevice::getData(int index) const {
    return m_data[index];
}

void NidaqDevice::throwIfCantConnect() const {
    if (getIsConnected()) {
        throw DeviceIsConnectedException("The device is already connected");
    }
}

void NidaqDevice::throwIfCantDisconnect() const {
    if (isRecording()) {
        throw DeviceIsRecordingException("The device is currently recording");
    }

    if (!getIsConnected()) {
        throw DeviceIsNotConnectedException("The device is not connected");
    }
}

void NidaqDevice::throwIfCantStartRecording() const {
    if (!getIsConnected()) {
        throw DeviceIsNotConnectedException("The device is not connected");
    }
    if (isRecording()) {
        throw DeviceIsRecordingException("The device is already recording");
    }
}

void NidaqDevice::throwIfCantStopRecording() const {
    if (!isRecording()) {
        throw DeviceIsNotRecordingException("The device is not recording");
    }
}

// Mock implementation
NidaqDeviceMock::NidaqDeviceMock(
    int nbChannels,
    int frameRate
) : NidaqDevice(nbChannels, frameRate) {
}

void NidaqDeviceMock::connect() {
    throwIfCantConnect();
    m_isConnected = true;
}

void NidaqDeviceMock::disconnect() {
    throwIfCantDisconnect();
    m_isConnected = false;
}

void NidaqDeviceMock::startRecording() {
    throwIfCantStartRecording();
    m_isRecording = true;
}

void NidaqDeviceMock::stopRecording() {
    throwIfCantStopRecording();
    m_isRecording = false;
}


}}