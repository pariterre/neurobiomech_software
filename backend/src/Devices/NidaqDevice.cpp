#include "Devices/NidaqDevice.h"
#include "Devices/CollectorData.h"

namespace STIMWALKER_NAMESPACE{ namespace Devices {

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
    // TODO: Implement this method
    m_isConnected = true;
}

void NidaqDevice::disconnect() {
    // TODO: Implement this method
    m_isConnected = false;
}

void NidaqDevice::dispose() {
}

bool NidaqDevice::isRecording() const {
    return m_isRecording;
}

void NidaqDevice::startRecording() {
    // TODO: Implement this method
    m_isRecording = true;
}

void NidaqDevice::stopRecording() {
    // TODO: Implement this method
    m_isRecording = false;
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

}}